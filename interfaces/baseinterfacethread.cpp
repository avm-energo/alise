#include "baseinterfacethread.h"

#include <QCoreApplication>
#include <gen/datamanager/typesproxy.h>
#include <thread>

using namespace Interface;

BaseInterfaceThread::BaseInterfaceThread(QObject *parent) : QObject(parent)
{
}

void BaseInterfaceThread::clear()
{
    QMutexLocker locker(&_mutex);
    m_progress = 0;
    m_currentCommand = CommandStruct();
    finishCommand();
}

void BaseInterfaceThread::wakeUp()
{
    _waiter.wakeOne();
}

void BaseInterfaceThread::checkQueue()
{
    CommandStruct inp;
    if (DataManager::GetInstance().deQueue(inp) != Error::Msg::NoError)
        return;

    m_isCommandRequested = true;
    m_progress = 0;
    m_currentCommand = inp;
    parseRequest(inp);
}

void BaseInterfaceThread::finishCommand()
{
    m_isCommandRequested = false;
    m_readData.clear();
    wakeUp();
}

void BaseInterfaceThread::run()
{
    while (BaseInterface::iface()->state() != State::Disconnect)
    {
        QMutexLocker locker(&_mutex);
        if (!m_isCommandRequested)
            checkQueue();
        _waiter.wait(&_mutex, 100);
        if (m_parsingDataReady)
        {
            parseResponse();
            m_readData.clear();
            m_parsingDataReady = false;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    emit finished();
}
