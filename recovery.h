#ifndef RECOVERY_H
#define RECOVERY_H

#include <QObject>

class Recovery : public QObject
{
    Q_OBJECT
public:
    Recovery(QObject *parent = nullptr);

public slots:
    void receiveBlock(const QVariant &msg);

signals:
    void rebootReq();

private:
    void eth0();
#if defined(AVTUK_NO_STM)
    void eth1();
#endif
#if defined(AVTUK_STM)
    void eth2();
#endif
    void sync();
    void restartNetwork();
    bool resetInit = false;
};

#endif // RECOVERY_H
