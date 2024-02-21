#include "controller.h"

#include "aliseconstants.h"

#include <QDebug>
#include <QTimer>
#include <iostream>

Controller::Controller(Broker *devBroker, ZeroRunner *runner, QObject *parent) noexcept
    : QObject(parent)
    , m_type(IS_INCORRECT_TYPE)
    , m_runner(runner)
    , m_deviceBroker(devBroker)
    , m_timeSynchronizer(this)
    , m_recoveryEngine(this)
{
}

bool Controller::launch()
{
    if (isIncorrectType())
    {
        qCritical() << "Controller has incorrect type or no type was given";
        return false;
    }

    // RecoveryEngine: rebooting and Power Status get from MCU
    connect(&m_recoveryEngine, &RecoveryEngine::rebootReq, m_deviceBroker, &Broker::rebootMyself);
    connect(m_deviceBroker, &Broker::receivedBlock, &m_recoveryEngine, &RecoveryEngine::receiveBlock);

#if defined(AVTUK_STM)
    m_deviceBroker->getTime();
#elif defined(AVTUK_NO_STM)
    m_timeSynchronizer.systemTime();
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

void Controller::ofType(Controller::ContrTypes type)
{
    m_type = type;
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
}

void Controller::adminjaSetup()
{
    connect(m_runner, &ZeroRunner::timeReceived, &m_timeSynchronizer, &TimeSyncronizer::printAndSetSystemTime);
#if defined(AVTUK_STM)
    connect(m_runner, &ZeroRunner::timeReceived, m_deviceBroker, &Broker::setTime);
    connect(m_runner, &ZeroRunner::timeRequest, m_deviceBroker, &Broker::getTime);
#elif defined(AVTUK_NO_STM)
    connect(m_runner, &ZeroRunner::timeRequest, &m_timeSynchronizer, //
        [this] { m_deviceBroker->updateTime(m_timeSynchronizer.systemTime()); });
#endif
    connect(m_deviceBroker, &Broker::receivedTime, m_runner, &ZeroRunner::publishTime, Qt::DirectConnection);
    connect(&m_timeSynchronizer, &TimeSyncronizer::ntpStatusChanged, this, [&](bool status) {
        m_runner->publishNtpStatus(status);
        if (!status)
            return;
    });
#if defined(AVTUK_STM)
    connect(&m_timeSynchronizer, &TimeSyncronizer::setTime, this,
        [&](const timespec &time) { m_deviceBroker->setTime(time); });
#endif
#ifdef __linux__
    auto connectionTimeSync = std::shared_ptr<QMetaObject::Connection>(new QMetaObject::Connection);
    *connectionTimeSync = connect(
        m_deviceBroker, &Broker::receivedTime, this,
        [=, &syncer = m_timeSynchronizer](const timespec &time) {
            QObject::disconnect(*connectionTimeSync);
            syncer.printAndSetSystemTime(time);
        },
        Qt::DirectConnection);
#endif
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
