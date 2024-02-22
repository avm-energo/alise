#include "interfaces/parsers/base_response_parser.h"

namespace Interface
{

BaseResponseParser::BaseResponseParser(QObject *parent)
    : QObject(parent), m_isFirstSectionReceived(true), m_isLastSectionReceived(false), m_isLastSectionSent(false)
{
}

void BaseResponseParser::setRequest(const CommandStruct &request) noexcept
{
    m_request = request;
}

bool BaseResponseParser::isLastSectionReceived() const noexcept
{
    return m_isLastSectionReceived;
}

void BaseResponseParser::accumulateToResponseBuffer(const QByteArray &responsePart) noexcept
{
    m_responseBuffer.append(responsePart);
}

const QByteArray &BaseResponseParser::getResponseBuffer() const noexcept
{
    return m_responseBuffer;
}

void BaseResponseParser::clearResponseBuffer() noexcept
{
    m_responseBuffer.clear();
}

void BaseResponseParser::lastSectionSent() noexcept
{
    m_isLastSectionSent = true;
}

void BaseResponseParser::processProgressCount(const quint64 count) noexcept
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::DataCount, count };
    emit responseParsed(resp);
}

void BaseResponseParser::processProgressRange(const quint64 count) noexcept
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::DataSize, count };
    emit responseParsed(resp);
}

void BaseResponseParser::processOk() noexcept
{
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::Ok, 0 };
    emit responseParsed(resp);
}

void BaseResponseParser::processError(int errorCode) noexcept
{
    qCritical() << "Device error code: " << QString::number(errorCode, 16);
    DataTypes::GeneralResponseStruct resp { DataTypes::GeneralResponseTypes::Error, static_cast<quint64>(errorCode) };
    emit responseParsed(resp);
}

} // namespace Interface
