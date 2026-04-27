#include "websocketclient.h"

#include <QJsonDocument>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QSslSocket>

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent)
{
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WebSocketClient::sendHeartbeat);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::attemptReconnect);
}

WebSocketClient::~WebSocketClient()
{
    cleanup();
}

void WebSocketClient::initializeWebSocket()
{
    if (m_webSocket) {
        m_webSocket->disconnect();
        m_webSocket->deleteLater();
    }

    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // Configure SSL
    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_webSocket->setSslConfiguration(sslConfig);
    m_webSocket->ignoreSslErrors();

    // Connect signals
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketClient::onBinaryMessageReceived);
    connect(m_webSocket, &QWebSocket::pong, this, &WebSocketClient::onPong);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocketClient::onError);
#endif
}

bool WebSocketClient::isConnected() const
{
    return m_connected;
}

void WebSocketClient::connectToServer(const QString& url, const QString& password)
{
    m_serverUrl = url;
    m_password = password;
    m_connecting = true;
    m_reconnectAttempts = 0;

    initializeWebSocket();

    QUrl wsUrl(url);
    if (!wsUrl.isValid()) {
        emit errorOccurred(tr("Invalid server URL"));
        return;
    }

    // Build URL with query parameters
    QUrlQuery query;
    if (!password.isEmpty()) {
        query.addQueryItem("password", password);
    }
    wsUrl.setQuery(query);

    QNetworkRequest request(wsUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    emit connectionStateChanged(tr("Connecting..."));
    m_webSocket->open(request);
}

void WebSocketClient::disconnect()
{
    m_autoReconnect = false;
    m_reconnectTimer->stop();

    if (m_webSocket && m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
    }

    m_connected = false;
    m_connecting = false;
}

void WebSocketClient::sendMessage(const QJsonObject& message)
{
    if (!m_connected || !m_webSocket) {
        // Queue message if not connected
        QMutexLocker locker(&m_queueMutex);
        m_messageQueue.enqueue(message);
        return;
    }

    QJsonDocument doc(message);
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(jsonStr);
}

void WebSocketClient::sendBinaryMessage(const QByteArray& data)
{
    if (!m_connected || !m_webSocket) {
        emit errorOccurred(tr("Cannot send binary message: not connected"));
        return;
    }

    m_webSocket->sendBinaryMessage(data);
}

void WebSocketClient::enableAutoReconnect(bool enable, int intervalMs)
{
    m_autoReconnect = enable;
    m_reconnectInterval = intervalMs;
}

void WebSocketClient::setHeartbeatInterval(int intervalMs)
{
    m_heartbeatInterval = intervalMs;
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start(m_heartbeatInterval);
    }
}

void WebSocketClient::onConnected()
{
    m_connected = true;
    m_connecting = false;
    m_reconnectAttempts = 0;

    emit connectionStateChanged(tr("Connected"));
    emit connected();

    // Start heartbeat
    m_heartbeatTimer->start(m_heartbeatInterval);

    // Process queued messages
    processMessageQueue();
}

void WebSocketClient::onDisconnected()
{
    m_connected = false;
    m_connecting = false;
    m_heartbeatTimer->stop();

    emit connectionStateChanged(tr("Disconnected"));
    emit disconnected();

    // Attempt reconnect if enabled
    if (m_autoReconnect && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void WebSocketClient::onTextMessageReceived(const QString& message)
{
    if (message.isEmpty())
        return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(tr("Failed to parse message: %1").arg(parseError.errorString()));
        return;
    }

    if (doc.isObject()) {
        emit messageReceived(doc.object());
    }
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray& data)
{
    if (!data.isEmpty()) {
        emit binaryMessageReceived(data);
    }
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    QString errorMsg;
    if (m_webSocket) {
        errorMsg = m_webSocket->errorString();
    } else {
        errorMsg = tr("Unknown WebSocket error");
    }

    m_connecting = false;
    emit connectionStateChanged(tr("Error: %1").arg(errorMsg));
    emit errorOccurred(errorMsg);

    // Attempt reconnect if enabled
    if (m_autoReconnect && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void WebSocketClient::onPong(quint64 elapsedTime, const QByteArray& payload)
{
    Q_UNUSED(elapsedTime);
    Q_UNUSED(payload);
    // Heartbeat response received - connection is alive
}

void WebSocketClient::sendHeartbeat()
{
    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->ping();
    }
}

void WebSocketClient::attemptReconnect()
{
    if (m_connected || m_connecting)
        return;

    m_reconnectAttempts++;
    emit connectionStateChanged(tr("Reconnecting... (attempt %1/%2)")
                                    .arg(m_reconnectAttempts)
                                    .arg(MAX_RECONNECT_ATTEMPTS));

    connectToServer(m_serverUrl, m_password);
}

void WebSocketClient::processMessageQueue()
{
    QMutexLocker locker(&m_queueMutex);

    while (!m_messageQueue.isEmpty() && m_connected) {
        QJsonObject message = m_messageQueue.dequeue();
        locker.unlock();
        sendMessage(message);
        locker.relock();
    }
}

void WebSocketClient::cleanup()
{
    m_heartbeatTimer->stop();
    m_reconnectTimer->stop();

    if (m_webSocket) {
        m_webSocket->disconnect();
        m_webSocket->close();
        m_webSocket->deleteLater();
        m_webSocket = nullptr;
    }

    m_connected = false;
    m_connecting = false;
}