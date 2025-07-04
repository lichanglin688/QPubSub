#include "IoPacker.h"
#include <QDebug>
#include <QDataStream>
#include <QtEndian>
#include <QThread>

IoPacker::IoPacker(QIODevice* device, QObject* parent)
	: QObject(parent)
	, m_device(device)
	, m_expectedSize(0)
	, m_alreadySize(0)
{
	connect(m_device, &QIODevice::readyRead, this, &IoPacker::onReadyRead);
}

QByteArray IoPacker::createSubscribe(const QString& topic)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << (quint32)0;
	stream << static_cast<quint8>(Command::Subscribe);
	stream << topic;
	stream.device()->seek(0);
	quint32 size = buffer.size() - sizeof(quint32);
	stream << size;
	return buffer;
}

QByteArray IoPacker::createUnsubscribe(const QString& topic)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << (quint32)0;
	stream << static_cast<quint8>(Command::Unsubscribe);
	stream << topic;
	stream.device()->seek(0);
	quint32 size = buffer.size() - sizeof(quint32);
	stream << size;
	return buffer;
}

QByteArray IoPacker::createPublish(const QString& topic, const QByteArray& message)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << (quint32)0;
	stream << static_cast<quint8>(Command::Publish);
	stream << topic;
	stream.writeRawData(message.data(), message.size());
	stream.device()->seek(0);
	quint32 size = buffer.size() - sizeof(quint32);
	stream << size;
	return buffer;
}

void IoPacker::onReadyRead()
{
	const int headerSize = sizeof(quint32);
	QByteArray data = m_device->readAll();
	m_buffer.append(data);
	int currentPos = 0;
	while (m_buffer.size() - currentPos > headerSize)
	{
		quint32 size = qFromBigEndian<quint32>(m_buffer.constData() + currentPos);
		quint32 allSize = size + headerSize;
		if (m_buffer.size() - currentPos < allSize)
		{
			break;
		}
		QByteArray message = m_buffer.mid(currentPos, allSize);
		processMessage(message);
		currentPos += allSize;
	}
	if (currentPos > 0)
	{
		m_buffer.remove(0, currentPos);
	}
}

void IoPacker::processMessage(QByteArray data)
{
	QDataStream stream(&data, QIODevice::ReadOnly);
	stream.device()->seek(sizeof(quint32));
	qint8 command = 0;
	QString topic;
	stream >> command;
	stream >> topic;
	Command cmd = static_cast<Command>(command);
	switch (cmd)
	{
		case Command::Subscribe:
		{
			emit subscribeRequested(topic);
			break;
		}
		case Command::Unsubscribe:
		{
			emit unsubscribeRequested(topic);
			break;
		}
		case Command::Publish:
		{
			emit publishRequested(topic, data);
			int pos = stream.device()->pos();
			QByteArray message = data.mid(stream.device()->pos());
			emit messageReceived(topic, message);
			break;
		}
		default:
		{
			qDebug() << "Unknown command:" << command;
			break;
		}
	}
}