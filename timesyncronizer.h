#pragma once

#include <QObject>
#include <ctime>

class TimeSyncronizer : public QObject
{
    Q_OBJECT
public:
    explicit TimeSyncronizer(QObject *parent = nullptr);
    virtual ~TimeSyncronizer();

    void init();
    timespec systemTime() const;
    bool ntpStatus() const;

    Q_PROPERTY(bool ntpStatus READ ntpStatus NOTIFY ntpStatusChanged)
    Q_PROPERTY(timespec systemTime READ systemTime WRITE printAndSetSystemTime)
public slots:
    void printAndSetSystemTime(const timespec time);

private:
    void setSystemTime(const timespec &systemTime);
    bool m_ntpStatus;
    int m_timeCounter;

private slots:
    void checkNtpAndSetTime();

signals:
    void ntpStatusChanged(bool);
    void setTime(const timespec &systemTime);
};
