#include "Client.h"
#include "IoPacker.h"
#include <QDataStream>

Client::Client(QObject* parent)
	: Socket(parent), m_ioPacker(nullptr)
{
	m_ioPacker = new IoPacker(this, this);
	connect(m_ioPacker, &IoPacker::messageReceived, this, [this] (const QString& topic, const QByteArray& message) {
		emit messageReceived(topic, message);
	});
}

void Client::publish(const QString& topic, const QByteArray& message)
{
	write(IoPacker::createPublish(topic, message));
}

void Client::subscribe(const QString& topic)
{
	write(IoPacker::createSubscribe(topic));
}

void Client::unsubscribe(const QString& topic)
{
	write(IoPacker::createUnsubscribe(topic));
}

