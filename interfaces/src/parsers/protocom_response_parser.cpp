#include "interfaces/parsers/protocom_response_parser.h"

namespace Interface
{

ProtocomResponseParser::ProtocomResponseParser(QObject *parent) : BaseResponseParser(parent)
{
}

bool ProtocomResponseParser::isCompleteResponse()
{
    constexpr auto protocomMinResponseSize = 4;
    if (m_responseBuffer.size() >= protocomMinResponseSize)
    {
        auto size = quint16(m_responseBuffer.at(2));
        if (m_responseBuffer.size() >= (size + protocomMinResponseSize))
            return true;
    }
    return false;
}

Error::Msg ProtocomResponseParser::validate()
{
    // parsing protocom header
    auto startByte = Proto::Starters(m_responseBuffer.at(0)); // start byte
    auto size = quint16(m_responseBuffer.at(2));              // size of data
    // only response should be received from device
    if (startByte == Proto::Starters::Response)
    {
        // checking size limits
        if (size <= Proto::MaxSegmenthLength)
            return Error::NoError;
        else
            return Error::SizeError;
    }
    else
        return Error::WrongCommandError;
}

void ProtocomResponseParser::parse()
{
    // Убираем заголовок Protocom, подготовка
    m_receivedCommand = Proto::Commands(m_responseBuffer.at(1));
    auto size = quint16(m_responseBuffer.at(2));
    m_responseBuffer.remove(0, 4);
    m_responseBuffer.resize(size);
    quint32 addr = m_request.arg1.toUInt();

    switch (m_receivedCommand)
    {
    case Proto::Commands::ResultOk:
        if (m_request.command == Commands::C_WriteFile || m_request.command == Commands::C_WriteHiddenBlock)
        {
            if (m_isLastSectionSent)
            {
                processOk();
                m_isLastSectionSent = false;
            }
        }
        else
            processOk();
        break;
    case Proto::Commands::ResultError:
        processError(quint8(m_responseBuffer.front()));
        break;
    case Proto::Commands::ReadTime:
#ifdef Q_OS_LINUX
        if (m_responseBuffer.size() == sizeof(quint64))
            processUnixTime(m_responseBuffer);
        else
#endif
            processU32(m_responseBuffer, addr);
        break;
    case Proto::Commands::ReadBlkStartInfo:
    case Proto::Commands::ReadBlkStartInfoExt:
        // Update data
        if (boardType.mTypeB != m_responseBuffer[0])
            boardType.mTypeB = m_responseBuffer[0];
        if (boardType.mTypeM != m_responseBuffer[4])
            boardType.mTypeM = m_responseBuffer[4];
        processU32(m_responseBuffer, addr);
        break;
    case Proto::Commands::ReadBlkAC:
    case Proto::Commands::ReadBlkDataA:
        processDataSection(m_responseBuffer);
        if (m_isLastSectionReceived)
        {
            processBlock(m_longDataBuffer, addr); // Ожидается что в addr хранится номер блока
            m_longDataBuffer.clear();
        }
        break;
    // В протокоме данные могут не влезать в одну посылку
    case Proto::Commands::ReadBlkData:
        processDataSection(m_responseBuffer);
        if (m_isLastSectionReceived)
        {
            processDataBlock(m_longDataBuffer, addr);
            m_longDataBuffer.clear();
        }
        break;
    case Proto::Commands::ReadProgress:
        processU32(m_responseBuffer, addr);
        break;
    case Proto::Commands::ReadMode:
        processInt(m_responseBuffer.toInt());
        break;
    default:
        qCritical("We shouldn't be here, something went wrong");
        qCritical() << m_responseBuffer.toHex();
        break;
    }
    clearResponseBuffer();
}

#ifdef Q_OS_LINUX
void ProtocomResponseParser::processUnixTime(const QByteArray &data)
{
    Q_ASSERT(data.size() == sizeof(quint64));
    const quint32 secs = *reinterpret_cast<const quint32 *>(data.data());
    const quint32 nsecs = *reinterpret_cast<const quint32 *>(data.data() + sizeof(quint32));
    timespec resp;
    resp.tv_nsec = nsecs;
    resp.tv_sec = secs;
    emit responseParsed(resp);
}
#endif

void ProtocomResponseParser::processU32(const QByteArray &data, quint16 startAddr)
{
    Q_ASSERT(data.size() % sizeof(quint32) == 0);
    Q_ASSERT(data.size() >= 4);
    for (std::size_t i = 0; i != (data.size() / sizeof(quint32)); i++)
    {
        QByteArray tba = data.mid(sizeof(qint32) * i, sizeof(qint32));
        quint32 value = *reinterpret_cast<const quint32 *>(tba.data());
        DataTypes::BitStringStruct resp { startAddr++, value, DataTypes::Quality::Good };
        emit responseParsed(resp);
    }
}

void ProtocomResponseParser::processFloat(const QByteArray &data, quint32 startAddr)
{
    // NOTE Проблема со стартовыми регистрами, получим на один регистр больше чем по другим протоколам
    Q_ASSERT(std::size_t(data.size()) >= sizeof(float));     // должен быть хотя бы один флоат
    Q_ASSERT(std::size_t(data.size()) % sizeof(float) == 0); // размер кратен размеру флоат
    int bapos = 0;
    const int baendpos = data.size();
    while (bapos != baendpos)
    {
        QByteArray tba = data.mid(bapos, sizeof(float));
        float blk = *reinterpret_cast<const float *>(tba.data());
        DataTypes::FloatStruct resp { startAddr++, blk, DataTypes::Quality::Good };
        emit responseParsed(resp);
        bapos += sizeof(float);
    }
}

void ProtocomResponseParser::processInt(const quint8 num)
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::Ok, num };
    emit responseParsed(resp);
}

void ProtocomResponseParser::processSinglePoint(const QByteArray &data, const quint16 startAddr)
{
    for (quint32 i = 0; i != quint32(data.size()); ++i)
    {
        quint8 value = data.at(i);
        DataTypes::SinglePointWithTimeStruct response { (startAddr + i), value, 0, DataTypes::Quality::Good };
        emit responseParsed(response);
    }
}

void ProtocomResponseParser::processBlock(const QByteArray &data, quint32 blockNum)
{
    DataTypes::BlockStruct resp { blockNum, data };
    emit responseParsed(resp);
}

void ProtocomResponseParser::processDataBlock(const QByteArray &data, const quint16 addr)
{
    switch (m_request.command)
    {
    case Commands::C_ReqStartup:
    case Commands::C_ReqFloats:
        processFloat(data, addr);
        break;
    case Commands::C_ReqAlarms:
        processSinglePoint(data, addr);
        break;
    case Commands::C_ReqBitStrings:
        processU32(data, addr);
        break;
    default:
        processBlock(data, addr);
        break;
    }
}

void ProtocomResponseParser::processDataSection(const QByteArray &dataSection)
{
    // Проверяем, получили мы последнюю секцию, или нет
    m_isLastSectionReceived = (std::size_t(dataSection.size()) < Proto::MaxSegmenthLength);
    // Добавляем полученные данные в буфер
    m_longDataBuffer.append(dataSection);
    // Если получили первую секцию
    if (m_isFirstSectionReceived && !m_isLastSectionReceived)
        emit readingLongData();
    if (m_receivedCommand == Proto::Commands::ReadFile)
        processProgressCount(m_longDataBuffer.size()); // Отсылаем текущий прогресс
    // Восстанавливаем флаг, когда получаем последнюю секцию
    m_isFirstSectionReceived = m_isLastSectionReceived;
}

} // namespace Interface
