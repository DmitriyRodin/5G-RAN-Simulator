#include <QString>
#include <QJsonObject>

#include "types.hpp"

MessageType parseMessageType(const QJsonObject& obj)
{
    const QString type = obj["type"].toString();

    if (type == "UAER_PLANE_DATA")                return MessageType::UserPlaneData;

    if (type == "MEASUREMENT_REPORT")             return MessageType::MeasurementReport;
    if (type == "RRC_RECONFIGURATION")            return MessageType::RrcReconfiguration;
    if (type == "RRC_RECONFIGURATION_COMPLETE")   return MessageType::RrcReconfigurationComplete;

    if (type == "SIB1")                           return MessageType::Sib1;
    if (type == "RACH_PREAMBLE")                  return MessageType::RachPreamble;
    if (type == "RAR")                            return MessageType::Rar;

    if (type == "REGISTRATION_REQUEST")           return MessageType::RegistrationRequest;
    if (type == "REGISTRATION_ACCEPT")            return MessageType::RegistrationAccept;
    if (type == "DEREGISTRATION_REQUEST")         return MessageType::DeregistrationRequest;
    if (type == "SERVICE_REQUEST")                return MessageType::ServiceRequest;
    if (type == "PAGING")                         return MessageType::Paging;

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
