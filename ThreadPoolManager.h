#pragma once

#include <QObject>
#include <QList>
#include <QThread>

class ThreadPoolManager : public QObject
{
    Q_OBJECT
public:
    explicit ThreadPoolManager(int maxThreads = 4, QObject* parent = nullptr);
    ~ThreadPoolManager();
    QThread* getNextThread();
    void startAll();
    void stopAll();
private:
    QList<QThread*> m_threads;
    int m_currentIndex;
};

