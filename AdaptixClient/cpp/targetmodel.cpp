#include "targetmodel.h"

TargetModel::TargetModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TargetModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_targets.count();
}

QVariant TargetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_targets.count())
        return QVariant();

    const QVariantMap& target = m_targets.at(index.row());

    switch (role) {
    case TargetIdRole:
        return target.value("target_id");
    case ComputerRole:
        return target.value("computer");
    case DomainRole:
        return target.value("domain");
    case AddressRole:
        return target.value("address");
    case TagRole:
        return target.value("tag");
    case OsRole:
        return target.value("os");
    case OsDescRole:
        return target.value("os_desc");
    case DateRole:
        return target.value("date");
    case DateTimestampRole:
        return target.value("date_timestamp");
    case InfoRole:
        return target.value("info");
    case AliveRole:
        return target.value("alive", false);
    case AgentsRole:
        return target.value("agents");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TargetModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TargetIdRole] = "targetId";
    roles[ComputerRole] = "computer";
    roles[DomainRole] = "domain";
    roles[AddressRole] = "address";
    roles[TagRole] = "tag";
    roles[OsRole] = "os";
    roles[OsDescRole] = "osDesc";
    roles[DateRole] = "date";
    roles[DateTimestampRole] = "dateTimestamp";
    roles[InfoRole] = "info";
    roles[AliveRole] = "alive";
    roles[AgentsRole] = "agents";
    return roles;
}

QVariantMap TargetModel::get(int index) const
{
    if (index < 0 || index >= m_targets.count())
        return QVariantMap();
    return m_targets.at(index);
}

QVariantMap TargetModel::getAllTargets() const
{
    QVariantMap result;
    for (int i = 0; i < m_targets.count(); ++i) {
        QString targetId = m_targets.at(i).value("target_id").toString();
        result[targetId] = m_targets.at(i);
    }
    return result;
}

int TargetModel::findIndex(const QString& targetId) const
{
    return m_idIndexMap.value(targetId, -1);
}

void TargetModel::addTarget(const QVariantMap& target)
{
    QString targetId = target.value("target_id").toString();
    if (targetId.isEmpty())
        return;

    if (m_idIndexMap.contains(targetId)) {
        updateTarget(targetId, target);
        return;
    }

    beginInsertRows(QModelIndex(), m_targets.count(), m_targets.count());
    m_targets.append(target);
    m_idIndexMap[targetId] = m_targets.count() - 1;
    endInsertRows();

    emit countChanged(m_targets.count());
    emit targetAdded(targetId);
}

void TargetModel::updateTarget(const QString& targetId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(targetId, -1);
    if (index < 0 || index >= m_targets.count())
        return;

    QVariantMap updatedTarget = m_targets.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedTarget[it.key()] = it.value();
    }
    m_targets[index] = updatedTarget;

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);

    emit targetUpdated(targetId);
}

void TargetModel::removeTarget(const QString& targetId)
{
    int index = m_idIndexMap.value(targetId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_targets.removeAt(index);
    m_idIndexMap.remove(targetId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_targets.count(); ++i) {
        QString id = m_targets.at(i).value("target_id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_targets.count());
    emit targetRemoved(targetId);
}

void TargetModel::clear()
{
    if (m_targets.isEmpty())
        return;

    beginResetModel();
    m_targets.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap TargetModel::getTarget(const QString& targetId) const
{
    int index = m_idIndexMap.value(targetId, -1);
    if (index < 0 || index >= m_targets.count())
        return QVariantMap();
    return m_targets.at(index);
}