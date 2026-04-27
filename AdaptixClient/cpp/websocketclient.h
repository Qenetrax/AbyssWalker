#ifndef ADAPTIXCLIENT_WEBSOCKETCLIENT_H
#define ADAPTIXCLIENT_WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <QJsonObject>
#include <QByteArray>
#include <QSslConfiguration>

class WebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketClient(QObject* parent = nullptr);
    ~WebSocketClient() override;

    bool isConnected() const;

    void connectToServer(const QString& url, const QString& password);
    void disconnect();
    void sendMessage(const QJsonObject& message);
    void sendBinaryMessage(const QByteArray& data);

    void enableAutoReconnect(bool enable, int intervalMs = 3000);
    void setHeartbeatInterval(int intervalMs);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void messageReceived(const QJsonObject& message);
    void binaryMessageReceived(const QByteArray& data);
    void connectionStateChanged(const QString& state);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& data);
    void onError(QAbstractSocket::SocketError error);
    void onPong(quint64 elapsedTime, const QByteArray& payload);
    void sendHeartbeat();
    void attemptReconnect();

private:
    void initializeWebSocket();
    void processMessageQueue();
    void cleanup();

    QWebSocket* m_webSocket = nullptr;
    QTimer* m_heartbeatTimer = nullptr;
    QTimer* m_reconnectTimer = nullptr;

    QString m_serverUrl;
    QString m_password;
    QString m_accessToken;

    bool m_autoReconnect = true;
    int m_reconnectInterval = 3000;
    int m_heartbeatInterval = 15000;
    int m_reconnectAttempts = 0;
    static const int MAX_RECONNECT_ATTEMPTS = 10;

    QMutex m_queueMutex;
    QQueue<QJsonObject> m_messageQueue;

    bool m_connecting = false;
    bool m_connected = false;
};

#endif // ADAPTIXCLIENT_WEBSOCKETCLIENT_H