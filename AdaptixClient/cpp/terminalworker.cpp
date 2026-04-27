#include "terminalworker.h"

#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslSocket>

TerminalWorker::TerminalWorker(const QString& terminalId, const QString& otp, const QUrl& wsUrl, QObject* parent)
    : QObject(parent)
    , m_terminalId(terminalId)
    , m_otp(otp)
    , m_wsUrl(wsUrl)
{
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TerminalWorker::onHeartbeat);
}

TerminalWorker::~TerminalWorker()
{
    stop();
}

QString TerminalWorker::terminalId() const
{
    return m_terminalId;
}

bool TerminalWorker::isConnected() const
{
    return m_connected;
}

void TerminalWorker::start()
{
    if (m_running.exchange(true))
        return;

    emit connectionStateChanged(tr("Starting terminal..."));
    initializeWebSocket();
}

void TerminalWorker::stop()
{
    if (m_stopped.exchange(true))
        return;

    m_running = false;
    m_heartbeatTimer->stop();

    cleanup();

    emit stopped();
}

void TerminalWorker::sendData(const QByteArray& data)
{
    if (!m_connected) {
        QMutexLocker locker(&m_bufferMutex);
        m_sendBuffer.enqueue(data);
        return;
    }

    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->sendBinaryMessage(data);
    }
}

void TerminalWorker::resize(int rows, int cols)
{
    if (!m_connected || !m_webSocket)
        return;

    QJsonObject resizeCmd;
    resizeCmd["type"] = "resize";
    resizeCmd["rows"] = rows;
    resizeCmd["cols"] = cols;

    QJsonDocument doc(resizeCmd);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void TerminalWorker::initializeWebSocket()
{
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_webSocket->setSslConfiguration(sslConfig);
    m_webSocket->ignoreSslErrors();

    connect(m_webSocket, &QWebSocket::connected, this, &TerminalWorker::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &TerminalWorker::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &TerminalWorker::onBinaryMessageReceived);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &TerminalWorker::onWebSocketError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &TerminalWorker::onWebSocketError);
#endif

    // Build URL with OTP
    QUrl url = m_wsUrl;
    QUrlQuery query;
    query.addQueryItem("otp", m_otp);
    query.addQueryItem("terminal_id", m_terminalId);
    url.setQuery(query);

    QNetworkRequest request(url);
    m_webSocket->open(request);
}

void TerminalWorker::onWebSocketConnected()
{
    m_connected = true;
    emit connectionStateChanged(tr("Terminal connected"));
    emit started();

    m_heartbeatTimer->start(HEARTBEAT_INTERVAL);

    // Process buffered data
    processBuffer();
}

void TerminalWorker::onWebSocketDisconnected()
{
    m_connected = false;
    emit connectionStateChanged(tr("Terminal disconnected"));

    if (!m_stopped) {
        emit errorOccurred(tr("Unexpected disconnection"));
    }
}

void TerminalWorker::onBinaryMessageReceived(const QByteArray& data)
{
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void TerminalWorker::onWebSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    QString errorMsg = m_webSocket ? m_webSocket->errorString() : tr("Unknown error");
    emit errorOccurred(errorMsg);
}

void TerminalWorker::onHeartbeat()
{
    if (m_webSocket && m_connected) {
        m_webSocket->ping();
    }
}

void TerminalWorker::processBuffer()
{
    QMutexLocker locker(&m_bufferMutex);

    while (!m_sendBuffer.isEmpty() && m_connected) {
        QByteArray data = m_sendBuffer.dequeue();
        locker.unlock();
        sendData(data);
        locker.relock();
    }
}

void TerminalWorker::cleanup()
{
    if (m_webSocket) {
        m_webSocket->disconnect();
        if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
            m_webSocket->close();
        }
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }

    m_connected = false;
}