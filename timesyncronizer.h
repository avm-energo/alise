#pragma once

#include "ntpstatus.h"

#include <QObject>
#include <ctime>

class TimeSyncronizer : public QObject
{
    Q_OBJECT
public:
    enum CurrentCommandEnum
    {
        NTP,
        HWCLOCK,
        TIMEZONE
    };

    explicit TimeSyncronizer(QObject *parent = nullptr);
    virtual ~TimeSyncronizer();

    void init();
    timespec systemTime() const;

public slots:
    void printAndSetSystemTime(const timespec time);
    void setTimeZone(const int8_t tz);

private:
    void setSystemTime(const timespec &systemTime);
    void setHWClock();
    int m_timeCounter;
    CurrentCommandEnum curCommand;
    NtpStatus *m_ntpStatus;
    int oldNtpStatus;

private slots:
    void checkNtpAndSetTime();
    void synchrHwClockWithNtp(int status);

signals:
    void ntpStatusChanged(int);
    void setTime(const timespec &systemTime);
};
