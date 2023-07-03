#include "protocomthread.h"

#include <QDebug>
#include <QQueue>

#ifdef Q_OS_LINUX
#include <time.h>
#endif

// typedef QQueue<QByteArray> ByteQueue;
using DataTypes::FileFormat;
using Interface::Direction;
using Proto::CommandStruct;
using Proto::Starters;
using namespace Interface;

ProtocomThread::ProtocomThread(QObject *parent) : BaseInterfaceThread(parent)
{
    isFirstBlock = true;
    m_longBlockChunks.clear();
}

ProtocomThread::~ProtocomThread()
{
}

void ProtocomThread::processReadBytes(QByteArray ba)
{
    QMutexLocker locker(&_mutex);
    if (!isValidIncomingData(ba))
    {
        finishCommand();
        return;
    }

    m_responseReceived = Proto::Commands(ba.at(1)); // received from device "command"
    auto size = quint16(ba.at(2));
    writeLog(ba, Interface::FromDevice);
    ba.remove(0, 4);
    ba.resize(size);
    m_readData.append(ba);

    // Если ответе меньше 60 байт или пришёл BSI
    if (isOneSegment(size) || (m_responseReceived == Proto::ReadBlkStartInfo))
    {
        m_parsingDataReady = true;
        isFirstBlock = true; // prepare bool for the next receive iteration
        wakeUp();
    }
    // Пришло 60 байт
    else
    {
        auto tba = prepareOk(false, m_responseReceived); // prepare "Ok" answer to the device
        Q_ASSERT(tba.size() == 4);
        isFirstBlock = false;     // there'll be another segment
        emit sendDataToPort(tba); // write "Ok" to the device
    }
}

void ProtocomThread::parseRequest(const CommandStruct &cmdStr)
{
    QByteArray ba;
#ifdef PROTOCOM_DEBUG
    qDebug("Start parse request");
#endif

    switch (cmdStr.command)
    {
    // commands without any arguments
    case Commands::C_ReqTime:
    {
        if (protoCommandMap.contains(cmdStr.command))
        {
            ba = prepareBlock(protoCommandMap.value(cmdStr.command));
            emit sendDataToPort(ba);
        }
        break;
    }
    // commands with only one uint8 parameter (blocknum or smth similar)
    case Commands::C_Reboot:
    case Commands::C_ReqBlkData:
    {
        if (protoCommandMap.contains(cmdStr.command))
        {
            ba = prepareBlock(
                protoCommandMap.value(cmdStr.command), StdFunc::ArrayFromNumber(cmdStr.arg1.value<quint8>()));
            emit sendDataToPort(ba);
        }
        break;
    }

    // write time command with different behaviour under different OS's
    case Commands::C_WriteTime:
    {
        QByteArray tba;
#ifdef Q_OS_LINUX
        if (cmdStr.arg1.canConvert<timespec>())
        {
            timespec time = cmdStr.arg1.value<timespec>();
            tba.push_back(StdFunc::ArrayFromNumber(quint32(time.tv_sec)));
            tba.push_back(StdFunc::ArrayFromNumber(quint32(time.tv_nsec)));
        }
        else
#endif
        {
            tba = StdFunc::ArrayFromNumber(cmdStr.arg1.value<quint32>());
        }
        ba = prepareBlock(Proto::Commands::WriteTime, tba);
        emit sendDataToPort(ba);
        break;
    }

    // QVariantList write
    case Commands::C_WriteUserValues:
    {
        if (cmdStr.arg1.canConvert<DataTypes::BlockStruct>())
        {
            DataTypes::BlockStruct bs = cmdStr.arg1.value<DataTypes::BlockStruct>();
            ba = StdFunc::ArrayFromNumber(bs.ID);
            ba.append(bs.data);
            writeBlock(protoCommandMap.value(cmdStr.command), ba);
        }
        break;
    }

    default:
        qDebug() << "There's no such command";
    }
}

void ProtocomThread::parseResponse()
{
    using namespace Proto;
    using namespace DataTypes;
    quint32 addr = m_currentCommand.arg1.toUInt();
    switch (m_responseReceived)
    {
    case ResultOk:
    {
        if (!m_longBlockChunks.isEmpty())
        {
            QByteArray ba = m_longBlockChunks.takeFirst();
            m_sentBytesCount += ba.size();
            emit sendDataToPort(ba);
            _waiter.wakeOne();
            return;
        }
        processOk();
        break;
    }

    case ResultError:
    {
        const quint8 errorCode = m_readData.front();
        processError(errorCode);
        break;
    }

    case ReadTime:
#ifdef Q_OS_LINUX
        if (m_readData.size() == sizeof(quint64))
        {
            processUnixTime(m_readData);
            break;
        }
#endif
        processU32(m_readData, addr);
        break;

    case ReadBlkData:
        processBlock(m_readData, addr);
        break;

    default:
        qCritical("We shouldn't be here, something went wrong");
        qDebug() << m_readData.toHex();
        break;
    }
    finishCommand();
}

void ProtocomThread::writeLog(const QByteArray &ba, Direction dir)
{
#ifdef PROTOCOM_DEBUG
    QString msg = metaObject()->className();
    switch (dir)
    {
    case Interface::FromDevice:
        msg += ": <- ";
        break;
    case Interface::ToDevice:
        msg += ": -> ";
        break;
    default:
        msg += ": ";
        break;
    }
    msg += ba.toHex();
    qInfo() << msg;
#else
    Q_UNUSED(ba);
    Q_UNUSED(dir);
#endif
}

void ProtocomThread::appendInt16(QByteArray &ba, quint16 data)
{
    ba.append(static_cast<char>(data % 0x100));
    ba.append(static_cast<char>(data / 0x100));
}

bool ProtocomThread::isOneSegment(quint16 length)
{
    // Если размер меньше MaxSegmenthLength то сегмент считается последним (единственным)
    Q_ASSERT(length <= Proto::MaxSegmenthLength);
    return (length != Proto::MaxSegmenthLength);
}

bool ProtocomThread::isSplitted(quint16 length)
{
    return !(length < Proto::MaxSegmenthLength);
}

bool ProtocomThread::isValidIncomingData(const QByteArray &data)
{
    // if there's no standard header
    if (data.size() >= 4)
    {
        // parsing protocom header
        auto startByte = Proto::Starters(data.at(0)); // start byte
        auto size = quint16(data.at(2));              // size of data
        // only response should be received from device
        if (startByte == Proto::Starters::Response)
        {
            // checking size limits
            if (size <= Proto::MaxSegmenthLength)
                return true;
            else
                qCritical() << Error::SizeError << size;
        }
        else
            qCritical() << Error::WrongCommandError << startByte;
    }
    else
        qCritical() << Error::HeaderSizeError << data.toHex();

    return false;
}

QByteArray ProtocomThread::prepareOk(bool isStart, byte cmd)
{
    QByteArray tmpba;
    if (isStart)
        tmpba.append(Proto::Starters::Request);
    else
        tmpba.append(Proto::Starters::Continue);
    // NOTE Михалыч не следует документации поэтому пока так
    // tmpba.append(Proto::Commands::ResultOk);
    tmpba.append(cmd);
    appendInt16(tmpba, 0x0000);
    return tmpba;
}

QByteArray ProtocomThread::prepareError()
{
    QByteArray tmpba;
    tmpba.append(Proto::Starters::Request);
    tmpba.append(Proto::Commands::ResultError);
    appendInt16(tmpba, 0x0001);
    // модулю не нужны коды ошибок
    tmpba.append(static_cast<char>(Proto::NullByte));
    return tmpba;
}

QByteArray ProtocomThread::prepareBlock(Proto::Commands cmd, const QByteArray &data, Proto::Starters startByte)
{
    QByteArray ba;
    ba.append(startByte);
    ba.append(cmd);
    appendInt16(ba, data.size());

    if (!data.isEmpty())
        ba.append(data);
    return ba;
}

void ProtocomThread::writeBlock(Proto::Commands cmd, const QByteArray &arg2)
{
    using Proto::MaxSegmenthLength;
    QByteArray ba = arg2;

    if (isSplitted(ba.size()))
    {
        // prepareLongBlk
        m_longBlockChunks.clear();
        // Количество сегментов
        quint64 segCount = (ba.size() + 1) // +1 Т.к. некоторые команды имеют в значимой части один дополнительный байт
                / MaxSegmenthLength        // Максимальная длинна сегмента
            + 1; // Добавляем еще один сегмент в него попадет последняя часть
        m_longBlockChunks.reserve(segCount);

        QByteArray tba;
        tba.append(ba.left(MaxSegmenthLength));

        m_longBlockChunks.append(prepareBlock(cmd, tba));

        for (int pos = MaxSegmenthLength; pos < ba.size(); pos += MaxSegmenthLength)
        {
            tba = ba.mid(pos, MaxSegmenthLength);
            m_longBlockChunks.append(prepareBlock(cmd, tba, Proto::Starters::Continue));
        }
        m_sentBytesCount = m_longBlockChunks.at(0).size();
        emit sendDataToPort(m_longBlockChunks.takeFirst()); // send first chunk
    }
    else
    {
        ba = prepareBlock(cmd, ba);
        emit sendDataToPort(ba);
    }
}

#ifdef Q_OS_LINUX
void ProtocomThread::processUnixTime(const QByteArray &ba)
{
    Q_ASSERT(ba.size() == sizeof(quint64));

    const quint32 secs = *reinterpret_cast<const quint32 *>(ba.data());
    const quint32 nsecs = *reinterpret_cast<const quint32 *>(ba.data() + sizeof(quint32));
    timespec resp;
    resp.tv_nsec = nsecs;
    resp.tv_sec = secs;
    DataManager::GetInstance().addSignalToOutList(resp);
}
#endif

void ProtocomThread::processU32(const QByteArray &ba, quint16 startAddr)
{
    Q_ASSERT(ba.size() % sizeof(quint32) == 0);
    Q_ASSERT(ba.size() >= 4);
    for (int i = 0; i != (ba.size() / sizeof(quint32)); i++)
    {
        QByteArray tba = ba.mid(sizeof(qint32) * i, sizeof(qint32));
        quint32 value = *reinterpret_cast<const quint32 *>(tba.data());
        DataTypes::BitStringStruct resp { startAddr++, value, DataTypes::Quality::Good };
        DataManager::GetInstance().addSignalToOutList(resp);
    }
}

void ProtocomThread::processOk()
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::Ok, 0 };
    DataManager::GetInstance().addSignalToOutList(resp);
}

void ProtocomThread::processError(int errorCode)
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::Error, static_cast<quint64>(errorCode) };
    DataManager::GetInstance().addSignalToOutList(resp);
    // Module error code
    qCritical() << "Error code: " << QString::number(errorCode, 16);
}

void ProtocomThread::processBlock(const QByteArray &ba, quint32 blkNum)
{
    DataTypes::BlockStruct resp { blkNum, ba };
    DataManager::GetInstance().addSignalToOutList(resp);
}
