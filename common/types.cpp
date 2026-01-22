#include <QString>
#include <QJsonObject>

#include "types.hpp"

MessageType parseMessageType(const QJsonObject& obj)
{
    const QString type = obj["type"].toString();

    if (type == "SIB1")                return MessageType::Sib1;
    if (type == "ATTACH_REQUEST")      return MessageType::AttachRequest;
    if (type == "ATTACH_ACCEPT")       return MessageType::AttachAccept;
    if (type == "MEASUREMENT_REPORT")  return MessageType::MeasurementReport;
    if (type == "RRC_RECONFIGURATION") return MessageType::RRCReconfiguration;
    if (type == "DATA_TRANSFER")       return MessageType::DataTransfer;    

    return MessageType::Unknown;
}

QDebug operator<<(QDebug stream, EntityType type)
{
    switch(type) {
        case EntityType::GNB:     stream.nospace() << "GNB"; break;
        case EntityType::UE:      stream.nospace() << "UE"; break;
        case EntityType::UNKNOWN: stream.nospace() << "Unknown"; break;
        default:                  stream.nospace() << "ErrorType"; break;
    }
    return stream;
}
