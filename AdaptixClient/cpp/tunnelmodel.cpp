#include "tunnelmodel.h"

TunnelModel::TunnelModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TunnelModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_tunnels.count();
}

QVariant TunnelModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_tunnels.count())
        return QVariant();

    const QVariantMap& tunnel = m_tunnels.at(index.row());

    switch (role) {
    case TunnelIdRole:
        return tunnel.value("tunnel_id");
    case AgentIdRole:
        return tunnel.value("agent_id");
    case ComputerRole:
        return tunnel.value("computer");
    case UsernameRole:
        return tunnel.value("username");
    case ProcessRole:
        return tunnel.value("process");
    case TypeRole:
        return tunnel.value("type");
    case InfoRole:
        return tunnel.value("info");
    case InterfaceRole:
        return tunnel.value("interface");
    case PortRole:
        return tunnel.value("port");
    case ClientRole:
        return tunnel.value("client");
    case FportRole:
        return tunnel.value("fport");
    case FhostRole:
        return tunnel.value("fhost");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TunnelModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TunnelIdRole] = "tunnelId";
    roles[AgentIdRole] = "agentId";
    roles[ComputerRole] = "computer";
    roles[UsernameRole] = "username";
    roles[ProcessRole] = "process";
    roles[TypeRole] = "type";
    roles[InfoRole] = "info";
    roles[InterfaceRole] = "interface";
    roles[PortRole] = "port";
    roles[ClientRole] = "client";
    roles[FportRole] = "fport";
    roles[FhostRole] = "fhost";
    return roles;
}

QVariantMap TunnelModel::get(int index) const
{
    if (index < 0 || index >= m_tunnels.count())
        return QVariantMap();
    return m_tunnels.at(index);
}

QVariantMap TunnelModel::getAllTunnels() const
{
    QVariantMap result;
    for (int i = 0; i < m_tunnels.count(); ++i) {
        QString tunnelId = m_tunnels.at(i).value("tunnel_id").toString();
        result[tunnelId] = m_tunnels.at(i);
    }
    return result;
}

int TunnelModel::findIndex(const QString& tunnelId) const
{
    return m_idIndexMap.value(tunnelId, -1);
}

void TunnelModel::addTunnel(const QVariantMap& tunnel)
{
    QString tunnelId = tunnel.value("tunnel_id").toString();
    if (tunnelId.isEmpty())
        return;

    if (m_idIndexMap.contains(tunnelId)) {
        updateTunnel(tunnelId, tunnel);
        return;
    }

    beginInsertRows(QModelIndex(), m_tunnels.count(), m_tunnels.count());
    m_tunnels.append(tunnel);
    m_idIndexMap[tunnelId] = m_tunnels.count() - 1;
    endInsertRows();

    emit countChanged(m_tunnels.count());
    emit tunnelAdded(tunnelId);
}

void TunnelModel::updateTunnel(const QString& tunnelId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(tunnelId, -1);
    if (index < 0 || index >= m_tunnels.count())
        return;

    QVariantMap updatedTunnel = m_tunnels.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedTunnel[it.key()] = it.value();
    }
    m_tunnels[index] = updatedTunnel;

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);

    emit tunnelUpdated(tunnelId);
}

void TunnelModel::removeTunnel(const QString& tunnelId)
{
    int index = m_idIndexMap.value(tunnelId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_tunnels.removeAt(index);
    m_idIndexMap.remove(tunnelId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_tunnels.count(); ++i) {
        QString id = m_tunnels.at(i).value("tunnel_id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_tunnels.count());
    emit tunnelRemoved(tunnelId);
}

void TunnelModel::clear()
{
    if (m_tunnels.isEmpty())
        return;

    beginResetModel();
    m_tunnels.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap TunnelModel::getTunnel(const QString& tunnelId) const
{
    int index = m_idIndexMap.value(tunnelId, -1);
    if (index < 0 || index >= m_tunnels.count())
        return QVariantMap();
    return m_tunnels.at(index);
}