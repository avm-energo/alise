#include "controller.h"

<<<<<<< HEAD
#include <gen/datatypes.h>
=======
#include "aliseconstants.h"

#include <QTimer>
>>>>>>> e31a3b7... [*] some controller refactoring, moving booter and core connections into switch-case [+] conffile into deb package to prevent its overwriting [*] log messages refactoring [+] Time to wait for hard reset introduced in settings [*] Recovery class refactoring
#include <iostream>

constexpr int minSecs = 60;

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

    // timer to periodically check the connection
    m_pingTimer = new QTimer;
    m_pingTimer->setInterval(AliseConstants::HealthQueryPeriod());

    // publish data to zeroMQ channel common to all the controllers
    connect(m_pingTimer, &QTimer::timeout, m_runner, &ZeroRunner::publishHealthQueryCallback);
    connect(proxyBS.get(), &DataTypesProxy::DataStorable, m_runner, &ZeroRunner::publishBlock);

    // RecoveryEngine: rebooting and Power Status get from MCU
    connect(&m_recoveryEngine, &RecoveryEngine::rebootReq, m_deviceBroker, &Broker::rebootMyself);
    connect(proxyBS.get(), &DataTypesProxy::DataStorable, &m_recoveryEngine, &RecoveryEngine::receiveBlock);

    m_pingTimer->start();
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
    case IS_BOOTER:
    {
        connect(m_runner, &ZeroRunner::healthReceived, m_deviceBroker, &Broker::healthReceived);
        break;
    }
    case IS_CORE:
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
        //    connect(&m_timeSynchronizer, &TimeSyncronizer::ntpStatusChanged, m_runner,
        //    &ZeropublishNtpStatus);
        connect(&m_timeSynchronizer, &TimeSyncronizer::ntpStatusChanged, this, [&](bool status) {
            m_runner->publishNtpStatus(status);
            if (!status)
                return;
            syncCounter++;
            if (syncCounter == minSecs)
            {
#if defined(AVTUK_STM)
                m_deviceBroker->setTime(m_timeSynchronizer.systemTime());
#endif
                syncCounter = 0;
            }
        });
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
    default:
        qCritical() << "Incorrect controller type";
        break;
    }
}

bool Controller::hasIncorrectType()
{
    return (m_type != IS_BOOTER) && (m_type != IS_CORE);
}
