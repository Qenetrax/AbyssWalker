#ifndef ADAPTIXCLIENT_TUNNELMODEL_H
#define ADAPTIXCLIENT_TUNNELMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

class TunnelModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum TunnelRoles {
        TunnelIdRole = Qt::UserRole + 1,
        AgentIdRole,
        ComputerRole,
        UsernameRole,
        ProcessRole,
        TypeRole,
        InfoRole,
        InterfaceRole,
        PortRole,
        ClientRole,
        FportRole,
        FhostRole
    };

    explicit TunnelModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllTunnels() const;
    Q_INVOKABLE int findIndex(const QString& tunnelId) const;

    void addTunnel(const QVariantMap& tunnel);
    void updateTunnel(const QString& tunnelId, const QVariantMap& data);
    void removeTunnel(const QString& tunnelId);
    void clear();

    QVariantMap getTunnel(const QString& tunnelId) const;

signals:
    void countChanged(int count);
    void tunnelAdded(const QString& tunnelId);
    void tunnelUpdated(const QString& tunnelId);
    void tunnelRemoved(const QString& tunnelId);

private:
    QList<QVariantMap> m_tunnels;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_TUNNELMODEL_H