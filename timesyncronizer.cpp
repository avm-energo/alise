#include "timesyncronizer.h"

#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <error.h>
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
}

TimeSyncronizer::~TimeSyncronizer()
{
}

void TimeSyncronizer::init()
{
    m_timeCounter = 0;
    emit ntpStatusChanged(ntpStatus()); // first time we must emit ntpStatus
    QTimer *timer = new QTimer(this);
    timer->setInterval(NTPSTATUSPERIOD);
    connect(timer, &QTimer::timeout, this, [this] {
        bool status = ntpStatus();
        ++m_timeCounter;
        if (m_timeCounter >= 20) // one time per minute
        {
            if (status)
            {
                m_timeCounter = 0;
                emit setTime(systemTime());
            }
            emit ntpStatusChanged(status);
        }
    });
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
    //    clock_gettime(CLOCK_REALTIME, &time);
    time.tv_sec = timeToGet.tv_sec;
    time.tv_nsec = timeToGet.tv_usec * 1000;
    return time;
}

void TimeSyncronizer::setSystemTime(const timespec &systemTime)
{
    QString program = "/usr/sbin/hwclock";
    QStringList arguments { "-w" };

    struct timeval timeToSet;
    timeToSet.tv_sec = systemTime.tv_sec;
    timeToSet.tv_usec = 0;
    settimeofday(&timeToSet, NULL);
    //    clock_settime(CLOCK_REALTIME, &systemTime); // set current datetime
    QProcess *myProcess = new QProcess(this); // set datetime to RTC
    qInfo() << "Set hwclock time: " << systemTime.tv_sec;
    myProcess->start(program, arguments);
    myProcess->waitForFinished();
    qInfo() << "HWClock exited with code: " << myProcess->exitCode() << " and status: " << myProcess->exitStatus();
}

bool TimeSyncronizer::ntpStatus() const
{
    QString output;
#if defined(Q_OS_LINUX)
    QProcess process;
    QString program = "/usr/bin/ntpq";
    QStringList arguments { "-pn" };
    process.start(program, arguments);

    if (!process.waitForFinished(1000))
    {
        qWarning() << "ntpq start error: " << process.errorString();
        return false;
    }
    output = process.readAllStandardOutput();
#else
    QFile file(qApp->applicationDirPath() + "/ntpq_output.txt");
    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        output = file.readAll();
        file.close();
    }
#endif
    //    qDebug() << "Ntpq -pn output: " << output;
    if (output.isEmpty())
    {
        qWarning() << "ntpq output is empty!";
        return false;
    }
    if (output.contains("Connection refused"))
    {
        qWarning() << "ntpq error: connection refused";
        return false;
    }

    // Split ntpq output
    QStringList lines = output.split('\n');
    if (lines.size() < 2)
    {
        qWarning() << "ntpq output is wrong!";
        return false;
    }
    // Removing ntpq table header
    lines.removeFirst(); //     remote           refid      st t when poll reach   delay   offset  jitter
    lines.removeFirst(); //==============================================================================
    foreach (QString str, lines)
    {
        if (str.startsWith("*")) // ntp is synchronized
            return true;
    }
    return false;
}
