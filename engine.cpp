#include "engine.h"

#include "aliseconstants.h"
#if defined(AVTUK_STM)
#include "stmbroker.h"
#else
#include "gpiobroker.h"
#endif

Engine::Engine(QObject *parent)
{
    Q_UNUSED(parent)
}

bool Engine::init(AliseSettings &settings)
{
    m_timeSynchronizer = new TimeSyncronizer;
    m_timeSynchronizer->init();
#if defined(AVTUK_STM)
    m_broker = new StmBroker(this);
#else
    m_broker = new GpioBroker(settings.m_gpioMap, this);
#endif
    bool ok = m_broker->connect();
    if (!ok)
        return false;

    m_mw = new HttpMiddleware(settings);
    m_server = new HttpApiServer(m_mw);
    m_server->initServer(settings.httpPort, Alise::AliseConstants::ReplyTimeoutPeriod());

    createLocalConnections();
    createHttpConnections();

#if defined(AVTUK_STM)
    m_broker->getTime(); // чтение времени из STM32 и запись его в систему
#else
//    m_timeSynchronizer->systemTime(); // не имеет смысла, т.к. по таймеру UpdateTime получим время
#endif

    return true;
}

void Engine::start()
{
}

void Engine::createLocalConnections()
{
    // check power status
    QTimer *checkPowerTimer = new QTimer(this);
    checkPowerTimer->setInterval(Alise::AliseConstants::PowerCheckPeriod());
    QObject::connect(checkPowerTimer, &QTimer::timeout, m_broker, &Broker::checkPowerUnit);
    checkPowerTimer->start();

    // check indication in
    QTimer *checkIndicationTimer = new QTimer(this);
    checkIndicationTimer->setInterval(checkIndicationPeriod);
    QObject::connect(checkIndicationTimer, &QTimer::timeout, m_broker, &Broker::checkIndication);
    checkIndicationTimer->start();

    // update time in Http MiddleWare periodically
    QTimer *updateTimeInHttpTimer = new QTimer(this);
    updateTimeInHttpTimer->setInterval(Alise::AliseConstants::UpdateTimePeriod());
    QObject::connect(updateTimeInHttpTimer, &QTimer::timeout, [&]() {
#if defined(AVTUK_STM)
        m_broker->getTime();
#else
        m_broker->updateTime(m_timeSynchronizer->systemTime());
#endif
    });
    updateTimeInHttpTimer->start();

    // RecoveryEngine: rebooting and Power Status get from MCU
    connect(&m_recoveryEngine, &RecoveryEngine::rebootReq, m_broker, &Broker::rebootMyself);
    connect(m_broker, &Broker::receivedBlock, &m_recoveryEngine, &RecoveryEngine::receiveBlock);

#if defined(AVTUK_STM)
    // if NTP is active write current system time into STM32 once per minute
    connect(m_timeSynchronizer, &TimeSyncronizer::setTime, m_broker, &Broker::setTime);

    // sets system time in Linux once at start when receive it from STM32
    auto connectionTimeSync = std::shared_ptr<QMetaObject::Connection>(new QMetaObject::Connection);
    *connectionTimeSync = connect(
        m_broker, &Broker::receivedTime, this,
        [=, &syncer = m_timeSynchronizer](const timespec &time) {
            QObject::disconnect(*connectionTimeSync);
            syncer->printAndSetSystemTime(time);
        },
        Qt::DirectConnection);
#endif
}

void Engine::createHttpConnections()
{
    // time received: set Linux system time
    connect(m_mw, &HttpMiddleware::timeReceived, m_timeSynchronizer, &TimeSyncronizer::printAndSetSystemTime);
    connect(m_mw, &HttpMiddleware::timeZoneReceived, m_timeSynchronizer, &TimeSyncronizer::setTimeZone);

#if defined(AVTUK_STM)
    // time received: write new time into STM32
    connect(m_mw, &HttpMiddleware::timeReceived, m_broker, &Broker::setTime);
#endif

    // force update time that is being sent through HTTP by receiving time from broker - either by TimeRequest command
    // or by timeTimer timeout
    connect(m_broker, &Broker::receivedTime, m_mw, &HttpMiddleware::setTimeStamp);

    connect(
        m_timeSynchronizer, &TimeSyncronizer::ntpStatusChanged, this, [&](int status) { m_mw->setNtpState(status); });

    connect(m_mw, &HttpMiddleware::healthReceived, m_broker, &Broker::healthReceived);
    connect(m_mw, &HttpMiddleware::booterConnectionIsLost, m_broker, &Broker::setFailedIndication);
    connect(m_broker, &Broker::receivedBlock, m_mw, &HttpMiddleware::setBlock);
}
