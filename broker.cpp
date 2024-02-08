#include "broker.h"

#include "aliseconstants.h"

#include <QDebug>

using namespace Alise;

Broker::Broker(QObject *parent) : QObject(parent)
{
    m_clientTimeoutTimer.setInterval(AliseConstants::ReplyTimeoutPeriod());
    QObject::connect(&m_clientTimeoutTimer, &QTimer::timeout, [&] {
        qDebug() << "Health Query Timeout";
        setIndication(AliseConstants::FailureIndication);
    });

    m_clientTimeoutTimer.start();
}

// Health code received
//
// code bits: 0: 0 - there's some errors, 1 - no errors
// 1 - booter working
// 2 - adminja working
// 3 - core working
// 4 - alise working
// 5 - ninja working
// 6 - vasya working
// 7 - petya working
// 8-10 - booter status:
//     000 - no status (bit 1 = 0) or working normally (bit 1 = 1)
//     001 - not installed
//     010 - starting
//     011 - need to merge settings
//     100 - broken hash
//     101 - stopping
//     110 - stopped
//     111 - unknown status
// 11-13 - adminja status
// 14-16 - core status
// 17-19 - alise status
// 20-22 - ninja status
// 23-25 - vasya status
// 26-28 - petya status
// 29-31 - errors: 000 - no errors, 001 - unknown controller, 111 - unknown error

void Broker::healthReceived(uint32_t code)
{
    uint8_t processStatus;
    uint8_t mainHealthBits = (code >> 29) & 0x07;             // main health bits - three MSB's
    uint8_t healthCode = code & 0x000000FF;                   // health bits by component and overall health in LSB
    uint32_t componentHealthCodes = (code >> 8) & 0x001FFFFF; // healthes by component
    m_worstProcessNumber = 0;
    m_worstProcessError = NORMAL; // 0 - no errors, 1 - starting/stopping, 2 - stopped, 3 - error
    AVTUK_CCU::Indication indic;
    qDebug() << "Setting indication: " << code;
    qDebug() << "MainHealthBits = " << mainHealthBits << "HealthCode = " << healthCode
             << "ComponentHealthCodes = " << componentHealthCodes;
    if (mainHealthBits || !(healthCode & 0x01))
        indic = AliseConstants::FailureIndication;
    else
    {
        for (int i = 0; i < numberOfProcesses; ++i)
        {
            componentHealthCodes >>= 3;
            processStatus = (componentHealthCodes & 0x00000007);
            qDebug() << "processStatus now is: " << processStatus;
            if (healthCode & (0x02 << i)) // process working
            {
                qDebug() << "healthCode for " << i << " = 1";
                // check three bits in corresponding position
                switch (processStatus)
                {
                case Alise::NEEDTOMERGE:
                case Alise::BROKENHASH:
                    setSemiWorkingProcessError(i);
                    break;
                case Alise::CHECKHTHBIT: // working normally
                    break;               // default value is NORMAL
                default:
                    qDebug() << "processStatus is unknown: " << processStatus << " not 0, 3, 4";
                    setFailedProcessError(i);
                    break;
                }
            }
            else // process isn't working
            {
                qDebug() << "healthCode for " << i << " = 0";
                // check three bits in corresponding position
                switch (processStatus)
                {
                // any failure causes failed indication
                case Alise::NOTINSTALLED:
                case Alise::CHECKHTHBIT:
                    break; // that's normal for now, checking remains
                case Alise::UNKNOWN:
                case Alise::BROKENHASH:
                    setFailedProcessError(i);
                    break;
                case Alise::STARTING:
                case Alise::STOPPING:
                    setStartingProcessError(i);
                    break;
                case Alise::NEEDTOMERGE:
                case Alise::STOPPED:
                    setStoppedProcessError(i);
                    break;
                }
            }
        }
        if ((m_worstProcessError == Alise::NORMAL) || (m_worstProcessNumber == 0)) // all is good
            indic = AliseConstants::NormalIndication;
        else
        {
            indic.PulseCnt1 = firstPulsesCount;         // first count is for showing status
            indic.PulseCnt2 = m_worstProcessNumber + 1; // second count is the number of process
            indic.PulseFreq2 = AliseConstants::ProcessBlink(Alise::RED);
            indic.PulseFreq1 = AliseConstants::ProcessBlink(m_worstProcessError);
        }
    }
    setIndication(indic);
    m_clientTimeoutTimer.start(); // restart health query timeout timer
}

void Broker::setStartingProcessError(int index)
{
    if ((m_worstProcessError != Alise::RED) && (m_worstProcessError != Alise::VIOLET)
        && (m_worstProcessError != Alise::ORANGE))
    {
        qDebug() << "set mode YELLOW for " << index << " component";
        m_worstProcessNumber = index;
        m_worstProcessError = Alise::YELLOW;
    }
}

void Broker::setStoppedProcessError(int index)
{
    if ((m_worstProcessError != Alise::RED) && (m_worstProcessError != Alise::VIOLET))
    {
        qDebug() << "set mode ORANGE for " << index << " component";
        m_worstProcessNumber = index;
        m_worstProcessError = Alise::ORANGE;
    }
}

void Broker::setFailedProcessError(int index)
{
    qDebug() << "set mode FAILED for " << index << " component";
    m_worstProcessNumber = index;
    m_worstProcessError = Alise::RED;
}

void Broker::setSemiWorkingProcessError(int index)
{
    if (m_worstProcessError != Alise::RED)
    {
        qDebug() << "set mode VIOLET for " << index << " component";
        m_worstProcessNumber = index;
        m_worstProcessError = Alise::VIOLET;
    }
}
