#include "downloadmodel.h"

DownloadModel::DownloadModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int DownloadModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_downloads.count();
}

int DownloadModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant DownloadModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_downloads.count())
        return QVariant();

    const QVariantMap& download = m_downloads.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnFilename:
            return download.value("filename");
        case ColumnAgent:
            return download.value("agent_name");
        case ColumnSize:
            return download.value("total_size");
        case ColumnProgress: {
            qint64 total = download.value("total_size").toLongLong();
            qint64 recv = download.value("recv_size").toLongLong();
            if (total > 0)
                return QString("%1%").arg(qRound(recv * 100.0 / total));
            return "0%";
        }
        case ColumnState:
            return download.value("state");
        case ColumnDate:
            return download.value("date");
        default:
            return QVariant();
        }
    }

    switch (role) {
    case FileIdRole:
        return download.value("file_id");
    case AgentIdRole:
        return download.value("agent_id");
    case AgentNameRole:
        return download.value("agent_name");
    case UserRole:
        return download.value("user");
    case ComputerRole:
        return download.value("computer");
    case FilenameRole:
        return download.value("filename");
    case TotalSizeRole:
        return download.value("total_size");
    case RecvSizeRole:
        return download.value("recv_size");
    case StateRole:
        return download.value("state");
    case DateRole:
        return download.value("date");
    case DateTimestampRole:
        return download.value("date_timestamp");
    case ProgressRole: {
        qint64 total = download.value("total_size").toLongLong();
        qint64 recv = download.value("recv_size").toLongLong();
        if (total > 0)
            return static_cast<double>(recv) / total;
        return 0.0;
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DownloadModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FileIdRole] = "fileId";
    roles[AgentIdRole] = "agentId";
    roles[AgentNameRole] = "agentName";
    roles[UserRole] = "user";
    roles[ComputerRole] = "computer";
    roles[FilenameRole] = "filename";
    roles[TotalSizeRole] = "totalSize";
    roles[RecvSizeRole] = "recvSize";
    roles[StateRole] = "state";
    roles[DateRole] = "date";
    roles[DateTimestampRole] = "dateTimestamp";
    roles[ProgressRole] = "progress";
    return roles;
}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case ColumnFilename:
        return tr("Filename");
    case ColumnAgent:
        return tr("Agent");
    case ColumnSize:
        return tr("Size");
    case ColumnProgress:
        return tr("Progress");
    case ColumnState:
        return tr("State");
    case ColumnDate:
        return tr("Date");
    default:
        return QVariant();
    }
}

QVariantMap DownloadModel::get(int index) const
{
    if (index < 0 || index >= m_downloads.count())
        return QVariantMap();
    return m_downloads.at(index);
}

QVariantMap DownloadModel::getAllDownloads() const
{
    QVariantMap result;
    for (int i = 0; i < m_downloads.count(); ++i) {
        QString fileId = m_downloads.at(i).value("file_id").toString();
        result[fileId] = m_downloads.at(i);
    }
    return result;
}

int DownloadModel::findIndex(const QString& fileId) const
{
    return m_idIndexMap.value(fileId, -1);
}

void DownloadModel::addDownload(const QVariantMap& download)
{
    QString fileId = download.value("file_id").toString();
    if (fileId.isEmpty())
        return;

    if (m_idIndexMap.contains(fileId)) {
        updateDownload(fileId, download);
        return;
    }

    beginInsertRows(QModelIndex(), m_downloads.count(), m_downloads.count());
    m_downloads.append(download);
    m_idIndexMap[fileId] = m_downloads.count() - 1;
    endInsertRows();

    emit countChanged(m_downloads.count());
    emit downloadAdded(fileId);
}

void DownloadModel::updateDownload(const QString& fileId, const QVariantMap& data)
{
    int index = m_idIndexMap.value(fileId, -1);
    if (index < 0 || index >= m_downloads.count())
        return;

    QVariantMap updatedDownload = m_downloads.at(index);
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        updatedDownload[it.key()] = it.value();
    }
    m_downloads[index] = updatedDownload;

    emit dataChanged(createIndex(index, 0), createIndex(index, ColumnCount - 1));

    emit downloadUpdated(fileId);
}

void DownloadModel::updateDownloadStatus(const QString& fileId, const QString& status)
{
    int index = m_idIndexMap.value(fileId, -1);
    if (index < 0 || index >= m_downloads.count())
        return;

    m_downloads[index]["state"] = status;

    emit dataChanged(createIndex(index, ColumnState), createIndex(index, ColumnState));

    emit downloadUpdated(fileId);
}

void DownloadModel::removeDownload(const QString& fileId)
{
    int index = m_idIndexMap.value(fileId, -1);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_downloads.removeAt(index);
    m_idIndexMap.remove(fileId);

    // Rebuild index map
    m_idIndexMap.clear();
    for (int i = 0; i < m_downloads.count(); ++i) {
        QString id = m_downloads.at(i).value("file_id").toString();
        m_idIndexMap[id] = i;
    }
    endRemoveRows();

    emit countChanged(m_downloads.count());
    emit downloadRemoved(fileId);
}

void DownloadModel::clear()
{
    if (m_downloads.isEmpty())
        return;

    beginResetModel();
    m_downloads.clear();
    m_idIndexMap.clear();
    endResetModel();

    emit countChanged(0);
}

QVariantMap DownloadModel::getDownload(const QString& fileId) const
{
    int index = m_idIndexMap.value(fileId, -1);
    if (index < 0 || index >= m_downloads.count())
        return QVariantMap();
    return m_downloads.at(index);
}