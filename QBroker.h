#pragma once

#include <QMap>
#include <QSet>
#include "ThreadPoolManager.h"
#include "config.h"

class SocketSession;
class QBroker : public SocketServer
{
    Q_OBJECT
public:
    explicit QBroker(QObject* parent = nullptr);
    ~QBroker();
    void stopServer();
private slots:
    void handleClientDisconnected(SocketSession* socket);
    void processSubscribe(SocketSession* socket, const QString& topic);
    void processUnsubscribe(SocketSession* socket, const QString& topic);
    void processPublish(SocketSession* socket, const QString& topic, const QByteArray& message);
protected:
    void incomingConnection(SocketDescriptor socketDescriptor) override;
private:
    QMap<QString, QSet<SocketSession*>> topicSubscribers;
    QMap<SocketSession*, QSet<QString>> socketSubscriptions;
    ThreadPoolManager* m_threadPool;
};