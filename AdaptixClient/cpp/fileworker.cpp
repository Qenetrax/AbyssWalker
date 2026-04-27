#include "fileworker.h"

#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QSslSocket>

FileWorker::FileWorker(QObject* parent)
    : QObject(parent)
{
    m_uploadTimer = new QTimer(this);
    m_uploadTimer->setSingleShot(false);
    connect(m_uploadTimer, &QTimer::timeout, this, &FileWorker::onUploadChunk);
}

FileWorker::~FileWorker()
{
    cleanup();
}

QString FileWorker::currentOperationId() const
{
    return m_currentOperationId;
}

bool FileWorker::isBusy() const
{
    return m_busy;
}

void FileWorker::uploadFile(const QString& operationId, const QString& filePath, const QString& agentId, const QString& remotePath, const QString& accessToken, const QUrl& serverUrl)
{
    if (m_busy) {
        emit operationFailed(operationId, tr("Another operation is in progress"));
        return;
    }

    m_currentOperationId = operationId;
    m_operationType = Upload;
    m_filePath = filePath;
    m_agentId = agentId;
    m_remotePath = remotePath;
    m_accessToken = accessToken;
    m_serverUrl = serverUrl;
    m_canceled = false;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        emit operationFailed(operationId, tr("File does not exist or is not readable"));
        return;
    }

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        emit operationFailed(operationId, tr("Failed to open file: %1").arg(m_file.errorString()));
        return;
    }

    m_fileSize = fileInfo.size();
    m_transferredSize = 0;
    m_busy = true;

    emit operationStarted(operationId, Upload);
    initializeWebSocket();
}

void FileWorker::downloadFile(const QString& operationId, const QString& filePath, const QString& agentId, const QString& remotePath, const QString& accessToken, const QUrl& serverUrl)
{
    if (m_busy) {
        emit operationFailed(operationId, tr("Another operation is in progress"));
        return;
    }

    m_currentOperationId = operationId;
    m_operationType = Download;
    m_filePath = filePath;
    m_agentId = agentId;
    m_remotePath = remotePath;
    m_accessToken = accessToken;
    m_serverUrl = serverUrl;
    m_canceled = false;

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly)) {
        emit operationFailed(operationId, tr("Failed to create file: %1").arg(m_file.errorString()));
        return;
    }

    m_fileSize = 0;
    m_transferredSize = 0;
    m_busy = true;

    emit operationStarted(operationId, Download);
    initializeWebSocket();
}

void FileWorker::cancelOperation(const QString& operationId)
{
    if (operationId != m_currentOperationId || !m_busy)
        return;

    m_canceled = true;
    m_uploadTimer->stop();

    cleanup();

    m_busy = false;
    emit operationCanceled(operationId);
}

void FileWorker::initializeWebSocket()
{
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_webSocket->setSslConfiguration(sslConfig);
    m_webSocket->ignoreSslErrors();

    connect(m_webSocket, &QWebSocket::connected, this, &FileWorker::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &FileWorker::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &FileWorker::onBinaryMessageReceived);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &FileWorker::onTextMessageReceived);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &FileWorker::onWebSocketError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &FileWorker::onWebSocketError);
#endif

    // Build WebSocket URL
    QUrl url = m_serverUrl;
    QString wsScheme = url.scheme() == "https" ? "wss" : "ws";
    QString endpoint = (m_operationType == Upload) ? "upload" : "download";
    QString wsUrl = QString("%1://%2%3/%4")
                        .arg(wsScheme)
                        .arg(url.host())
                        .arg(url.port() > 0 ? QString(":%1").arg(url.port()) : "")
                        .arg(endpoint);

    QUrl fileUrl(wsUrl);
    QUrlQuery query;
    query.addQueryItem("token", m_accessToken);
    query.addQueryItem("agent_id", m_agentId);
    query.addQueryItem("path", m_remotePath);
    query.addQueryItem("operation_id", m_currentOperationId);
    fileUrl.setQuery(query);

    QNetworkRequest request(fileUrl);
    m_webSocket->open(request);
}

void FileWorker::onWebSocketConnected()
{
    m_connected = true;

    if (m_operationType == Upload) {
        startUpload();
    } else {
        startDownload();
    }
}

void FileWorker::onWebSocketDisconnected()
{
    m_connected = false;

    if (m_busy && !m_canceled && m_transferredSize < m_fileSize) {
        finishOperation(false, tr("Connection lost during transfer"));
    }
}

void FileWorker::onBinaryMessageReceived(const QByteArray& data)
{
    if (m_canceled)
        return;

    if (m_operationType == Download) {
        processDownloadData(data);
    }
}

void FileWorker::onTextMessageReceived(const QString& message)
{
    if (m_canceled)
        return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        finishOperation(false, tr("Invalid response from server"));
        return;
    }

    if (m_operationType == Upload) {
        processUploadResponse(doc.object());
    }
}

void FileWorker::onWebSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    QString errorMsg = m_webSocket ? m_webSocket->errorString() : tr("Unknown error");
    finishOperation(false, errorMsg);
}

void FileWorker::onUploadChunk()
{
    if (m_canceled || !m_connected || !m_file.isOpen())
        return;

    QByteArray chunk = m_file.read(m_chunkSize);
    if (chunk.isEmpty()) {
        // EOF reached
        m_uploadTimer->stop();

        QJsonObject eofMsg;
        eofMsg["type"] = "upload_eof";
        eofMsg["total_size"] = m_transferredSize;
        QJsonDocument doc(eofMsg);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));

        return;
    }

    m_webSocket->sendBinaryMessage(chunk);
    m_transferredSize += chunk.size();

    emit uploadProgress(m_currentOperationId, m_transferredSize, m_fileSize);
}

void FileWorker::startUpload()
{
    // Send file info first
    QJsonObject fileInfo;
    fileInfo["type"] = "upload_start";
    fileInfo["filename"] = QFileInfo(m_filePath).fileName();
    fileInfo["size"] = m_fileSize;
    fileInfo["remote_path"] = m_remotePath;

    QJsonDocument doc(fileInfo);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));

    // Start sending chunks
    m_uploadTimer->start(UPLOAD_INTERVAL_MS);
}

void FileWorker::startDownload()
{
    // Request download
    QJsonObject downloadRequest;
    downloadRequest["type"] = "download_start";
    downloadRequest["remote_path"] = m_remotePath;

    QJsonDocument doc(downloadRequest);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void FileWorker::processUploadResponse(const QJsonObject& response)
{
    QString type = response["type"].toString();

    if (type == "upload_ack") {
        // Server acknowledged chunk, continue uploading
    }
    else if (type == "upload_progress") {
        // Server reports progress
        qint64 received = response["received"].toVariant().toLongLong();
        emit uploadProgress(m_currentOperationId, received, m_fileSize);
    }
    else if (type == "upload_complete") {
        finishOperation(true);
    }
    else if (type == "upload_error") {
        QString error = response["error"].toString();
        finishOperation(false, error);
    }
}

void FileWorker::processDownloadData(const QByteArray& data)
{
    if (!m_file.isOpen())
        return;

    m_file.write(data);
    m_transferredSize += data.size();

    emit downloadProgress(m_currentOperationId, m_transferredSize, m_fileSize);
}

void FileWorker::cleanup()
{
    m_uploadTimer->stop();

    if (m_webSocket) {
        m_webSocket->disconnect();
        if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
            m_webSocket->close();
        }
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }

    if (m_file.isOpen()) {
        m_file.close();
    }

    m_connected = false;
}

void FileWorker::finishOperation(bool success, const QString& message)
{
    m_uploadTimer->stop();
    m_busy = false;

    cleanup();

    if (success) {
        emit operationCompleted(m_currentOperationId, m_filePath);
    } else {
        emit operationFailed(m_currentOperationId, message);
    }
}