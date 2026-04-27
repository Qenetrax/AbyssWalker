#include "backend.h"
#include "agentmodel.h"
#include "listenermodel.h"
#include "credentialmodel.h"
#include "targetmodel.h"
#include "downloadmodel.h"
#include "tunnelmodel.h"
#include "websocketclient.h"
#include "buildworker.h"
#include "fileworker.h"

#include <QJsonDocument>
#include <QDateTime>
#include <QUuid>

Backend* Backend::s_instance = nullptr;

Backend* Backend::instance()
{
    if (!s_instance) {
        s_instance = new Backend();
    }
    return s_instance;
}

Backend::Backend(QObject* parent)
    : QObject(parent)
{
    initializeModels();
    setupConnections();
}

Backend::~Backend()
{
    disconnect();

    if (m_buildThread) {
        m_buildThread->quit();
        m_buildThread->wait();
        delete m_buildThread;
    }
    if (m_fileThread) {
        m_fileThread->quit();
        m_fileThread->wait();
        delete m_fileThread;
    }

    delete m_webSocketClient;
    delete m_agentsModel;
    delete m_listenersModel;
    delete m_credentialsModel;
    delete m_targetsModel;
    delete m_downloadsModel;
    delete m_tunnelsModel;

    s_instance = nullptr;
}

void Backend::initializeModels()
{
    m_agentsModel = new AgentModel(this);
    m_listenersModel = new ListenerModel(this);
    m_credentialsModel = new CredentialModel(this);
    m_targetsModel = new TargetModel(this);
    m_downloadsModel = new DownloadModel(this);
    m_tunnelsModel = new TunnelModel(this);

    m_webSocketClient = new WebSocketClient(this);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!m_connected && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            m_reconnectAttempts++;
            connectToServer(m_serverUrl, QString());
        }
    });
}

void Backend::setupConnections()
{
    connect(m_webSocketClient, &WebSocketClient::connected,
            this, &Backend::onWebSocketConnected);
    connect(m_webSocketClient, &WebSocketClient::disconnected,
            this, &Backend::onWebSocketDisconnected);
    connect(m_webSocketClient, &WebSocketClient::errorOccurred,
            this, &Backend::onWebSocketError);
    connect(m_webSocketClient, &WebSocketClient::messageReceived,
            this, &Backend::onWebSocketMessageReceived);
}

bool Backend::isConnected() const
{
    return m_connected;
}

QString Backend::serverUrl() const
{
    return m_serverUrl;
}

QString Backend::username() const
{
    return m_username;
}

AgentModel* Backend::agentsModel() const
{
    return m_agentsModel;
}

ListenerModel* Backend::listenersModel() const
{
    return m_listenersModel;
}

CredentialModel* Backend::credentialsModel() const
{
    return m_credentialsModel;
}

TargetModel* Backend::targetsModel() const
{
    return m_targetsModel;
}

DownloadModel* Backend::downloadsModel() const
{
    return m_downloadsModel;
}

TunnelModel* Backend::tunnelsModel() const
{
    return m_tunnelsModel;
}

void Backend::connectToServer(const QString& url, const QString& password)
{
    m_serverUrl = url;
    m_webSocketClient->connectToServer(url, password);
}

void Backend::disconnect()
{
    m_reconnectTimer->stop();
    m_reconnectAttempts = 0;
    m_webSocketClient->disconnect();
    m_connected = false;
    emit connectedChanged(false);
    emit disconnected();
}

QVariantMap Backend::getAgents()
{
    return m_agentsModel->getAllAgents();
}

void Backend::executeCommand(const QString& agentId, const QString& command)
{
    QJsonObject data;
    data["agent_id"] = agentId;
    data["command"] = command;
    sendRequest("agent_command", data);
}

void Backend::removeAgent(const QString& agentId)
{
    QJsonObject data;
    data["agent_id"] = agentId;
    sendRequest("agent_remove", data);
}

void Backend::updateAgent(const QString& agentId, const QVariantMap& data)
{
    m_agentsModel->updateAgent(agentId, data);
    emit agentUpdated(agentId);
}

QVariantMap Backend::getListeners()
{
    return m_listenersModel->getAllListeners();
}

void Backend::startListener(const QVariantMap& config)
{
    QJsonObject data = QJsonObject::fromVariantMap(config);
    sendRequest("listener_start", data);
}

void Backend::stopListener(const QString& listenerId)
{
    QJsonObject data;
    data["listener_id"] = listenerId;
    sendRequest("listener_stop", data);
}

void Backend::editListener(const QString& listenerId, const QVariantMap& config)
{
    QJsonObject data = QJsonObject::fromVariantMap(config);
    data["listener_id"] = listenerId;
    sendRequest("listener_edit", data);
}

void Backend::buildAgent(const QVariantMap& config)
{
    if (m_buildWorker) {
        emit buildFailed("Build already in progress");
        return;
    }

    m_buildThread = new QThread(this);
    m_buildWorker = new BuildWorker(config, m_accessToken, m_serverUrl);

    m_buildWorker->moveToThread(m_buildThread);

    connect(m_buildThread, &QThread::started, m_buildWorker, &BuildWorker::start);
    connect(m_buildWorker, &BuildWorker::finished, this, &Backend::onBuildWorkerFinished);
    connect(m_buildWorker, &BuildWorker::errorOccurred, this, &Backend::onBuildWorkerError);
    connect(m_buildWorker, &BuildWorker::progress, this, &Backend::onBuildWorkerProgress);
    connect(m_buildWorker, &BuildWorker::finished, m_buildThread, &QThread::quit);
    connect(m_buildWorker, &BuildWorker::finished, m_buildWorker, &QObject::deleteLater);
    connect(m_buildThread, &QThread::finished, m_buildThread, &QObject::deleteLater);

    m_buildThread->start();
    emit buildStarted();
}

void Backend::cancelBuild()
{
    if (m_buildWorker) {
        m_buildWorker->cancel();
    }
}

void Backend::createTunnel(const QVariantMap& config)
{
    QJsonObject data = QJsonObject::fromVariantMap(config);
    sendRequest("tunnel_create", data);
}

void Backend::closeTunnel(const QString& tunnelId)
{
    QJsonObject data;
    data["tunnel_id"] = tunnelId;
    sendRequest("tunnel_close", data);
}

void Backend::uploadFile(const QString& agentId, const QString& localPath, const QString& remotePath)
{
    Q_UNUSED(localPath)
    QJsonObject data;
    data["agent_id"] = agentId;
    data["remote_path"] = remotePath;
    sendRequest("file_upload", data);
}

void Backend::downloadFile(const QString& agentId, const QString& remotePath, const QString& localPath)
{
    Q_UNUSED(localPath)
    QJsonObject data;
    data["agent_id"] = agentId;
    data["remote_path"] = remotePath;
    sendRequest("file_download", data);
}

void Backend::cancelDownload(const QString& downloadId)
{
    QJsonObject data;
    data["download_id"] = downloadId;
    sendRequest("download_cancel", data);
}

void Backend::addCredential(const QVariantMap& credential)
{
    QJsonObject data = QJsonObject::fromVariantMap(credential);
    sendRequest("credential_add", data);
}

void Backend::removeCredential(const QString& credId)
{
    QJsonObject data;
    data["cred_id"] = credId;
    sendRequest("credential_remove", data);
}

QVariantMap Backend::getCredential(const QString& credId)
{
    return m_credentialsModel->getCredential(credId);
}

void Backend::addTarget(const QVariantMap& target)
{
    QJsonObject data = QJsonObject::fromVariantMap(target);
    sendRequest("target_add", data);
}

void Backend::removeTarget(const QString& targetId)
{
    QJsonObject data;
    data["target_id"] = targetId;
    sendRequest("target_remove", data);
}

void Backend::updateTarget(const QString& targetId, const QVariantMap& data)
{
    m_targetsModel->updateTarget(targetId, data);
    emit targetUpdated(targetId);
}

void Backend::sendChatMessage(const QString& message)
{
    QJsonObject data;
    data["message"] = message;
    data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    sendRequest("chat_message", data);
    emit chatMessageSent(message);
}

QVariantMap Backend::getDownloads()
{
    return m_downloadsModel->getAllDownloads();
}

QVariantMap Backend::getTunnels()
{
    return m_tunnelsModel->getAllTunnels();
}

void Backend::onWebSocketConnected()
{
    m_connected = true;
    m_reconnectAttempts = 0;
    emit connectedChanged(true);
    emit connectionSucceeded();

    sendRequest("sync_agents", QJsonObject());
    sendRequest("sync_listeners", QJsonObject());
    sendRequest("sync_credentials", QJsonObject());
    sendRequest("sync_targets", QJsonObject());
    sendRequest("sync_downloads", QJsonObject());
    sendRequest("sync_tunnels", QJsonObject());
}

void Backend::onWebSocketDisconnected()
{
    m_connected = false;
    emit connectedChanged(false);
    emit disconnected();

    if (!m_serverUrl.isEmpty() && m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        m_reconnectTimer->start(3000);
    }
}

void Backend::onWebSocketError(const QString& error)
{
    emit connectionFailed(error);
    emit notification(tr("Connection Error"), error, "error");
}

void Backend::onWebSocketMessageReceived(const QJsonObject& message)
{
    handleServerMessage(message);
}

void Backend::onBuildWorkerFinished(const QString& outputPath)
{
    emit buildCompleted(outputPath);
    m_buildWorker = nullptr;
}

void Backend::onBuildWorkerError(const QString& error)
{
    emit buildFailed(error);
    m_buildWorker = nullptr;
}

void Backend::onBuildWorkerProgress(const QString& status, int percent)
{
    emit buildProgress(status, percent);
}

void Backend::handleServerMessage(const QJsonObject& message)
{
    QString type = message["type"].toString();

    if (type == "agent_new") {
        QJsonObject agentData = message["data"].toObject();
        m_agentsModel->addAgent(agentData.toVariantMap());
        emit agentAdded(agentData["id"].toString());
    }
    else if (type == "agent_update") {
        QJsonObject agentData = message["data"].toObject();
        QString agentId = agentData["id"].toString();
        m_agentsModel->updateAgent(agentId, agentData.toVariantMap());
        emit agentUpdated(agentId);
    }
    else if (type == "agent_remove") {
        QString agentId = message["agent_id"].toString();
        m_agentsModel->removeAgent(agentId);
        emit agentRemoved(agentId);
    }
    else if (type == "agent_task") {
        QJsonObject taskData = message["data"].toObject();
        QString agentId = taskData["agent_id"].toString();
        QString taskId = taskData["task_id"].toString();
        QString output = taskData["output"].toString();
        emit commandExecuted(agentId, taskId);
        if (!output.isEmpty()) {
            emit commandOutput(agentId, output);
        }
    }
    else if (type == "listener_new") {
        QJsonObject listenerData = message["data"].toObject();
        m_listenersModel->addListener(listenerData.toVariantMap());
        emit listenerStarted(listenerData["id"].toString());
    }
    else if (type == "listener_remove") {
        QString listenerId = message["listener_id"].toString();
        m_listenersModel->removeListener(listenerId);
        emit listenerStopped(listenerId);
    }
    else if (type == "listener_error") {
        QString error = message["error"].toString();
        emit listenerError(error);
    }
    else if (type == "tunnel_new") {
        QJsonObject tunnelData = message["data"].toObject();
        m_tunnelsModel->addTunnel(tunnelData.toVariantMap());
        emit tunnelCreated(tunnelData["id"].toString());
    }
    else if (type == "tunnel_remove") {
        QString tunnelId = message["tunnel_id"].toString();
        m_tunnelsModel->removeTunnel(tunnelId);
        emit tunnelClosed(tunnelId);
    }
    else if (type == "tunnel_error") {
        QString tunnelId = message["tunnel_id"].toString();
        QString error = message["error"].toString();
        emit tunnelError(tunnelId, error);
    }
    else if (type == "download_new") {
        QJsonObject downloadData = message["data"].toObject();
        m_downloadsModel->addDownload(downloadData.toVariantMap());
    }
    else if (type == "download_update") {
        QJsonObject downloadData = message["data"].toObject();
        QString downloadId = downloadData["id"].toString();
        qint64 received = downloadData["received"].toVariant().toLongLong();
        qint64 total = downloadData["total"].toVariant().toLongLong();
        m_downloadsModel->updateDownload(downloadId, downloadData.toVariantMap());
        emit fileDownloadProgress(downloadId, received, total);
    }
    else if (type == "download_complete") {
        QString downloadId = message["download_id"].toString();
        QString path = message["path"].toString();
        m_downloadsModel->updateDownloadStatus(downloadId, "completed");
        emit fileDownloadComplete(downloadId, path);
    }
    else if (type == "download_remove") {
        QString downloadId = message["download_id"].toString();
        m_downloadsModel->removeDownload(downloadId);
    }
    else if (type == "credential_new") {
        QJsonObject credData = message["data"].toObject();
        m_credentialsModel->addCredential(credData.toVariantMap());
        emit credentialAdded(credData["id"].toString());
    }
    else if (type == "credential_remove") {
        QString credId = message["cred_id"].toString();
        m_credentialsModel->removeCredential(credId);
        emit credentialRemoved(credId);
    }
    else if (type == "target_new") {
        QJsonObject targetData = message["data"].toObject();
        m_targetsModel->addTarget(targetData.toVariantMap());
        emit targetAdded(targetData["id"].toString());
    }
    else if (type == "target_update") {
        QJsonObject targetData = message["data"].toObject();
        QString targetId = targetData["id"].toString();
        m_targetsModel->updateTarget(targetId, targetData.toVariantMap());
        emit targetUpdated(targetId);
    }
    else if (type == "target_remove") {
        QString targetId = message["target_id"].toString();
        m_targetsModel->removeTarget(targetId);
        emit targetRemoved(targetId);
    }
    else if (type == "chat_message") {
        QString user = message["username"].toString();
        QString msg = message["message"].toString();
        QString timestamp = message["timestamp"].toString();
        emit chatMessageReceived(user, msg, timestamp);
    }
    else if (type == "file_upload_progress") {
        QString agentId = message["agent_id"].toString();
        QString path = message["path"].toString();
        qint64 sent = message["sent"].toVariant().toLongLong();
        qint64 total = message["total"].toVariant().toLongLong();
        emit fileUploadProgress(agentId, path, sent, total);
    }
    else if (type == "file_upload_complete") {
        QString agentId = message["agent_id"].toString();
        QString path = message["path"].toString();
        emit fileUploadComplete(agentId, path);
    }
    else if (type == "error") {
        QString error = message["message"].toString();
        emit notification(tr("Server Error"), error, "error");
    }
}

void Backend::sendRequest(const QString& type, const QJsonObject& data)
{
    if (!m_connected) {
        emit notification(tr("Error"), tr("Not connected to server"), "error");
        return;
    }

    QJsonObject request;
    request["type"] = type;
    request["data"] = data;
    request["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    m_webSocketClient->sendMessage(request);
}
