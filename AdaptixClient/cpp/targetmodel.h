#ifndef ADAPTIXCLIENT_TARGETMODEL_H
#define ADAPTIXCLIENT_TARGETMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

class TargetModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum TargetRoles {
        TargetIdRole = Qt::UserRole + 1,
        ComputerRole,
        DomainRole,
        AddressRole,
        TagRole,
        OsRole,
        OsDescRole,
        DateRole,
        DateTimestampRole,
        InfoRole,
        AliveRole,
        AgentsRole
    };

    explicit TargetModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllTargets() const;
    Q_INVOKABLE int findIndex(const QString& targetId) const;

    void addTarget(const QVariantMap& target);
    void updateTarget(const QString& targetId, const QVariantMap& data);
    void removeTarget(const QString& targetId);
    void clear();

    QVariantMap getTarget(const QString& targetId) const;

signals:
    void countChanged(int count);
    void targetAdded(const QString& targetId);
    void targetUpdated(const QString& targetId);
    void targetRemoved(const QString& targetId);

private:
    QList<QVariantMap> m_targets;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_TARGETMODEL_H