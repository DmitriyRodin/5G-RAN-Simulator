#include "types.hpp"

#include <QJsonObject>
#include <QString>

QDebug operator<<(QDebug stream, EntityType type)
{
    switch (type) {
        case EntityType::GNB:
            stream.nospace() << "GNB";
            break;
        case EntityType::UE:
            stream.nospace() << "UE";
            break;
        case EntityType::UNKNOWN:
            stream.nospace() << "Unknown";
            break;
        default:
            stream.nospace() << "ErrorType";
            break;
    }
    return stream;
}

QString typeToString(EntityType type)
{
    switch (type) {
        case EntityType::UE:
            return "UE";
        case EntityType::GNB:
            return "gNB";
        default:
            return "Unknown";
    }
}

QString toString(RegistrationStatus reg_status)
{
    switch (reg_status) {
        case RegistrationStatus::Rejected:
            return "RegistrationStatus::Rejected";
        case RegistrationStatus::Accepted:
            return "RegistrationStatus::Accepted";
        case RegistrationStatus::Pending:
            return "RegistrationStatus::Pending";
        default:
            return "RegistrationStatus: Unkonwn";
    }
}

QString toString(UeRrcState ue_rrc_state)
{
    switch (ue_rrc_state) {
        case UeRrcState::RRC_IDLE:
            return "RRC_IDLE";
        case UeRrcState::RRC_CONNECTED:
            return "RRC_CONNECTED";
        case UeRrcState::RRC_INACTIVE:
            return "RRC_INACTIVE";
        default:
            return "UeRrcState: Unknown";
    }
}

QString toString(CellSearchStatus cell_status)
{
    switch (cell_status) {
        case CellSearchStatus::CELL_SELECTION:
            return "CELL_SELECTION";
        case CellSearchStatus::CAMPED:
            return "CAMPED";
        default:
            return "CellSearchStatus: unknown";
    }
}

QDataStream& operator<<(QDataStream& out, const RachConfigCommon& config)
{
    out << config.total_number_of_RA_preambles;
    out << config.preamble_trans_max;
    out << config.ra_response_window_ms;
    out << config.preamble_received_target_power;
    out << config.power_ramping_step;
    return out;
}

QDataStream& operator>>(QDataStream& in, RachConfigCommon& config)
{
    in >> config.total_number_of_RA_preambles;
    in >> config.preamble_trans_max;
    in >> config.ra_response_window_ms;
    in >> config.preamble_received_target_power;
    in >> config.power_ramping_step;
    return in;
}

QDataStream& operator<<(QDataStream& out, const SibMapping& sib_map)
{
    out << static_cast<uint8_t>(sib_map.sib_type);
    out << sib_map.periodicity_ms;
    return out;
}
QDataStream& operator>>(QDataStream& in, SibMapping& sib_map)
{
    in >> sib_map.sib_type;
    in >> sib_map.periodicity_ms;
    return in;
}

QDataStream& operator<<(QDataStream& out, const SiSchedulingInfo& info)
{
    out << info.si_window_length_ms;
    out << info.system_info_value_tag;
    out << static_cast<uint32_t>(info.scheduled_sibs.size());
    for (const auto& sib : info.scheduled_sibs) {
        out << sib;
    }
    return out;
}
QDataStream& operator>>(QDataStream& in, SiSchedulingInfo& info)
{
    in >> info.si_window_length_ms;
    in >> info.system_info_value_tag;
    const uint32_t vector_size = 0;
    for (uint32_t i = 0; i < vector_size; ++i) {
        SibMapping sib_mapping;
        in >> sib_mapping;
        info.scheduled_sibs.push_back(std::move(sib_mapping));
    }
    return in;
}
