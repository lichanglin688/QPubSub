#pragma once

#include "config.h"
#include <QByteArray>
#include <QString>

class IoPacker;
class Client  : public Socket
{
	Q_OBJECT
public:
	Client(QObject *parent = nullptr);
	void publish(const QString& topic, const QByteArray& message);
	void subscribe(const QString& topic);
	void unsubscribe(const QString& topic);
signals:
	void messageReceived(const QString& topic, const QByteArray& message);
private:
	IoPacker* m_ioPacker;
};

