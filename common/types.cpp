#include <QString>
#include <QJsonObject>

#include "types.hpp"

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

QString typeToString(EntityType type)
{
    switch(type) {
        case EntityType::UE: return "UE";
        case EntityType::GNB: return "gNB";
        default: return "Unknown";
    }
}

QString toString(RegistrationStatus reg_status)
{
    switch(reg_status) {
    case RegistrationStatus::Rejected:
        return "RegistrationStatus::Rejected";
    case RegistrationStatus::Accepted:
        return "RegistrationStatus::Accepted";
    case RegistrationStatus::Pending:
        return "RegistrationStatus::Pending";
    default:
        return "Unkonwn RegistrationStatus";
    }
}

QString toString(UeRrcState ue_rrc_state)
{
    switch (ue_rrc_state) {
        case UeRrcState::DETACHED:
            return "UeRrcState::DEACHED";
        case UeRrcState::SEARCHING_FOR_CELL:
            return "UeRrcState::SEARCHING_FOR_CELL";
        case UeRrcState::RRC_IDLE:
            return "UeRrcState::RRC_IDLE";
        case UeRrcState::RRC_CONNECTING:
            return "UeRrcState::RRC_CONNECTING" ;
        case UeRrcState::RRC_CONNECTED:
            return "RRC_CONNECTED";
        case UeRrcState::RRC_INACTIVE:
            return "RRC_INACTIVE";
        default:
            return "unknown";
    }
}
