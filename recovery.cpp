#include "recovery.h"

#include "avtukccu.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <gen/datatypes.h>

constexpr char ethPathString[] = "/etc/network/interfaces.d/eth";
constexpr char ethResourcePathString[] = ":/network/eth";

RecoveryEngine::RecoveryEngine(QObject *parent) : QObject(parent)
{
}

void RecoveryEngine::setDefEth(int ethNum)
{
    QString ethLetter = QString::number(ethNum);
    QString ethPath = ethPathString + ethLetter;
    QString ethResourcePath = ethResourcePathString + ethLetter;
    qDebug() << "[Recovery] Set default ip eth" + ethLetter;
    if (!QFile::exists(ethResourcePath))
    {
        qCritical() << "No eth0 recovery";
        return;
    }
    if (QFile::exists(ethPath))
    {
        QFile::remove(ethPath);
    }

    if (!QFile::copy(ethResourcePath, ethPath))
    {
        qCritical() << "Couldn't copy eth" + ethLetter;
        return;
    }
    if (!QFile(ethPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser
            | QFileDevice::ReadGroup | QFileDevice::ReadOther))
    {
        qCritical() << "Couldn't change perm for " << ethPath;
    }
}

void RecoveryEngine::sync()
{
    qDebug() << "[Recovery] SYNC";
    QString program = "sync";
    QProcess *myProcess = new QProcess(this);
    myProcess->setProgram(program);
    myProcess->start();
    myProcess->waitForFinished();
}

void RecoveryEngine::restartNetwork()
{
    qInfo() << "[Recovery] Restarting network";
    QString program = "/etc/init.d/networking";
    QStringList arguments { "restart" };
    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);
    myProcess->waitForFinished();
}

void RecoveryEngine::receiveBlock(const QVariant &msg)
{
    auto blk = msg.value<DataTypes::BlockStruct>();
    qDebug() << "[Recovery] <= MCU : Block ID = " << blk.ID << ", data = " << blk.data;
    switch (blk.ID)
    {
    case AVTUK_CCU::MainBlock:
    {
        AVTUK_CCU::Main mainBlock;
        memcpy(&mainBlock, blk.data.data(), sizeof(mainBlock));
        //        qDebug() << "[Recovery] Main block has been received from MCU, reset is: " << mainBlock.resetReq;
        if (mainBlock.resetReq && (!resetInit))
        {
            setDefEth(0);
#if defined(AVTUK_NO_STM)
            setDefEth(1);
#endif
#if defined(AVTUK_STM)
            setDefEth(2);
#endif
            sync();
            restartNetwork();
            emit rebootReq();
            resetInit = true;
        }
    }
    }
}
