#include "buildworker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

BuildWorker::BuildWorker(const QVariantMap& config, const QString& accessToken, const QString& serverUrl, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_accessToken(accessToken)
    , m_serverUrl(serverUrl)
{
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &BuildWorker::onTimeout);
}

BuildWorker::~BuildWorker()
{
    cleanup();
}

void BuildWorker::start()
{
    if (m_canceled)
        return;

    emit started();
    emit progress(tr("Initializing build..."), 0);

    initializeWebSocket();
}

void BuildWorker::cancel()
{
    if (m_canceled.exchange(true))
        return;

    emit progress(tr("Build canceled"), 0);
    cleanup();
    emit errorOccurred(tr("Build canceled by user"));
}

void BuildWorker::initializeWebSocket()
{
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_webSocket->setSslConfiguration(sslConfig);
    m_webSocket->ignoreSslErrors();

    connect(m_webSocket, &QWebSocket::connected, this, &BuildWorker::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &BuildWorker::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &BuildWorker::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &BuildWorker::onBinaryMessageReceived);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &BuildWorker::onWebSocketError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &BuildWorker::onWebSocketError);
#endif

    // Build WebSocket URL
    QUrl url(m_serverUrl);
    QString wsScheme = url.scheme() == "https" ? "wss" : "ws";
    QString wsUrl = QString("%1://%2%3/build")
                        .arg(wsScheme)
                        .arg(url.host())
                        .arg(url.path().isEmpty() ? "" : url.path().left(url.path().lastIndexOf('/')));

    QUrl buildUrl(wsUrl);
    QUrlQuery query;
    query.addQueryItem("token", m_accessToken);
    buildUrl.setQuery(query);

    QNetworkRequest request(buildUrl);
    m_webSocket->open(request);

    m_timeoutTimer->start(TIMEOUT_MS);
}

void BuildWorker::onWebSocketConnected()
{
    if (m_canceled)
        return;

    emit progress(tr("Connected to build server"), 5);
    sendBuildRequest();
}

void BuildWorker::onWebSocketDisconnected()
{
    if (!m_finished && !m_canceled) {
        emit errorOccurred(tr("Connection lost during build"));
    }
    cleanup();
}

void BuildWorker::onTextMessageReceived(const QString& message)
{
    if (m_canceled)
        return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorOccurred(tr("Invalid response from server"));
        return;
    }

    processBuildResponse(doc.object());
}

void BuildWorker::onBinaryMessageReceived(const QByteArray& data)
{
    if (m_canceled)
        return;

    handleBinaryData(data);
}

void BuildWorker::onWebSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    QString errorMsg = m_webSocket ? m_webSocket->errorString() : tr("Unknown error");
    emit errorOccurred(tr("WebSocket error: %1").arg(errorMsg));
    cleanup();
}

void BuildWorker::onTimeout()
{
    if (!m_finished && !m_canceled) {
        emit errorOccurred(tr("Build timed out"));
        cleanup();
    }
}

void BuildWorker::sendBuildRequest()
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QJsonObject request;
    request["type"] = "build_request";
    request["config"] = QJsonObject::fromVariantMap(m_config);
    request["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(request);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));

    emit progress(tr("Build request sent"), 10);
}

void BuildWorker::processBuildResponse(const QJsonObject& response)
{
    QString type = response["type"].toString();

    if (type == "build_status") {
        QString status = response["status"].toString();
        int percent = response["percent"].toInt();
        emit progress(status, percent);
        emit logMessage(status);
    }
    else if (type == "build_ready") {
        m_expectedSize = response["size"].toVariant().toLongLong();
        QString filename = response["filename"].toString();

        // Prepare output file
        QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (downloadDir.isEmpty()) {
            downloadDir = QDir::homePath();
        }

        QDir().mkpath(downloadDir);
        m_outputPath = QDir(downloadDir).filePath(filename);
        m_outputFile.setFileName(m_outputPath);

        if (!m_outputFile.open(QIODevice::WriteOnly)) {
            emit errorOccurred(tr("Failed to create output file: %1").arg(m_outputPath));
            return;
        }

        emit progress(tr("Receiving binary..."), 90);

        // Acknowledge ready to receive
        QJsonObject ack;
        ack["type"] = "build_ack";
        QJsonDocument doc(ack);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
    else if (type == "build_complete") {
        m_finished = true;
        m_outputFile.close();
        emit progress(tr("Build completed"), 100);
        emit finished(m_outputPath);
        cleanup();
    }
    else if (type == "build_error") {
        QString error = response["error"].toString();
        emit errorOccurred(tr("Build failed: %1").arg(error));
        cleanup();
    }
    else if (type == "build_log") {
        QString message = response["message"].toString();
        emit logMessage(message);
    }
}

void BuildWorker::handleBinaryData(const QByteArray& data)
{
    if (!m_outputFile.isOpen())
        return;

    m_outputFile.write(data);
    m_receivedSize += data.size();

    if (m_expectedSize > 0) {
        int percent = static_cast<int>((m_receivedSize * 100.0) / m_expectedSize);
        percent = qMin(percent, 99); // Reserve 100% for completion
        emit progress(tr("Downloading: %1%").arg(percent), percent);
    }
}

void BuildWorker::cleanup()
{
    m_timeoutTimer->stop();

    if (m_webSocket) {
        m_webSocket->disconnect();
        if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
            m_webSocket->close();
        }
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }

    if (m_outputFile.isOpen()) {
        m_outputFile.close();
    }
}