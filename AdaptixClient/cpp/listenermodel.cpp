#include "listenermodel.h"

ListenerModel::ListenerModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ListenerModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_listeners.count();
}

QVariant ListenerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_listeners.count())
        return QVariant();

    const QVariantMap& listener = m_listeners.at(index.row());

    switch (role) {
    case IdRole:
        return listener.value("id");
    case NameRole:
        return listener.value("name");
    case ListenerRegNameRole:
        return listener.value("listener_reg_name");
    case ListenerProtocolRole:
        return listener.value("listener_protocol");
    case ListenerTypeRole:
        return listener.value("listener_type");
    case BindHostRole:
        return listener.value("bind_host");
    case BindPortRole:
        return listener.value("bind_port");
    case AgentAddressesRole:
        return listener.value("agent_addresses");
    case DateRole:
        return listener.value("date");
    case DateTimestampRole:
        return listener.value("date_timestamp");
    case StatusRole:
        return listener.value("status");
    case DataRole:
        return listener.value("data");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ListenerModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "listenerId";
    roles[NameRole] = "name";
    roles[ListenerRegNameRole] = "listenerRegName";
    roles[ListenerProtocolRole] = "listenerProtocol";
    roles[ListenerTypeRole] = "listenerType";
    roles[BindHostRole] = "bindHost";
    roles[BindPortRole] = "bindPort";
    roles[AgentAddressesRole] = "agentAddresses";
    roles[DateRole] = "date";
    roles[DateTimestampRole] = "dateTimestamp";
    roles[StatusRole] = "status";
    roles[DataRole] = "data";
    return roles;
}

QVariantMap ListenerModel::get(int index) const
{
    if (index < 0 || index >= m_listeners.count())
        return QVariantMap();
    return m_listeners.at(index);
}

QVariantMap ListenerModel::getAllListeners() const
{
    QVariantMap result;
    for (int i = 0; i < m_listeners.count(); ++i) {
        QString listenerId = m_listeners.at(i).value("id").toString();
        result[listenerId] = m_listeners.at(i);
    }
    return result;
}

int ListenerModel::findIndex(const QString& listenerId) const
{
    return m_idIndexMap.value(listenerId, -1);
}

void ListenerModel::addListener(const QVariantMap& listener)
{
    QString listenerId = listener.value("id").toString();
    if (listenerId.isEmpty())
        return;

    if (m_idIndexMap.contains(listenerId)) {
        updateListener(listenerId, listener);
        return;
    }

    beginInsertRows(QModelIndex(), m_listeners.count(), m_listeners.count());
    m_listeners.append(listener);
    m_idIndexMap[listenerId] = m_listeners.count() - 1;
    endInsertRows();

    emit countChanged(m_listeners.count());
    emit listenerAdded(listenerId);
}

void ListenerModel::updateListener(const QString& listenerId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(listenerId, -1);
    if (index < 0 || index >= m_listeners.count())
        return;

    QVariantMap updatedListener = m_listeners.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedListener[it.key()] = it.value();
    }
    m_listeners[index] = updatedListener;

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);

    emit listenerUpdated(listenerId);
}

void ListenerModel::removeListener(const QString& listenerId)
{
    int index = m_idIndexMap.value(listenerId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_listeners.removeAt(index);
    m_idIndexMap.remove(listenerId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_listeners.count(); ++i) {
        QString id = m_listeners.at(i).value("id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_listeners.count());
    emit listenerRemoved(listenerId);
}

void ListenerModel::clear()
{
    if (m_listeners.isEmpty())
        return;

    beginResetModel();
    m_listeners.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap ListenerModel::getListener(const QString& listenerId) const
{
    int index = m_idIndexMap.value(listenerId, -1);
    if (index < 0 || index >= m_listeners.count())
        return QVariantMap();
    return m_listeners.at(index);
}