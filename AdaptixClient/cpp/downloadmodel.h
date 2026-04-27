#ifndef ADAPTIXCLIENT_DOWNLOADMODEL_H
#define ADAPTIXCLIENT_DOWNLOADMODEL_H

#include <QAbstractTableModel>
#include <QVariantMap>
#include <QList>

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum DownloadRoles {
        FileIdRole = Qt::UserRole + 1,
        AgentIdRole,
        AgentNameRole,
        UserRole,
        ComputerRole,
        FilenameRole,
        TotalSizeRole,
        RecvSizeRole,
        StateRole,
        DateRole,
        DateTimestampRole,
        ProgressRole
    };

    enum ColumnRoles {
        ColumnFilename = 0,
        ColumnAgent,
        ColumnSize,
        ColumnProgress,
        ColumnState,
        ColumnDate,
        ColumnCount
    };

    explicit DownloadModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllDownloads() const;
    Q_INVOKABLE int findIndex(const QString& fileId) const;

    void addDownload(const QVariantMap& download);
    void updateDownload(const QString& fileId, const QVariantMap& data);
    void updateDownloadStatus(const QString& fileId, const QString& status);
    void removeDownload(const QString& fileId);
    void clear();

    QVariantMap getDownload(const QString& fileId) const;

signals:
    void countChanged(int count);
    void downloadAdded(const QString& fileId);
    void downloadUpdated(const QString& fileId);
    void downloadRemoved(const QString& fileId);

private:
    QList<QVariantMap> m_downloads;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_DOWNLOADMODEL_H