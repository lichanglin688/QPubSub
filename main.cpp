#include <QCoreApplication>
#include <QThread>
#include <QSharedPointer>
#include <QTimer>
#include <QDebug>
#include "QBroker.h"
#include <qtcpsocket.h>
#include <qlocalsocket.h>
#include <QtEndian>
#include <QDataStream>
#include <qsemaphore.h>
#include <QTime>

#include "Client.h"

const int count = 100;

Client* makeClient()
{
	Client* publishSocket = new Client();
#ifdef BROKER_TCP
	publishSocket->connectToHost(QHostAddress::LocalHost, 8888);
#else
	publishSocket->connectToServer("localServer");
#endif // BROKER_TCP
	publishSocket->waitForConnected();
	return publishSocket;
}

class SyncThread
{
public:
	void waitStart()
	{
		s.acquire();
	}
	void initFinished()
	{
		s.release();
	}
private:
	QSemaphore s;
};
class BrokerThread : public QThread, public SyncThread
{
	Q_OBJECT
protected:
	virtual void run()
	{
		QBroker broker;
	#ifdef BROKER_TCP
		broker.listen(QHostAddress::LocalHost, 8888);
	#else
		broker.listen("localServer");
	#endif // BROKER_TCP
		initFinished();
		exec();
	}
};

class SubscribeSocketThread : public QThread, public SyncThread
{
	Q_OBJECT
protected:
	virtual void run()
	{
		Client* subscribeSocket = makeClient();
		subscribeSocket->subscribe("test");
		subscribeSocket->subscribe("test1");
		QTime time;
		QObject::connect(subscribeSocket, &Client::messageReceived, [&] (const QString& topic, const QByteArray& data)
		{
			if (topic == "test1")
			{
				qDebug() << QString::fromUtf8(data);
			}
			else
			{
				int number = QString::fromUtf8(data).toInt();
				if (number == 0)
				{
					time.start();
				}
				qDebug() << "=============messageReceivedCount====================" << number;
				//messageReceivedCount++;
				if (number == count)
				{
					qDebug() << "=================================" << time.elapsed();
				}
			}
		});
		initFinished();
		exec();
	}
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

	BrokerThread thread;
	thread.start();
	thread.waitStart();

	SubscribeSocketThread thread1;
	thread1.start();
	thread1.waitStart();

	{
		Client* publishSocket = makeClient();
		for (size_t i = 0; i <= count; i++)
		{
			QByteArray numberData = QString::number(i).toUtf8();
			publishSocket->publish("test", numberData);
		}
		publishSocket->waitForBytesWritten();
		publishSocket->deleteLater();
	}

	int publish_count = 0;
	QTimer timer;
	QObject::connect(&timer, &QTimer::timeout, [&] ()
	{
		Client* publishSocket = makeClient();
		QByteArray numberData = QString::number(publish_count).toUtf8();
		publishSocket->publish("test1", numberData);
		publishSocket->waitForBytesWritten();
		publishSocket->deleteLater();
		publish_count++;
		if (publish_count == 100)
		{
			timer.stop();
		}
	});
	timer.start(1);
    return app.exec();
}
#include "main.moc"