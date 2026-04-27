#include "credentialmodel.h"

CredentialModel::CredentialModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int CredentialModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_credentials.count();
}

int CredentialModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant CredentialModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_credentials.count())
        return QVariant();

    const QVariantMap& cred = m_credentials.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnUsername:
            return cred.value("username");
        case ColumnPassword:
            return cred.value("password");
        case ColumnRealm:
            return cred.value("realm");
        case ColumnType:
            return cred.value("type");
        case ColumnTag:
            return cred.value("tag");
        case ColumnDate:
            return cred.value("date");
        case ColumnHost:
            return cred.value("host");
        default:
            return QVariant();
        }
    }

    switch (role) {
    case CredIdRole:
        return cred.value("cred_id");
    case UsernameRole:
        return cred.value("username");
    case PasswordRole:
        return cred.value("password");
    case RealmRole:
        return cred.value("realm");
    case TypeRole:
        return cred.value("type");
    case TagRole:
        return cred.value("tag");
    case DateRole:
        return cred.value("date");
    case DateTimestampRole:
        return cred.value("date_timestamp");
    case StorageRole:
        return cred.value("storage");
    case AgentIdRole:
        return cred.value("agent_id");
    case HostRole:
        return cred.value("host");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CredentialModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CredIdRole] = "credId";
    roles[UsernameRole] = "username";
    roles[PasswordRole] = "password";
    roles[RealmRole] = "realm";
    roles[TypeRole] = "type";
    roles[TagRole] = "tag";
    roles[DateRole] = "date";
    roles[DateTimestampRole] = "dateTimestamp";
    roles[StorageRole] = "storage";
    roles[AgentIdRole] = "agentId";
    roles[HostRole] = "host";
    return roles;
}

QVariant CredentialModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case ColumnUsername:
        return tr("Username");
    case ColumnPassword:
        return tr("Password");
    case ColumnRealm:
        return tr("Realm");
    case ColumnType:
        return tr("Type");
    case ColumnTag:
        return tr("Tag");
    case ColumnDate:
        return tr("Date");
    case ColumnHost:
        return tr("Host");
    default:
        return QVariant();
    }
}

QVariantMap CredentialModel::get(int index) const
{
    if (index < 0 || index >= m_credentials.count())
        return QVariantMap();
    return m_credentials.at(index);
}

QVariantMap CredentialModel::getAllCredentials() const
{
    QVariantMap result;
    for (int i = 0; i < m_credentials.count(); ++i) {
        QString credId = m_credentials.at(i).value("cred_id").toString();
        result[credId] = m_credentials.at(i);
    }
    return result;
}

int CredentialModel::findIndex(const QString& credId) const
{
    return m_idIndexMap.value(credId, -1);
}

void CredentialModel::addCredential(const QVariantMap& credential)
{
    QString credId = credential.value("cred_id").toString();
    if (credId.isEmpty())
        return;

    if (m_idIndexMap.contains(credId)) {
        updateCredential(credId, credential);
        return;
    }

    beginInsertRows(QModelIndex(), m_credentials.count(), m_credentials.count());
    m_credentials.append(credential);
    m_idIndexMap[credId] = m_credentials.count() - 1;
    endInsertRows();

    emit countChanged(m_credentials.count());
    emit credentialAdded(credId);
}

void CredentialModel::updateCredential(const QString& credId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(credId, -1);
    if (index < 0 || index >= m_credentials.count())
        return;

    QVariantMap updatedCred = m_credentials.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedCred[it.key()] = it.value();
    }
    m_credentials[index] = updatedCred;

    emit dataChanged(createIndex(index, 0), createIndex(index, ColumnCount - 1));

    emit credentialUpdated(credId);
}

void CredentialModel::removeCredential(const QString& credId)
{
    int index = m_idIndexMap.value(credId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_credentials.removeAt(index);
    m_idIndexMap.remove(credId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_credentials.count(); ++i) {
        QString id = m_credentials.at(i).value("cred_id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_credentials.count());
    emit credentialRemoved(credId);
}

void CredentialModel::clear()
{
    if (m_credentials.isEmpty())
        return;

    beginResetModel();
    m_credentials.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap CredentialModel::getCredential(const QString& credId) const
{
    int index = m_idIndexMap.value(credId, -1);
    if (index < 0 || index >= m_credentials.count())
        return QVariantMap();
    return m_credentials.at(index);
}