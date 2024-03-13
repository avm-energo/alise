#include "controller.h"

#include "aliseconstants.h"

#include <QDebug>
#include <QTimer>
#include <iostream>

Controller::Controller(QObject *parent) noexcept : QObject(parent)
{
}

bool Controller::launchOnPort(int port)
{
    if (isIncorrectType())
    {
        qCritical() << "Controller has incorrect type or no type was given";
        return false;
    }
    m_runner->runServer(port);
#if defined(AVTUK_STM)
    m_deviceBroker->getTime();
#elif defined(AVTUK_NO_STM)
    m_timeSynchronizer->systemTime();
#endif
    return true;
}

void Controller::shutdown()
{
    QMetaObject::invokeMethod(m_runner, &ZeroRunner::stopServer, Qt::DirectConnection);
}

void Controller::syncTime(const timespec &)
{
}

Controller *Controller::withBroker(Broker *broker)
{
    m_deviceBroker = broker;
    return this;
}

Controller *Controller::withTimeSynchonizer(TimeSyncronizer *tm)
{
    m_timeSynchronizer = tm;
    return this;
}

Controller *Controller::ofType(Controller::ContrTypes type)
{
    if (!c_controllerMap.contains(type))
        m_type = Controller::ContrTypes::IS_INCORRECT_TYPE;
    else
        m_type = type;
    m_runner = new ZeroRunner(c_controllerMap[type], this);
    switch (type)
    {
    case IS_ADMINJA:
        adminjaSetup();
        break;
    case IS_BOOTER:
        booterSetup();
        break;
    case IS_CORE:
        coreSetup();
        break;
    default:
        qCritical() << "Incorrect controller type";
        break;
    }
    return this;
}

void Controller::adminjaSetup()
{
    connect(m_runner, &ZeroRunner::timeReceived, m_timeSynchronizer, &TimeSyncronizer::printAndSetSystemTime);
#if defined(AVTUK_STM)
    connect(m_runner, &ZeroRunner::timeReceived, m_deviceBroker, &Broker::setTime);
    connect(m_runner, &ZeroRunner::timeRequest, m_deviceBroker, &Broker::getTime);
#elif defined(AVTUK_NO_STM)
    connect(m_runner, &ZeroRunner::timeRequest, m_timeSynchronizer, //
        [this] { m_deviceBroker->updateTime(m_timeSynchronizer->systemTime()); });
#endif
    connect(m_deviceBroker, &Broker::receivedTime, m_runner, &ZeroRunner::publishTime, Qt::DirectConnection);
    connect(m_timeSynchronizer, &TimeSyncronizer::ntpStatusChanged, this, [&](bool status) {
        m_runner->publishNtpStatus(status);
        if (!status)
            return;
    });
}

void Controller::booterSetup()
{
    m_pingTimer = new QTimer(this);
    m_pingTimer->setInterval(Alise::AliseConstants::HealthQueryPeriod());
    // publish data to zeroMQ channel
    connect(m_pingTimer, &QTimer::timeout, m_runner, &ZeroRunner::publishHealthQueryCallback);
    m_pingTimer->start();
    connect(m_runner, &ZeroRunner::healthReceived, m_deviceBroker, &Broker::healthReceived);
}

void Controller::coreSetup()
{
    connect(m_deviceBroker, &Broker::receivedBlock, m_runner, &ZeroRunner::publishBlock);
}

bool Controller::isIncorrectType()
{
    return (m_type == IS_INCORRECT_TYPE);
}
