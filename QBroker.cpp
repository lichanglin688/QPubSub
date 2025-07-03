#include "QBroker.h"
#include "SocketSession.h"
#include <QDebug>

QBroker::QBroker(QObject* parent)
	: SocketServer(parent),
	m_threadPool(new ThreadPoolManager(QThread::idealThreadCount(), this))
{
	m_threadPool->startAll();
}

QBroker::~QBroker()
{
	stopServer();
	delete m_threadPool;
}

void QBroker::stopServer()
{
	m_threadPool->stopAll();
	close();
}

void QBroker::incomingConnection(SocketDescriptor socketDescriptor)
{
	QThread* thread = m_threadPool->getNextThread();
	SocketSession* session = new SocketSession(socketDescriptor);
	session->moveToThread(thread);

	connect(session, &SocketSession::disconnected, this, &QBroker::handleClientDisconnected, Qt::QueuedConnection);
	connect(session, &SocketSession::subscribeRequested, this, &QBroker::processSubscribe, Qt::QueuedConnection);
	connect(session, &SocketSession::unsubscribeRequested, this, &QBroker::processUnsubscribe, Qt::QueuedConnection);
	connect(session, &SocketSession::publishRequested, this, &QBroker::processPublish, Qt::QueuedConnection);

	QMetaObject::invokeMethod(session, "start", Qt::QueuedConnection);

	//emit newConnection();
}

void QBroker::handleClientDisconnected(SocketSession* socket)
{
	auto& topics = socketSubscriptions.find(socket);
	if (topics == socketSubscriptions.end())
	{
		return;
	}
	QStringList emptyTopics;
	for (const QString& topic : topics.value())
	{
		auto& sockets = topicSubscribers.find(topic);
		if (sockets == topicSubscribers.end())
		{
			continue;
		}
		sockets->remove(socket);
		if (sockets->empty())
		{
			emptyTopics << topic;
		}
	}
	for (const QString& topic : emptyTopics)
	{
		topicSubscribers.remove(topic);
	}
	socketSubscriptions.remove(socket);
}

void QBroker::processSubscribe(SocketSession* socket, const QString& topic)
{
	topicSubscribers[topic].insert(socket);
	socketSubscriptions[socket].insert(topic);
	//qDebug() << "QBroker Client subscribed to" << topic;
}

void QBroker::processUnsubscribe(SocketSession* socket, const QString& topic)
{
	auto& iter = topicSubscribers.find(topic);
	if (iter == topicSubscribers.end())
	{
		return;
	}
	iter->remove(socket);
	if (iter->empty())
	{
		topicSubscribers.remove(topic);
	}
	QSet<QString>& topics = socketSubscriptions.find(socket).value();
	topics.remove(topic);
	if (topics.empty())
	{
		socketSubscriptions.remove(socket);
	}
	//qDebug() << "QBroker Client unsubscribed from" << topic;
}

void QBroker::processPublish(SocketSession* socket, const QString& topic, const QByteArray& frame)
{
	if (!topicSubscribers.contains(topic))
	{
		return;
	}
	for (SocketSession* socket_ : topicSubscribers[topic])
	{
		if (socket_ != socket)
		{
			QMetaObject::invokeMethod(socket_, "write", Qt::QueuedConnection, Q_ARG(const QByteArray&, frame));
		}
	}
}