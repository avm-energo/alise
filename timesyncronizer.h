#pragma once

#include <QObject>
#include <ctime>

class TimeSyncronizer : public QObject
{
    Q_OBJECT
public:
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
    int ntpStatus() const;

public slots:
    void printAndSetSystemTime(const timespec time);

private:
    void setSystemTime(const timespec &systemTime);
    int m_timeCounter;

private slots:
    void checkNtpAndSetTime();

signals:
    void ntpStatusChanged(int);
    void setTime(const timespec &systemTime);
};
