#pragma once

#include <QObject>
#include <QIODevice>

enum class Command : quint8
{
	Invalid = 0,
	Subscribe,
	Unsubscribe,
	Publish
};

class IoPacker : public QObject
{
	Q_OBJECT
public:
	IoPacker(QIODevice* device, QObject* parent = nullptr);
	static QByteArray createSubscribe(const QString& topic);
	static QByteArray createUnsubscribe(const QString& topic);
	static QByteArray createPublish(const QString& topic, const QByteArray& message);
signals:
	void subscribeRequested(const QString& topic);
	void unsubscribeRequested(const QString& topic);
	void publishRequested(const QString& topic, const QByteArray& frame);
	void messageReceived(const QString& topic, const QByteArray& message);
private slots:
	void onReadyRead();
private:
	void processMessage(QByteArray frame);
private:
	QByteArray m_buffer; 
	QIODevice* m_device; 
	quint32 m_expectedSize;
	quint32 m_alreadySize;
};

