#include "tunnelworker.h"

#include <QNetworkRequest>
#include <QUrlQuery>
#include <QSslSocket>

TunnelWorker::TunnelWorker(const QString& tunnelId, const QString& otp, const QUrl& wsUrl, QObject* parent)
    : QObject(parent)
    , m_tunnelId(tunnelId)
    , m_otp(otp)
    , m_wsUrl(wsUrl)
{
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        if (m_webSocket && m_connected) {
            m_webSocket->ping();
        }
    });
}

TunnelWorker::~TunnelWorker()
{
    stop();
}

QString TunnelWorker::tunnelId() const
{
    return m_tunnelId;
}

bool TunnelWorker::isActive() const
{
    return m_running && m_connected;
}

void TunnelWorker::start()
{
    if (m_running.exchange(true))
        return;

    emit connectionStateChanged(tr("Starting tunnel..."));
    initializeWebSocket();
}

void TunnelWorker::stop()
{
    if (m_stopped.exchange(true))
        return;

    m_running = false;
    m_heartbeatTimer->stop();

    cleanup();

    emit stopped();
}

void TunnelWorker::sendData(const QByteArray& data)
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

void TunnelWorker::initializeWebSocket()
{
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    m_webSocket->setSslConfiguration(sslConfig);
    m_webSocket->ignoreSslErrors();

    connect(m_webSocket, &QWebSocket::connected, this, &TunnelWorker::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &TunnelWorker::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &TunnelWorker::onBinaryMessageReceived);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &TunnelWorker::onWebSocketError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &TunnelWorker::onWebSocketError);
#endif

    // Build URL with OTP
    QUrl url = m_wsUrl;
    QUrlQuery query;
    query.addQueryItem("otp", m_otp);
    query.addQueryItem("tunnel_id", m_tunnelId);
    url.setQuery(query);

    QNetworkRequest request(url);
    m_webSocket->open(request);
}

void TunnelWorker::onWebSocketConnected()
{
    m_connected = true;
    emit connectionStateChanged(tr("Tunnel connected"));
    emit started();

    m_heartbeatTimer->start(HEARTBEAT_INTERVAL);

    // Process buffered data
    processBuffer();
}

void TunnelWorker::onWebSocketDisconnected()
{
    m_connected = false;
    emit connectionStateChanged(tr("Tunnel disconnected"));

    if (!m_stopped) {
        emit errorOccurred(tr("Unexpected disconnection"));
    }
}

void TunnelWorker::onBinaryMessageReceived(const QByteArray& data)
{
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void TunnelWorker::onWebSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    QString errorMsg = m_webSocket ? m_webSocket->errorString() : tr("Unknown error");
    emit errorOccurred(errorMsg);
}

void TunnelWorker::processBuffer()
{
    QMutexLocker locker(&m_bufferMutex);

    while (!m_sendBuffer.isEmpty() && m_connected) {
        QByteArray data = m_sendBuffer.dequeue();
        locker.unlock();
        sendData(data);
        locker.relock();
    }
}

void TunnelWorker::cleanup()
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