#pragma once

#include <QObject>
#include <ctime>

class TimeSyncronizer : public QObject {
  Q_OBJECT
public:
  explicit TimeSyncronizer(QObject *parent = nullptr);
  virtual ~TimeSyncronizer();

  timespec systemTime() const;
  bool ntpStatus() const;

  Q_PROPERTY(bool ntpStatus READ ntpStatus NOTIFY ntpStatusChanged)
  Q_PROPERTY(timespec systemTime READ systemTime WRITE handleTime)
public slots:
  void handleTime(const timespec time);

private:
  void setSystemTime(const timespec &systemTime);

signals:
  void ntpStatusChanged(bool);
};
