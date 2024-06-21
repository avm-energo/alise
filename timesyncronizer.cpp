#include "timesyncronizer.h"

#include <QDateTime>
#include <QTimer>
#include <QtDebug>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <error.h>
#include <gen/stdfunc.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/timex.h>

#define NTP_PORT 123

/* This program uses an NTP mode 6 control message, which is the
   same as that used by the ntpq command.  The ntpdc command uses
   NTP mode 7, details of which are elusive.
   For details on the format of NTP control message, see
   http://www.eecis.udel.edu/~mills/database/rfc/rfc1305/rfc1305b.ps.
   This document covers NTP version 2, however the control message
   format works with ntpd version 4 and earlier.
   Section 3.2 (pp 9 ff) of RFC1305b describes the data formats used by
   this program.
   This returns:
      0:   if clock is synchronised.
      1:   if clock is not synchronised.
      2:   if clock state cannot be determined, eg if
           ntpd is not contactable.                 */
/* ------------------------------------------------------------------------*/
/* value of Byte1 and Byte2 in the ntpmsg       */
#define B1VAL 0x16 /* VN = 2, Mode = 6 */
#define B2VAL                                                                                                          \
    2                   /* Response = 0; ( this makes the packet a command )                                           \
                           Error    = 0;                                                                               \
                           More     = 0;                                                                               \
                           Op Code  = 2 (read variables command) */
#define RMASK 0x80      /* bit mask for the response bit in Status Byte2 */
#define EMASK 0x40      /* bit mask for the error bit in Status Byte2 */
#define MMASK 0x20      /* bit mask for the more bit in Status Byte2 */
#define PAYLOADSIZE 468 /* size in bytes of the message payload string */

#define NTPSTATUSPERIOD 3000

TimeSyncronizer::TimeSyncronizer(QObject *parent) : QObject(parent)
{
    executor = new ExecuteCommandAsync;
    connect(executor, &ExecuteCommandAsync::resultAcquired, this, &TimeSyncronizer::commandResultAcquired);
    connect(executor, &ExecuteCommandAsync::finished, this, &TimeSyncronizer::commandExitCodeAcuired);
    connect(this, &TimeSyncronizer::ntpStatusChanged, this, &TimeSyncronizer::ntpStatusChanged);
}

TimeSyncronizer::~TimeSyncronizer()
{
}

void TimeSyncronizer::init()
{
    m_timeCounter = 0;
    requestNtpStatus(); // first time we must emit ntpStatus
    QTimer *timer = new QTimer(this);
    timer->setInterval(NTPSTATUSPERIOD);
    connect(timer, &QTimer::timeout, this, &TimeSyncronizer::checkNtpAndSetTime);
    timer->start();
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
    QString program = "/usr/sbin/hwclock -w";

    struct timeval timeToSet;
    timeToSet.tv_sec = systemTime.tv_sec;
    timeToSet.tv_usec = 0;
    settimeofday(&timeToSet, NULL);
    qInfo() << "Setting hwclock time: " << systemTime.tv_sec;
    curCommand = CurrentCommandEnum::HWCLOCK;
    executor->execute(program);
#else
    // to be written...
#endif
}

void TimeSyncronizer::checkNtpAndSetTime()
{
    requestNtpStatus();
    ++m_timeCounter;
}

void TimeSyncronizer::commandResultAcquired(const QString &output)
{
    if (curCommand == CurrentCommandEnum::NTP)
    {
        qDebug() << "Ntpq -p output: " << output;
        if (output.isEmpty())
        {
            qWarning() << "ntpq output is empty!";
            emit ntpStatusChanged(NO_SYNC);
        }
        if (output.contains("Connection refused"))
        {
            qWarning() << "ntpq error: connection refused";
            emit ntpStatusChanged(NO_SYNC);
        }

        // Split ntpq output
        QStringList lines = output.split('\n');
        if (lines.size() < 2)
        {
            qWarning() << "ntpq output is wrong!";
            emit ntpStatusChanged(NO_SYNC);
        }
        // Removing ntpq table header
        lines.removeFirst(); //     remote           refid      st t when poll reach   delay   offset  jitter
        lines.removeFirst(); //==============================================================================
        foreach (QString str, lines)
        {
            if (str.startsWith("*LOCAL")) // ntp is synchronized locally
                emit ntpStatusChanged(SYNC_LOCAL);
            else if (str.startsWith("*")) // ntp is synchronized externally
                emit ntpStatusChanged(SYNC_EXT);
            qDebug() << "Sync";
        }
        emit ntpStatusChanged(NO_SYNC);
        qDebug() << "Not sync";
    }
}

void TimeSyncronizer::commandExitCodeAcuired(int exitCode)
{
    if (exitCode != 0)
        qWarning() << "Error executing command, status: " << exitCode;
}

void TimeSyncronizer::ntpStatusReceived(int status)
{
    if (m_timeCounter >= 20) // one time per minute
    {
        if (status != NO_SYNC) // ntp is working
        {
            m_timeCounter = 0;
            emit setTime(systemTime());
        }
    }
}

void TimeSyncronizer::requestNtpStatus()
{
    curCommand = CurrentCommandEnum::NTP;
    QString program = "/usr/bin/ntpq -p";
    qDebug() << "Requesting NTP status...";
    executor->execute(program);
}
