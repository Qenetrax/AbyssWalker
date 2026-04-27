#ifndef ADAPTIXCLIENT_BUILDWORKER_H
#define ADAPTIXCLIENT_BUILDWORKER_H

#include <QObject>
#include <QVariantMap>
#include <QWebSocket>
#include <QUrl>
#include <QFile>
#include <QTimer>
#include <atomic>

class BuildWorker : public QObject
{
    Q_OBJECT

public:
    explicit BuildWorker(const QVariantMap& config, const QString& accessToken, const QString& serverUrl, QObject* parent = nullptr);
    ~BuildWorker() override;

public slots:
    void start();
    void cancel();

signals:
    void started();
    void progress(const QString& status, int percent);
    void finished(const QString& outputPath);
    void errorOccurred(const QString& error);
    void logMessage(const QString& message);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& data);
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onTimeout();

private:
    void initializeWebSocket();
    void sendBuildRequest();
    void processBuildResponse(const QJsonObject& response);
    void handleBinaryData(const QByteArray& data);
    void cleanup();

    QWebSocket* m_webSocket = nullptr;
    QTimer* m_timeoutTimer = nullptr;

    QVariantMap m_config;
    QString m_accessToken;
    QString m_serverUrl;
    QString m_otp;
    QString m_outputPath;

    QFile m_outputFile;
    qint64 m_expectedSize = 0;
    qint64 m_receivedSize = 0;

    std::atomic<bool> m_canceled{false};
    std::atomic<bool> m_finished{false};

    static const int TIMEOUT_MS = 300000; // 5 minutes
};

#endif // ADAPTIXCLIENT_BUILDWORKER_H