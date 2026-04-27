#ifndef ADAPTIXCLIENT_TERMINALWORKER_H
#define ADAPTIXCLIENT_TERMINALWORKER_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include <atomic>

class TerminalWorker : public QObject
{
    Q_OBJECT

public:
    explicit TerminalWorker(const QString& terminalId, const QString& otp, const QUrl& wsUrl, QObject* parent = nullptr);
    ~TerminalWorker() override;

    QString terminalId() const;
    bool isConnected() const;

public slots:
    void start();
    void stop();
    void sendData(const QByteArray& data);
    void resize(int rows, int cols);

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
    void onHeartbeat();

private:
    void initializeWebSocket();
    void processBuffer();
    void cleanup();

    QString m_terminalId;
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

#endif // ADAPTIXCLIENT_TERMINALWORKER_H