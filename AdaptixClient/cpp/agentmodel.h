#ifndef ADAPTIXCLIENT_AGENTMODEL_H
#define ADAPTIXCLIENT_AGENTMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

class AgentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum AgentRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ListenerRole,
        AsyncRole,
        ExternalIPRole,
        InternalIPRole,
        GmtOffsetRole,
        AcpRole,
        OemCpRole,
        KillDateRole,
        WorkingTimeRole,
        SleepRole,
        JitterRole,
        PidRole,
        TidRole,
        ArchRole,
        ElevatedRole,
        ProcessRole,
        OsRole,
        OsDescRole,
        DomainRole,
        ComputerRole,
        UsernameRole,
        ImpersonatedRole,
        TagsRole,
        MarkRole,
        ColorRole,
        LastTickRole,
        DateRole,
        DateTimestampRole,
        ActiveRole
    };

    explicit AgentModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllAgents() const;
    Q_INVOKABLE int findIndex(const QString& agentId) const;

    void addAgent(const QVariantMap& agent);
    void updateAgent(const QString& agentId, const QVariantMap& data);
    void removeAgent(const QString& agentId);
    void clear();

    QVariantMap getAgent(const QString& agentId) const;

signals:
    void countChanged(int count);
    void agentAdded(const QString& agentId);
    void agentUpdated(const QString& agentId);
    void agentRemoved(const QString& agentId);

private:
    QList<QVariantMap> m_agents;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_AGENTMODEL_H
