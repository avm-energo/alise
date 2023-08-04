#include "broker.h"

#include "aliseconstants.h"

#include <QDebug>

Broker::Broker(QObject *parent) : QObject(parent)
{
    QTimer *checkPowerTimer = new QTimer(this);
    checkPowerTimer->setInterval(AliseConstants::PowerCheckPeriod());
    QObject::connect(checkPowerTimer, &QTimer::timeout, this, &Broker::checkPowerUnit);

    QTimer *checkIndicationTimer = new QTimer(this);
    checkIndicationTimer->setInterval(checkIndicationPeriod);
    QObject::connect(checkIndicationTimer, &QTimer::timeout, this, &Broker::checkIndication);

    m_clientTimeoutTimer.setInterval(
        AliseConstants::HealthQueryPeriod() * 3); // 3 times of Health Query period without replies
    QObject::connect(&m_clientTimeoutTimer, &QTimer::timeout, [&] {
        qDebug() << "Health Query Timeout";
        criticalBlinking();
    });

    checkIndicationTimer->start();
    checkPowerTimer->start();
    m_clientTimeoutTimer.start();
}

void Broker::healthReceived(alise::Health_Code code)
{
    qDebug() << "Setting indication: " << code;
    m_clientTimeoutTimer.start();
    switch (code)
    {
    case alise::Health_Code_Startup:
    {
        if (m_currentBlinkingPeriod == AliseConstants::SonicaStartingBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::SonicaStartingBlinkPeriod();
        break;
    }
    case alise::Health_Code_Work:
    {
        if (m_currentBlinkingPeriod == AliseConstants::SonicaNormalBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::SonicaNormalBlinkPeriod();
        break;
    }
    default:
    {
        if (m_currentBlinkingPeriod == AliseConstants::FailureBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::FailureBlinkPeriod();
        break;
    }
    }
    setIndication();
}

void Broker::criticalBlinking()
{
    m_currentBlinkingPeriod = AliseConstants::FailureBlinkPeriod();
    setIndication();
}
