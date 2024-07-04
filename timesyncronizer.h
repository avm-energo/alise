#pragma once

#include <QObject>
#include <ctime>
#include <gen/executecommand.h>

class TimeSyncronizer : public QObject
{
    Q_OBJECT
public:
    enum CurrentCommandEnum
    {
        NTP,
        HWCLOCK
    };

    enum NtpStatusEnum
    {
        NO_SYNC = 1,
        SYNC_EXT = 2,
        SYNC_LOCAL = 3
    };

    explicit TimeSyncronizer(QObject *parent = nullptr);
    virtual ~TimeSyncronizer();

    void init();
    timespec systemTime() const;
    void requestNtpStatus();

public slots:
    void printAndSetSystemTime(const timespec time);

private:
    void setSystemTime(const timespec &systemTime);
    void setHWClock();
    int m_timeCounter;
    CurrentCommandEnum curCommand;
    ExecuteCommandAsync *executor;
    int oldNtpStatus;

private slots:
    void checkNtpAndSetTime();
    void commandResultAcquired(const QString &output);
    void commandExitCodeAcuired(int exitCode);
    void synchrHwClockWithNtp(int status);

signals:
    void ntpStatusChanged(int);
    void setTime(const timespec &systemTime);
};
