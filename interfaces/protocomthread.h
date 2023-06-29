#pragma once

#define PROTOCOM_DEBUG

#include "baseinterfacethread.h"
#include "protocomprivate.h"

namespace Interface
{

class ProtocomThread : public BaseInterfaceThread
{
    Q_OBJECT
public:
    explicit ProtocomThread(QObject *parent = nullptr);
    ~ProtocomThread();

public slots:
    void processReadBytes(QByteArray ba) override;

private:
    bool isFirstBlock;
    Proto::Commands m_commandSent;
    Proto::Commands m_responseReceived;
    QList<QByteArray> m_longBlockChunks;
    int m_sentBytesCount;

    void parseRequest(const CommandStruct &cmdStr) override;
    void parseResponse() override;

    void writeLog(const QByteArray &ba, Interface::Direction dir = Interface::NoDirection);
    void appendInt16(QByteArray &ba, quint16 data);
    bool isOneSegment(quint16 length);
    bool isSplitted(quint16 length);
    bool isValidIncomingData(const QByteArray &data);

    QByteArray prepareOk(bool isStart, byte cmd);
    QByteArray prepareError();
    QByteArray prepareBlock(Proto::Commands cmd, const QByteArray &data = QByteArray(),
        Proto::Starters startByte = Proto::Starters::Request);
    void writeBlock(Proto::Commands cmd, const QByteArray &arg2);

#ifdef Q_OS_LINUX
    void processUnixTime(const QByteArray &ba);
#endif
    void processU32(const QByteArray &ba, quint16 startAddr);
    void processOk();
    void processError(int errorCode = 0);
    void processBlock(const QByteArray &ba, quint32 blkNum);

    const QMap<Interface::Commands, Proto::Commands> protoCommandMap {
        { C_ReqTime, Proto::ReadTime },             // *
        { C_WriteUserValues, Proto::WriteBlkData }, // *
        { C_ReqBlkData, Proto::ReadBlkData },       // *
        { C_Reboot, Proto::WriteBlkCmd },           // *
    };
};

}
