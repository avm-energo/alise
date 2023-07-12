#include "recovery.h"

#include "avtukccu.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <gen/datatypes.h>

constexpr char eth0path[] = "/etc/network/interfaces.d/eth0";
constexpr char eth1path[] = "/etc/network/interfaces.d/eth1";
constexpr char eth2path[] = "/etc/network/interfaces.d/eth2";

Recovery::Recovery(QObject *parent) : QObject(parent)
{
}

void Recovery::eth0()
{
    qDebug() << "[Recovery] Eth0";
    if (!QFile::exists(":/network/eth0"))
    {
        qCritical() << "No eth0 recovery";
        return;
    }
    if (QFile::exists(eth0path))
    {
        QFile::remove(eth0path);
    }

    if (!QFile::copy(":/network/eth0", eth0path))
    {
        qCritical() << "Couldn't copy eth0";
        return;
    }
    if (!QFile(eth0path).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser
            | QFileDevice::ReadGroup | QFileDevice::ReadOther))
    {
        qCritical() << "Couldn't change perm for " << eth0path;
    }
}
#if defined(AVTUK_NO_STM)
void Recovery::eth1()
{
    qDebug() << "[Recovery] Eth1";

    if (!QFile::exists(":/network/eth1"))
    {
        qCritical() << "No eth1 recovery";
        return;
    }
    if (QFile::exists(eth1path))
    {
        QFile::remove(eth1path);
    }

    if (!QFile::copy(":/network/eth1", eth1path))
    {
        qCritical() << "Couldn't copy eth1";
        return;
    }
    if (!QFile(eth1path).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser
            | QFileDevice::ReadGroup | QFileDevice::ReadOther))
    {
        qCritical() << "Couldn't change perm for " << eth1path;
    }
}
#endif
#if defined(AVTUK_STM)
void Recovery::eth2()
{
    qDebug() << "[Recovery] Eth2";
    if (!QFile::exists(":/network/eth2"))
    {
        qCritical() << "No eth2 recovery";
        return;
    }
    if (QFile::exists(eth2path))
    {
        QFile::remove(eth2path);
    }

    if (!QFile::copy(":/network/eth2", eth2path))
    {
        qCritical() << "Couldn't copy eth2";
        return;
    }
    if (!QFile(eth2path).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser
            | QFileDevice::ReadGroup | QFileDevice::ReadOther))
    {
        qCritical() << "Couldn't change perm for " << eth2path;
    }
}
#endif
void Recovery::sync()
{
    qDebug() << "[Recovery] SYNC";
    QString program = "sync";
    QProcess *myProcess = new QProcess(this);
    myProcess->setProgram(program);
    myProcess->start();
    myProcess->waitForFinished();
}

void Recovery::restartNetwork()
{
    qInfo() << "[Recovery] Restarting network";
    QString program = "/etc/init.d/networking";
    QStringList arguments { "restart" };
    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);
    myProcess->waitForFinished();
}

// void Recovery::receiveBlock(const DataTypes::BlockStruct blk)
void Recovery::receiveBlock(const QVariant &msg)
{
    auto blk = msg.value<DataTypes::BlockStruct>();
    qDebug() << "Block received, ID: " << blk.ID << ", data: " << blk.data;
    switch (blk.data.size())
    {
    case sizeof(AVTUK_CCU::Main):
    {
        AVTUK_CCU::Main mainBlock;
        memcpy(&mainBlock, blk.data.data(), sizeof(mainBlock));
        qDebug() << "[Recovery] Main block has been received from MCU, reset is: " << mainBlock.resetReq
                 << ", PWRIN: " << mainBlock.PWRIN;
        if (mainBlock.resetReq && (!resetInit))
        {
            eth0();
#if defined(AVTUK_NO_STM)
            eth1();
#endif
#if defined(AVTUK_STM)
            eth2();
#endif
            sync();
            restartNetwork();
            sync();
            emit rebootReq();
            resetInit = true;
        }
    }
    }
}
