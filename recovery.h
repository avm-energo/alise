#ifndef RECOVERY_H
#define RECOVERY_H

#include <QObject>
#include <gen/datatypes.h>

class RecoveryEngine : public QObject
{
    Q_OBJECT
public:
    RecoveryEngine(QObject *parent = nullptr);

public slots:
    void receiveBlock(const DataTypes::BlockStruct &blk);

signals:
    void rebootReq();

private:
    void setDefEth(int ethNum);
    void sync();
    void restartNetwork();
    bool resetInit = false;
};

#endif // RECOVERY_H
