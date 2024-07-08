#include "timesyncronizer.h"

#include "aliseconstants.h"

#include <QDateTime>
#include <QTimer>
#include <QtDebug>
#include <gen/executecommand.h>
#include <gen/stdfunc.h>
#include <sys/time.h>

TimeSyncronizer::TimeSyncronizer(QObject *parent) : QObject(parent)
{
    m_ntpStatus = new NtpStatus;
    oldNtpStatus = NtpStatus::NO_SYNC;
}

TimeSyncronizer::~TimeSyncronizer()
{
}

void TimeSyncronizer::init()
{
    m_timeCounter = 0;
    checkNtpAndSetTime(); // first time we must emit ntpStatus
    QTimer *timer = new QTimer(this);
    timer->setInterval(Alise::AliseConstants::UpdateTimePeriod());
    connect(timer, &QTimer::timeout, this, &TimeSyncronizer::checkNtpAndSetTime);
    timer->start();
    connect(this, &TimeSyncronizer::ntpStatusChanged, this, &TimeSyncronizer::synchrHwClockWithNtp);
}

void printts(const timespec &st)
{
    auto datetime = QDateTime::fromMSecsSinceEpoch(((st.tv_sec * 1000) + (st.tv_nsec / 1.0e6)));
    qDebug() << "Setting datetime: " << datetime;
}

void TimeSyncronizer::printAndSetSystemTime(const timespec time)
{
    printts(time);
    setSystemTime(time);
}

timespec TimeSyncronizer::systemTime() const
{
    timespec time;
    struct timeval timeToGet;
    gettimeofday(&timeToGet, NULL);
    time.tv_sec = timeToGet.tv_sec;
    time.tv_nsec = timeToGet.tv_usec * 1000;
    return time;
}

void TimeSyncronizer::setSystemTime(const timespec &systemTime)
{
#if defined(Q_OS_LINUX)
    struct timeval timeToSet;
    timeToSet.tv_sec = systemTime.tv_sec;
    timeToSet.tv_usec = 0;
    settimeofday(&timeToSet, NULL);
    setHWClock();
#else
    // to be written...
#endif
}

void TimeSyncronizer::setHWClock()
{
    QString program = "/usr/sbin/hwclock -w";
    qInfo() << "Setting hwclock time: " << systemTime().tv_sec;
    curCommand = CurrentCommandEnum::HWCLOCK;
    ExecuteCommandAsync *executor = new ExecuteCommandAsync;
    executor->execute(program);
}

void TimeSyncronizer::checkNtpAndSetTime()
{
    emit ntpStatusChanged(m_ntpStatus->getNtpStatus());
    ++m_timeCounter;
}

/// \brief Synchronize HW RTC Clock upon Ntp is synchronized with external source and send setTime signal once per
/// minute
/// \param status - status of NTP. If it was changed from anything to SYNC_EXT emit hwclock -w command

void TimeSyncronizer::synchrHwClockWithNtp(int status)
{
    if (status != oldNtpStatus)
    {
        oldNtpStatus = status;
        if (status == NtpStatus::SYNC_EXT)
            setHWClock();
    }
    if (m_timeCounter >= 20) // one time per minute
    {
        if (status != NtpStatus::NO_SYNC) // ntp is working
        {
            m_timeCounter = 0;
            emit setTime(systemTime());
        }
    }
}
