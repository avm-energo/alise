#include "controller.h"

#include "aliseconstants.h"

#include <QTimer>
#include <iostream>

Controller::Controller(Broker *devBroker, ZeroRunner *runner, QObject *parent) noexcept
    : QObject(parent), m_runner(runner), m_deviceBroker(devBroker)
{

    m_type = IS_INCORRECT_TYPE;
    proxyBS = UniquePointer<DataTypesProxy>(new DataTypesProxy(&DataManager::GetInstance()));
    proxyBS->RegisterType<DataTypes::BlockStruct>();
#ifdef __linux__
    proxyTS = UniquePointer<DataTypesProxy>(new DataTypesProxy(&DataManager::GetInstance()));
    proxyTS->RegisterType<timespec>();
#endif
}

Controller::~Controller()
{
}

bool Controller::launch()
{
    if (hasIncorrectType())
    {
        qCritical() << "Controller has incorrect type or no type was given";
        return false;
    }

    // RecoveryEngine: rebooting and Power Status get from MCU
    connect(&m_recoveryEngine, &RecoveryEngine::rebootReq, m_deviceBroker, &Broker::rebootMyself);
    connect(proxyBS.get(), &DataTypesProxy::DataStorable, &m_recoveryEngine, &RecoveryEngine::receiveBlock);
    connect(proxyBS.get(), &DataTypesProxy::DataStorable, m_deviceBroker, &Broker::currentIndicationReceived);

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
    {
        connect(m_runner, &ZeroRunner::timeReceived, &m_timeSynchronizer, &TimeSyncronizer::printAndSetSystemTime);
#if defined(AVTUK_STM)
        connect(m_runner, &ZeroRunner::timeReceived, m_deviceBroker, &Broker::setTime);
        connect(m_runner, &ZeroRunner::timeRequest, m_deviceBroker, &Broker::getTime);
#elif defined(AVTUK_NO_STM)
        connect(m_runner, &ZeroRunner::timeRequest, &m_timeSynchronizer, //
            [&] {
                auto ts = m_timeSynchronizer.systemTime();
                DataManager::GetInstance().addSignalToOutList(ts);
            });
#endif
        connect(proxyTS.get(), &DataTypesProxy::DataStorable, m_runner, &ZeroRunner::publishTime, Qt::DirectConnection);
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
            proxyTS.get(), &DataTypesProxy::DataStorable, &m_timeSynchronizer,
            // [=, &syncer = timeSync](const timespec &time) {
            [=, &syncer = m_timeSynchronizer](const QVariant &msg) {
                auto time = msg.value<timespec>();
                QObject::disconnect(*connectionTimeSync);
                syncer.printAndSetSystemTime(time);
            },
            Qt::DirectConnection);
#endif
        break;
    }
    case IS_BOOTER:
    {
        m_pingTimer = new QTimer;
        m_pingTimer->setInterval(Alise::AliseConstants::HealthQueryPeriod());
        // publish data to zeroMQ channel
        connect(m_pingTimer, &QTimer::timeout, m_runner, &ZeroRunner::publishHealthQueryCallback);
        m_pingTimer->start();
        connect(m_runner, &ZeroRunner::healthReceived, m_deviceBroker, &Broker::healthReceived);
        break;
    }
    case IS_CORE:
    {
        connect(proxyBS.get(), &DataTypesProxy::DataStorable, m_runner, &ZeroRunner::publishBlock);
        break;
    }
    default:
        qCritical() << "Incorrect controller type";
        break;
    }
}

bool Controller::hasIncorrectType()
{
    return (m_type == IS_INCORRECT_TYPE);
}
