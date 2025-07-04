#include "SocketSession.h"
#include "IoPacker.h"
#include <QDebug>
#include <QThread>
#include <QtEndian>
#include <QDataStream>

SocketSession::SocketSession(SocketDescriptor socketDescriptor, QObject* parent)
	: QObject(parent)
	, m_socketDescriptor(socketDescriptor)
	, m_socket(nullptr)
	, m_ioPacker(nullptr)
	, m_expectedSize(0)
	, m_alreadySize(0)
{
}

void SocketSession::start()
{
	m_socket = new Socket(this);
	m_socket->setSocketDescriptor(m_socketDescriptor);
	this->setObjectName(QString::number(m_socketDescriptor));
	connect(m_socket, &Socket::disconnected, this, &SocketSession::onDisconnected);

#if QT_VERSION <= QT_VERSION_CHECK(5, 15, 0)
	connect(m_socket, QOverload<SocketErrorType>::of(&Socket::error), this, &SocketSession::onHandshakeError);
#else
	connect(m_socket, &Socket::errorOccurred, this, &SocketSession::onHandshakeError);
#endif 

	m_ioPacker = new IoPacker(m_socket, this);
	connect(m_ioPacker, &IoPacker::subscribeRequested, this, [this] (const QString& topic)
	{
		emit subscribeRequested(this, topic);
	});
	connect(m_ioPacker, &IoPacker::unsubscribeRequested, this, [this] (const QString& topic)
	{
		emit unsubscribeRequested(this, topic);
	});
	connect(m_ioPacker, &IoPacker::publishRequested, this, [this] (const QString& topic, const QByteArray& message)
	{
		emit publishRequested(this, topic, message);
	});
}

void SocketSession::write(const QByteArray& frame)
{
	m_socket->write(frame);
}

void SocketSession::onDisconnected()
{
	if (m_socket == nullptr)
	{
		return;
	}
	emit disconnected(this);
	deleteLater();
	m_socket = nullptr;
}

void SocketSession::onHandshakeError(SocketErrorType error)
{
	onDisconnected();
}