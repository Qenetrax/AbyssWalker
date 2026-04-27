#ifndef ADAPTIXCLIENT_LISTENERMODEL_H
#define ADAPTIXCLIENT_LISTENERMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

class ListenerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum ListenerRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ListenerRegNameRole,
        ListenerProtocolRole,
        ListenerTypeRole,
        BindHostRole,
        BindPortRole,
        AgentAddressesRole,
        DateRole,
        DateTimestampRole,
        StatusRole,
        DataRole
    };

    explicit ListenerModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getAllListeners() const;
    Q_INVOKABLE int findIndex(const QString& listenerId) const;

    void addListener(const QVariantMap& listener);
    void updateListener(const QString& listenerId, const QVariantMap& data);
    void removeListener(const QString& listenerId);
    void clear();

    QVariantMap getListener(const QString& listenerId) const;

signals:
    void countChanged(int count);
    void listenerAdded(const QString& listenerId);
    void listenerUpdated(const QString& listenerId);
    void listenerRemoved(const QString& listenerId);

private:
    QList<QVariantMap> m_listeners;
    QHash<QString, int> m_idIndexMap;
};

#endif // ADAPTIXCLIENT_LISTENERMODEL_H