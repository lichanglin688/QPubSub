#pragma once

#include <QSharedPointer>
#include "config.h"

class IoPacker;
class SocketSession : public QObject
{
    Q_OBJECT
public:
    explicit SocketSession(SocketDescriptor socketDescriptor, QObject* parent = nullptr);
signals:
    void disconnected(SocketSession* socket);
    void subscribeRequested(SocketSession* socket, const QString& topic);
    void unsubscribeRequested(SocketSession* socket, const QString& topic);
    void publishRequested(SocketSession* socket, const QString& topic, const QByteArray& message);
public slots:
    void start();
    void write(const QByteArray& frame);
private slots:
    void onDisconnected();
    void onHandshakeError(SocketErrorType error);
private:
    SocketDescriptor m_socketDescriptor;
    Socket* m_socket;
    IoPacker* m_ioPacker;
    quint32 m_expectedSize;
    quint32 m_alreadySize;
    QByteArray m_buffer;
};
