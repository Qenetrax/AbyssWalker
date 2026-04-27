#ifndef ADAPTIXCLIENT_CREDENTIALMODEL_H
#define ADAPTIXCLIENT_CREDENTIALMODEL_H

#include <QAbstractTableModel>
#include <QVariantMap>
#include <QList>

class CredentialModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum CredentialRoles {
        CredIdRole = Qt::UserRole + 1,
        UsernameRole,
        PasswordRole,
        RealmRole,
        TypeRole,
        TagRole,
        DateRole,
        DateTimestampRole,
        StorageRole,
        AgentIdRole,
        HostRole
    };

    enum ColumnRoles {
        ColumnUsername = 0,
        ColumnPassword,
        ColumnRealm,
        ColumnType,
        ColumnTag,
        ColumnDate,
        ColumnHost,
        ColumnCount
    };

    explicit CredentialModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllCredentials() const;
    Q_INVOKABLE int findIndex(const QString& credId) const;

    void addCredential(const QVariantMap& credential);
    void updateCredential(const QString& credId, const QVariantMap& data);
    void removeCredential(const QString& credId);
    void clear();

    QVariantMap getCredential(const QString& credId) const;

signals:
    void countChanged(int count);
    void credentialAdded(const QString& credId);
    void credentialUpdated(const QString& credId);
    void credentialRemoved(const QString& credId);

private:
    QList<QVariantMap> m_credentials;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_CREDENTIALMODEL_H