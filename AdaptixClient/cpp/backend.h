#ifndef ADAPTIXCLIENT_BACKEND_H
#define ADAPTIXCLIENT_BACKEND_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QAbstractListModel>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QQueue>

// Forward declarations
class AgentModel;
class ListenerModel;
class CredentialModel;
class TargetModel;
class DownloadModel;
class TunnelModel;
class WebSocketClient;
class BuildWorker;
class TunnelWorker;
class TerminalWorker;
class FileWorker;

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(AgentModel* agents READ agentsModel CONSTANT)
    Q_PROPERTY(ListenerModel* listeners READ listenersModel CONSTANT)
    Q_PROPERTY(CredentialModel* credentials READ credentialsModel CONSTANT)
    Q_PROPERTY(TargetModel* targets READ targetsModel CONSTANT)
    Q_PROPERTY(DownloadModel* downloads READ downloadsModel CONSTANT)
    Q_PROPERTY(TunnelModel* tunnels READ tunnelsModel CONSTANT)

public:
    static Backend* instance();

    bool isConnected() const;
    QString serverUrl() const;
    QString username() const;

    AgentModel* agentsModel() const;
    ListenerModel* listenersModel() const;
    CredentialModel* credentialsModel() const;
    TargetModel* targetsModel() const;
    DownloadModel* downloadsModel() const;
    TunnelModel* tunnelsModel() const;

public slots:
    // Connection management
    void connectToServer(const QString& url, const QString& password);
    void disconnect();

    // Agent operations
    QVariantMap getAgents();
    void executeCommand(const QString& agentId, const QString& command);
    void removeAgent(const QString& agentId);
    void updateAgent(const QString& agentId, const QVariantMap& data);

    // Listener operations
    QVariantMap getListeners();
    void startListener(const QVariantMap& config);
    void stopListener(const QString& listenerId);
    void editListener(const QString& listenerId, const QVariantMap& config);

    // Agent building
    void buildAgent(const QVariantMap& config);
    void cancelBuild();

    // Tunnel operations
    void createTunnel(const QVariantMap& config);
    void closeTunnel(const QString& tunnelId);

    // File operations
    void uploadFile(const QString& agentId, const QString& localPath, const QString& remotePath);
    void downloadFile(const QString& agentId, const QString& remotePath, const QString& localPath);
    void cancelDownload(const QString& downloadId);

    // Credential operations
    void addCredential(const QVariantMap& credential);
    void removeCredential(const QString& credId);
    QVariantMap getCredential(const QString& credId);

    // Target operations
    void addTarget(const QVariantMap& target);
    void removeTarget(const QString& targetId);
    void updateTarget(const QString& targetId, const QVariantMap& data);

    // Chat operations
    void sendChatMessage(const QString& message);

    // Data retrieval
    QVariantMap getDownloads();
    QVariantMap getTunnels();

signals:
    void connectedChanged(bool connected);
    void serverUrlChanged(const QString& url);
    void usernameChanged(const QString& username);

    void connectionSucceeded();
    void connectionFailed(const QString& error);
    void disconnected();

    void agentAdded(const QString& agentId);
    void agentRemoved(const QString& agentId);
    void agentUpdated(const QString& agentId);
    void commandExecuted(const QString& agentId, const QString& taskId);
    void commandOutput(const QString& agentId, const QString& output);

    void listenerStarted(const QString& listenerId);
    void listenerStopped(const QString& listenerId);
    void listenerError(const QString& error);

    void buildStarted();
    void buildProgress(const QString& status, int percent);
    void buildCompleted(const QString& outputPath);
    void buildFailed(const QString& error);

    void tunnelCreated(const QString& tunnelId);
    void tunnelClosed(const QString& tunnelId);
    void tunnelError(const QString& tunnelId, const QString& error);

    void fileUploadProgress(const QString& agentId, const QString& path, qint64 sent, qint64 total);
    void fileDownloadProgress(const QString& downloadId, qint64 received, qint64 total);
    void fileUploadComplete(const QString& agentId, const QString& path);
    void fileDownloadComplete(const QString& downloadId, const QString& path);
    void fileOperationError(const QString& error);

    void credentialAdded(const QString& credId);
    void credentialRemoved(const QString& credId);

    void targetAdded(const QString& targetId);
    void targetRemoved(const QString& targetId);
    void targetUpdated(const QString& targetId);

    void chatMessageReceived(const QString& username, const QString& message, const QString& timestamp);
    void chatMessageSent(const QString& message);

    void notification(const QString& title, const QString& message, const QString& type);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketError(const QString& error);
    void onWebSocketMessageReceived(const QJsonObject& message);

    void onBuildWorkerFinished(const QString& outputPath);
    void onBuildWorkerError(const QString& error);
    void onBuildWorkerProgress(const QString& status, int percent);

private:
    explicit Backend(QObject* parent = nullptr);
    ~Backend() override;
    Q_DISABLE_COPY(Backend)

    void initializeModels();
    void setupConnections();
    void handleServerMessage(const QJsonObject& message);
    void sendRequest(const QString& type, const QJsonObject& data);

    static Backend* s_instance;

    bool m_connected = false;
    QString m_serverUrl;
    QString m_username;
    QString m_accessToken;
    QString m_refreshToken;

    AgentModel* m_agentsModel = nullptr;
    ListenerModel* m_listenersModel = nullptr;
    CredentialModel* m_credentialsModel = nullptr;
    TargetModel* m_targetsModel = nullptr;
    DownloadModel* m_downloadsModel = nullptr;
    TunnelModel* m_tunnelsModel = nullptr;

    WebSocketClient* m_webSocketClient = nullptr;
    BuildWorker* m_buildWorker = nullptr;
    FileWorker* m_fileWorker = nullptr;

    QThread* m_buildThread = nullptr;
    QThread* m_fileThread = nullptr;

    QTimer* m_reconnectTimer = nullptr;
    int m_reconnectAttempts = 0;
    static const int MAX_RECONNECT_ATTEMPTS = 5;
};

#endif // ADAPTIXCLIENT_BACKEND_H
