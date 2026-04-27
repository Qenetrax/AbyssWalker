#include "agentmodel.h"

AgentModel::AgentModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AgentModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_agents.count();
}

QVariant AgentModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_agents.count())
        return QVariant();

    const QVariantMap& agent = m_agents.at(index.row());

    switch (role) {
    case IdRole:
        return agent.value("id");
    case NameRole:
        return agent.value("name");
    case ListenerRole:
        return agent.value("listener");
    case AsyncRole:
        return agent.value("async");
    case ExternalIPRole:
        return agent.value("external_ip");
    case InternalIPRole:
        return agent.value("internal_ip");
    case GmtOffsetRole:
        return agent.value("gmt_offset");
    case AcpRole:
        return agent.value("acp");
    case OemCpRole:
        return agent.value("oem_cp");
    case KillDateRole:
        return agent.value("kill_date");
    case WorkingTimeRole:
        return agent.value("working_time");
    case SleepRole:
        return agent.value("sleep");
    case JitterRole:
        return agent.value("jitter");
    case PidRole:
        return agent.value("pid");
    case TidRole:
        return agent.value("tid");
    case ArchRole:
        return agent.value("arch");
    case ElevatedRole:
        return agent.value("elevated");
    case ProcessRole:
        return agent.value("process");
    case OsRole:
        return agent.value("os");
    case OsDescRole:
        return agent.value("os_desc");
    case DomainRole:
        return agent.value("domain");
    case ComputerRole:
        return agent.value("computer");
    case UsernameRole:
        return agent.value("username");
    case ImpersonatedRole:
        return agent.value("impersonated");
    case TagsRole:
        return agent.value("tags");
    case MarkRole:
        return agent.value("mark");
    case ColorRole:
        return agent.value("color");
    case LastTickRole:
        return agent.value("last_tick");
    case DateRole:
        return agent.value("date");
    case DateTimestampRole:
        return agent.value("date_timestamp");
    case ActiveRole:
        return agent.value("active", true);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AgentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "agentId";
    roles[NameRole] = "agentName";
    roles[ListenerRole] = "listener";
    roles[AsyncRole] = "async";
    roles[ExternalIPRole] = "externalIp";
    roles[InternalIPRole] = "internalIp";
    roles[GmtOffsetRole] = "gmtOffset";
    roles[AcpRole] = "acp";
    roles[OemCpRole] = "oemCp";
    roles[KillDateRole] = "killDate";
    roles[WorkingTimeRole] = "workingTime";
    roles[SleepRole] = "sleep";
    roles[JitterRole] = "jitter";
    roles[PidRole] = "pid";
    roles[TidRole] = "tid";
    roles[ArchRole] = "arch";
    roles[ElevatedRole] = "elevated";
    roles[ProcessRole] = "process";
    roles[OsRole] = "os";
    roles[OsDescRole] = "osDesc";
    roles[DomainRole] = "domain";
    roles[ComputerRole] = "computer";
    roles[UsernameRole] = "username";
    roles[ImpersonatedRole] = "impersonated";
    roles[TagsRole] = "tags";
    roles[MarkRole] = "mark";
    roles[ColorRole] = "color";
    roles[LastTickRole] = "lastTick";
    roles[DateRole] = "date";
    roles[DateTimestampRole] = "dateTimestamp";
    roles[ActiveRole] = "active";
    return roles;
}

QVariantMap AgentModel::get(int index) const
{
    if (index < 0 || index >= m_agents.count())
        return QVariantMap();
    return m_agents.at(index);
}

QVariantMap AgentModel::getAllAgents() const
{
    QVariantMap result;
    for (int i = 0; i < m_agents.count(); ++i) {
        QString agentId = m_agents.at(i).value("id").toString();
        result[agentId] = m_agents.at(i);
    }
    return result;
}

int AgentModel::findIndex(const QString& agentId) const
{
    return m_idIndexMap.value(agentId, -1);
}

void AgentModel::addAgent(const QVariantMap& agent)
{
    QString agentId = agent.value("id").toString();
    if (agentId.isEmpty())
        return;

    if (m_idIndexMap.contains(agentId)) {
        updateAgent(agentId, agent);
        return;
    }

    beginInsertRows(QModelIndex(), m_agents.count(), m_agents.count());
    m_agents.append(agent);
    m_idIndexMap[agentId] = m_agents.count() - 1;
    endInsertRows();

    emit countChanged(m_agents.count());
    emit agentAdded(agentId);
}

void AgentModel::updateAgent(const QString& agentId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(agentId, -1);
    if (index < 0 || index >= m_agents.count())
        return;

    QVariantMap updatedAgent = m_agents.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedAgent[it.key()] = it.value();
    }
    m_agents[index] = updatedAgent;

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);

    emit agentUpdated(agentId);
}

void AgentModel::removeAgent(const QString& agentId)
{
    int index = m_idIndexMap.value(agentId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_agents.removeAt(index);
    m_idIndexMap.remove(agentId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_agents.count(); ++i) {
        QString id = m_agents.at(i).value("id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_agents.count());
    emit agentRemoved(agentId);
}

void AgentModel::clear()
{
    if (m_agents.isEmpty())
        return;

    beginResetModel();
    m_agents.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap AgentModel::getAgent(const QString& agentId) const
{
    int index = m_idIndexMap.value(agentId, -1);
    if (index < 0 || index >= m_agents.count())
        return QVariantMap();
    return m_agents.at(index);
}
