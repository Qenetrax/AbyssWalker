#ifndef ADAPTIXCLIENT_TUNNELWORKER_H
#define ADAPTIXCLIENT_TUNNELWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>
#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <atomic>

class TunnelWorker : public QObject
{
    Q_OBJECT

public:
    explicit TunnelWorker(const QString& tunnelId, const QString& otp, const QUrl& wsUrl, QObject* parent = nullptr);
    ~TunnelWorker() override;

    QString tunnelId() const;
    bool isActive() const;

public slots:
    void start();
    void stop();
    void sendData(const QByteArray& data);

signals:
    void started();
    void stopped();
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);
    void connectionStateChanged(const QString& state);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);
    void onWebSocketError(QAbstractSocket::SocketError error);

private:
    void initializeWebSocket();
    void processBuffer();
    void cleanup();

    QString m_tunnelId;
    QString m_otp;
    QUrl m_wsUrl;

    QWebSocket* m_webSocket = nullptr;
    QTimer* m_heartbeatTimer = nullptr;

    QMutex m_bufferMutex;
    QQueue<QByteArray> m_sendBuffer;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_stopped{false};

    static const int HEARTBEAT_INTERVAL = 30000;
};

#endif // ADAPTIXCLIENT_TUNNELWORKER_H