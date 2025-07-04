#include "ThreadPoolManager.h"

ThreadPoolManager::ThreadPoolManager(int maxThreads, QObject* parent)
    : QObject(parent), m_currentIndex(0)
{
    for (int i = 0; i < maxThreads; ++i)
    {
        QThread* thread = new QThread(this);
        m_threads.append(thread);
    }
}

ThreadPoolManager::~ThreadPoolManager()
{
    stopAll();
}

QThread* ThreadPoolManager::getNextThread()
{
    QThread* thread = m_threads.at(m_currentIndex);
    m_currentIndex = (m_currentIndex + 1) % m_threads.count();
    return thread;
}

void ThreadPoolManager::startAll()
{
    for (QThread* thread : m_threads)
    {
        thread->start();
    }
}

void ThreadPoolManager::stopAll()
{
    for (QThread* thread : m_threads)
    {
        thread->quit();
        thread->wait();
    }
    qDeleteAll(m_threads);
    m_threads.clear();
}

