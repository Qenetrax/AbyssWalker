#ifndef ADAPTIXCLIENT_FILEWORKER_H
#define ADAPTIXCLIENT_FILEWORKER_H

#include <QObject>
#include <QFile>
#include <QWebSocket>
#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <atomic>

class FileWorker : public QObject
{
    Q_OBJECT

public:
    enum OperationType {
        Upload,
        Download
    };

    explicit FileWorker(QObject* parent = nullptr);
    ~FileWorker() override;

    QString currentOperationId() const;
    bool isBusy() const;

public slots:
    void uploadFile(const QString& operationId, const QString& filePath, const QString& agentId, const QString& remotePath, const QString& accessToken, const QUrl& serverUrl);
    void downloadFile(const QString& operationId, const QString& filePath, const QString& agentId, const QString& remotePath, const QString& accessToken, const QUrl& serverUrl);
    void cancelOperation(const QString& operationId);

signals:
    void operationStarted(const QString& operationId, int type);
    void uploadProgress(const QString& operationId, qint64 sent, qint64 total);
    void downloadProgress(const QString& operationId, qint64 received, qint64 total);
    void operationCompleted(const QString& operationId, const QString& path);
    void operationFailed(const QString& operationId, const QString& error);
    void operationCanceled(const QString& operationId);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onBinaryMessageReceived(const QByteArray& data);
    void onTextMessageReceived(const QString& message);
    void onWebSocketError(QAbstractSocket::SocketError error);
    void onUploadChunk();

private:
    void initializeWebSocket();
    void startUpload();
    void startDownload();
    void processUploadResponse(const QJsonObject& response);
    void processDownloadData(const QByteArray& data);
    void cleanup();
    void finishOperation(bool success, const QString& message = QString());

    QWebSocket* m_webSocket = nullptr;
    QTimer* m_uploadTimer = nullptr;

    QString m_currentOperationId;
    OperationType m_operationType = Upload;
    QString m_filePath;
    QString m_agentId;
    QString m_remotePath;
    QString m_accessToken;
    QUrl m_serverUrl;

    QFile m_file;
    qint64 m_fileSize = 0;
    qint64 m_transferredSize = 0;
    qint64 m_chunkSize = 65536; // 64KB chunks

    std::atomic<bool> m_busy{false};
    std::atomic<bool> m_canceled{false};
    std::atomic<bool> m_connected{false};

    static const int UPLOAD_INTERVAL_MS = 10;
};

#endif // ADAPTIXCLIENT_FILEWORKER_H