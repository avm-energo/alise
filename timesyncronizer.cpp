#include "timesyncronizer.h"

#include "helper.h"

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

void printts(const timespec &st)
{
    auto datetime = QDateTime::fromMSecsSinceEpoch(((st.tv_sec * 1000) + (st.tv_nsec / 1.0e6)));
    std::cout << datetime.toString().toStdString().c_str() << std::endl;
    std::cout << st << std::endl;
}

TimeSyncronizer::TimeSyncronizer(QObject *parent) : QObject(parent)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(NTPSTATUSPERIOD);
    connect(timer, &QTimer::timeout, this, [this] { emit ntpStatusChanged(ntpStatus()); });
    timer->start();
}

void TimeSyncronizer::handleTime(const timespec time)
{
    printts(time);
    setSystemTime(time);
}

timespec TimeSyncronizer::systemTime() const
{
    timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return time;
}

void TimeSyncronizer::setSystemTime(const timespec &systemTime)
{
    QString program = "/usr/sbin/hwclock";
    QStringList arguments { "-w" };

    clock_settime(CLOCK_REALTIME, &systemTime); // set current datetime
    QProcess *myProcess = new QProcess(this);   // set datetime to RTC
    qInfo() << "HWClock is starting...";
    myProcess->start(program, arguments);
    myProcess->waitForFinished();
    qInfo() << "HWClock exited with code: " << myProcess->exitCode() << " and status: " << myProcess->exitStatus();
}

bool TimeSyncronizer::ntpStatus() const
{
    ntptimeval time;
    int status = ntp_gettime(&time);
    qDebug() << "NTP Status: " << status;
    switch (status)
    {
    case TIME_OK:
        return true;
    case TIME_INS:
        return true;
    case TIME_DEL:
        return true;
    case TIME_OOP:
        return true;
    case TIME_WAIT:
        return true;
    case TIME_ERROR:
    default:
        return false;
        // return !ntpdStatus();
    }
}
