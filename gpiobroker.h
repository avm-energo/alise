#ifndef GPIOBROKER_H
#define GPIOBROKER_H

#include "protos/protos.pb.h"

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gpiod.hpp>

class GpioBroker : public QObject {
public:
  struct GpioPin {
    int chip;
    int offset;
  };
  GpioBroker(QObject *parent = nullptr);
  void checkPowerUnit();
  void setIndication(alise::Health_Code code);

  void setTime(timespec time);
  void getTime();
  void rebootMyself();

private:
  bool blinkStatus = true;
  int resetCounter = 0;
  QMutex _mutex;

  QTimer m_timer;
  QTimer m_gpioTimer;
  QTimer m_resetTimer, m_healthQueryTimeoutTimer;
  int m_currentBlinkingPeriod;
  void reset();
  void criticalBlinking();

  ::gpiod::chip chip0, chip1, chip2, chip3;

private slots:
  void blink();
};

#endif // GPIOBROKER_H
