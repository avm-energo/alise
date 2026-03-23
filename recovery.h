#pragma once
#include <QObject>
#include <avm-gen/datatypes.h>

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
    bool m_resetInit;
};
