#include "interfaces/parsers/protocom_request_parser.h"

#include <gen/files.h>
#include <gen/stdfunc.h>

namespace Interface
{

const std::map<Interface::Commands, Proto::Commands> ProtocomRequestParser::s_protoCmdMap {
    { Commands::C_ReqTime, Proto::ReadTime },                      //
    { Commands::C_ReqBSI, Proto::ReadBlkStartInfo },               //
    { Commands::C_ReqBSIExt, Proto::ReadBlkStartInfoExt },         //
    { Commands::C_StartFirmwareUpgrade, Proto::WriteUpgrade },     //
    { Commands::C_SetNewConfiguration, Proto::WriteBlkTech },      //
    { Commands::C_WriteUserValues, Proto::WriteBlkData },          //
    { Commands::C_EraseJournals, Proto::EraseTech },               //
    { Commands::C_ReqProgress, Proto::ReadProgress },              //
    { Commands::C_EraseTechBlock, Proto::EraseTech },              //
    { Commands::C_Test, Proto::Test },                             //
    { Commands::C_WriteSingleCommand, Proto::WriteSingleCommand }, //
    { Commands::C_ReqTuningCoef, Proto::ReadBlkAC },               //
    { Commands::C_WriteTuningCoef, Proto::WriteBlkAC },            //
    { Commands::C_ReqBlkData, Proto::ReadBlkData },                //
    { Commands::C_ReqBlkDataA, Proto::ReadBlkDataA },              //
    { Commands::C_ReqBlkDataTech, Proto::ReadBlkTech },            //
    { Commands::C_ReqOscInfo, Proto::ReadBlkTech },                //
    { Commands::C_WriteBlkDataTech, Proto::WriteBlkTech },         //
    { Commands::C_Reboot, Proto::WriteBlkCmd },                    //
    { Commands::C_GetMode, Proto::ReadMode },                      //
    { Commands::C_SetMode, Proto::WriteMode },                     //
    { Commands::C_WriteHiddenBlock, Proto::WriteHiddenBlock }            //
};

ProtocomRequestParser::ProtocomRequestParser(QObject *parent) : BaseRequestParser(parent)
{
}

void ProtocomRequestParser::basicProtocolSetup() noexcept
{
    using namespace Protocol;
    m_protocol.addGroup(ProtocomGroup { 1, 15, 0 }); // BSI request
    /// TODO: добавить загрузку ВПО, секретные операции
}

QByteArray ProtocomRequestParser::parse(const CommandStruct &cmd)
{
    m_request.clear();
    switch (cmd.command)
    {
    // commands requesting regs with addresses ("fake" read regs commands)
    case Commands::C_ReqAlarms:
    case Commands::C_ReqStartup:
    case Commands::C_ReqFloats:
    case Commands::C_ReqBitStrings:
    {
        const auto addr = cmd.arg1.toUInt();
        const auto group = getGroupByAddress(addr);
        if (group.m_startAddr == addr)
        {
            const auto block = static_cast<quint8>(group.m_block); // сужающий каст
            m_request = prepareBlock(Proto::Commands::ReadBlkData, StdFunc::toByteArray(block));
            m_continueCommand = createContinueCommand(Proto::Commands::ReadBlkData);
        }
        break;
    }
    // commands without any arguments
    case Commands::C_ReqBSI:
    case Commands::C_ReqBSIExt:
    case Commands::C_ReqTime:
    case Commands::C_StartFirmwareUpgrade:
    case Commands::C_ReqProgress:
    case Commands::C_GetMode:
    {
        if (isSupportedCommand(cmd.command))
            m_request = prepareBlock(s_protoCmdMap.at(cmd.command), QByteArray());
        break;
    }
    // commands with only one uint8 parameter (blocknum or smth similar)
    case Commands::C_EraseTechBlock:
    case Commands::C_EraseJournals:
    case Commands::C_SetMode:
    case Commands::C_Reboot:
    case Commands::C_ReqTuningCoef:
    case Commands::C_ReqBlkData:
    case Commands::C_ReqBlkDataA:
    case Commands::C_ReqBlkDataTech:
    case Commands::C_ReqOscInfo:
    case Commands::C_Test:
    {
        if (isSupportedCommand(cmd.command))
        {
            const auto protoCommand = s_protoCmdMap.at(cmd.command);
            m_request = prepareBlock(protoCommand, StdFunc::toByteArray(cmd.arg1.value<quint8>()));
            m_continueCommand = createContinueCommand(protoCommand);
        }
        break;
    }
    // file request: known file types should be download from disk and others must be taken from module by Protocom,
    // arg1 - file number
    case Commands::C_ReqFile:
    {
        m_request = prepareBlock(Proto::Commands::ReadFile, StdFunc::toByteArray(cmd.arg1.value<quint16>()));
        m_continueCommand = createContinueCommand(Proto::Commands::ReadFile);
        break;
    }
    // commands with one bytearray argument arg2
    case Commands::C_WriteFile:
    {
        if (cmd.arg2.canConvert<QByteArray>())
            m_request = writeLongData(Proto::Commands::WriteFile, cmd.arg2.value<QByteArray>());
        break;
    }
    // write time command with different behaviour under different OS's
    case Commands::C_WriteTime:
    {
        QByteArray tmpba;
#ifdef Q_OS_LINUX
        if (cmd.arg1.canConvert<timespec>())
        {
            timespec time = cmd.arg1.value<timespec>();
            tmpba.push_back(StdFunc::toByteArray(quint32(time.tv_sec)));
            tmpba.push_back(StdFunc::toByteArray(quint32(time.tv_nsec)));
        }
        else
#endif
        {
            tmpba = StdFunc::toByteArray(cmd.arg1.value<quint32>());
        }
        m_request = prepareBlock(Proto::Commands::WriteTime, tmpba);
        break;
    }
    // block write, arg1 is BlockStruct of one quint32 (block ID) and one QByteArray (block contents)
    case Commands::C_WriteHiddenBlock:
    case Commands::C_WriteBlkDataTech:
    case Commands::C_SetNewConfiguration:
    case Commands::C_WriteTuningCoef:
    {
        if (cmd.arg1.canConvert<DataTypes::BlockStruct>())
        {
            auto bs = cmd.arg1.value<DataTypes::BlockStruct>();
            QByteArray tmpba = StdFunc::toByteArray(static_cast<quint8>(bs.ID)); // сужающий каст
            tmpba.append(bs.data);
            m_request = writeLongData(s_protoCmdMap.at(cmd.command), tmpba);
        }
        break;
    }
    // Block write
    case Commands::C_WriteUserValues:
    {
        if (cmd.arg1.canConvert<DataTypes::BlockStruct>())
        {
            DataTypes::BlockStruct bs = cmd.arg1.value<DataTypes::BlockStruct>();
            QByteArray tmpba = StdFunc::toByteArray(static_cast<quint8>(bs.ID));
            tmpba.append(bs.data);
            m_request = writeLongData(s_protoCmdMap.at(cmd.command), tmpba);
        }
        break;
    }
    // WS Commands
    case Commands::C_WriteSingleCommand:
    {
        if (cmd.arg1.canConvert<DataTypes::SingleCommand>())
        {
            DataTypes::SingleCommand scmd = cmd.arg1.value<DataTypes::SingleCommand>();
            auto tmpba = StdFunc::toByteArray(scmd.addr) + StdFunc::toByteArray(scmd.value);
            m_request = prepareBlock(Proto::WriteSingleCommand, tmpba);
        }
        break;
    }
    // "WS" commands
    case Commands::C_ClearStartupError:
    case Commands::C_ClearStartupUnbounced:
    case Commands::C_ClearStartupValues:
    case Commands::C_SetStartupValues:
    case Commands::C_SetStartupPhaseA:
    case Commands::C_SetStartupPhaseB:
    case Commands::C_SetStartupPhaseC:
    case Commands::C_SetStartupUnbounced:
    case Commands::C_StartWorkingChannel:
    case Commands::C_SetTransOff:
    {
        uint24 converted(s_wsCmdMap.at(cmd.command));
        QByteArray tmpba = StdFunc::toByteArray(converted) + StdFunc::toByteArray(cmd.arg1.value<quint8>());
        m_request = prepareBlock(Proto::WriteSingleCommand, tmpba);
        break;
    }
    case Commands::C_EnableHardwareWriting:
    {
        setExceptionalSituationStatus(true);
        break;
    }
    default:
        qCritical() << "Undefined command: " << cmd.command;
    }
    return m_request;
}

QByteArray ProtocomRequestParser::getNextContinueCommand() noexcept
{
    return m_continueCommand;
}

void ProtocomRequestParser::exceptionalAction(const CommandStruct &cmd) noexcept
{
    Q_UNUSED(cmd);
    setExceptionalSituationStatus(false);
}

bool ProtocomRequestParser::isSupportedCommand(const Commands command) const noexcept
{
    const auto search = s_protoCmdMap.find(command);
    return search != s_protoCmdMap.cend();
}

Protocol::ProtocomGroup ProtocomRequestParser::getGroupByAddress(const quint32 addr) const noexcept
{
    return getGroupsByAddress<Protocol::ProtocomGroup>(addr);
}

QByteArray ProtocomRequestParser::prepareBlock(
    const Proto::Commands cmd, const QByteArray &data, Proto::Starters startByte)
{
    QByteArray ba;
    ba.append(startByte);
    ba.append(cmd);
    auto length = static_cast<quint16>(data.size());
    ba.append(StdFunc::toByteArray(length));
    if (!data.isEmpty())
        ba.append(data);
    return ba;
}

void ProtocomRequestParser::prepareLongData(const Proto::Commands cmd, const QByteArray &data)
{
    using Proto::MaxSegmenthLength;
    m_progressCount = 0;
    m_longDataSections.clear();
    // Количество сегментов
    quint64 segCount = (data.size() + 1) // +1 Т.к. некоторые команды имеют в значимой части один дополнительный байт
            / MaxSegmenthLength          // Максимальная длинна сегмента
        + 1; // Добавляем еще один сегмент, в него попадет последняя часть

    QByteArray tba = data.left(MaxSegmenthLength);
    m_longDataSections.push_back(prepareBlock(cmd, tba));
    for (int pos = MaxSegmenthLength; pos < data.size(); pos += MaxSegmenthLength)
    {
        tba = data.mid(pos, MaxSegmenthLength);
        m_longDataSections.push_back(prepareBlock(cmd, tba, Proto::Starters::Continue));
    }
    emit totalWritingBytes(data.size() + segCount * 4);
}

QByteArray ProtocomRequestParser::writeLongData(const Proto::Commands cmd, const QByteArray &data)
{
    if (std::size_t(data.size()) > Proto::MaxSegmenthLength)
    {
        emit writingLongData();
        prepareLongData(cmd, data);
        return getNextDataSection(); // send first chunk
    }
    else
    {
        emit writingLastSection();
        return prepareBlock(cmd, data);
    }
}

QByteArray ProtocomRequestParser::createContinueCommand(const Proto::Commands cmd) const noexcept
{
    QByteArray continueCmd;
    continueCmd.append(Proto::Starters::Continue);
    // NOTE Михалыч не следует документации поэтому пока так
    // tmpba.append(Proto::Commands::ResultOk);
    continueCmd.append(cmd);
    continueCmd.append(StdFunc::toByteArray(quint16(0x0000)));
    return continueCmd;
}

} // namespace Interface
