#ifndef BASEINTERFACETHREAD_H
#define BASEINTERFACETHREAD_H

#include "baseinterface.h"

#include <QObject>
#include <QWaitCondition>

namespace Interface
{

class BaseInterfaceThread : public QObject
{
    Q_OBJECT
public:
    explicit BaseInterfaceThread(QObject *parent = nullptr);

    void clear();
    void wakeUp();

    void checkQueue();
    void finishCommand();
    virtual void parseRequest(const CommandStruct &cmdStr) = 0;

    bool m_isCommandRequested = false;
    bool m_parsingDataReady = false;
    quint64 m_progress = 0;
    CommandStruct m_currentCommand;
    QMutex _mutex;
    QByteArray m_readData;
    QWaitCondition _waiter;

protected:
signals:
    void finished();
    void sendDataToPort(const QByteArray &ba);

public slots:
    void run();
    virtual void processReadBytes(QByteArray ba) = 0;
    virtual void parseResponse() = 0;
};

}

#endif // BASEINTERFACETHREAD_H
