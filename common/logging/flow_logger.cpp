#include "flow_logger.hpp"

void FlowLogger::log(const EntityType type, const uint32_t from,
                     const uint32_t to, const ProtocolMsgType msg_type,
                     const bool isIncoming)
{
    QMutexLocker locker(&logMutex);

    QString direction = isIncoming ? "  <----------  " : "  ---------->  ";
    QString msgName = msgTypeToString(msg_type);

    qDebug().noquote() << QString("[%1#%2] %3 %4[%5]  :  %6")
                              .arg(typeToString(type))
                              .arg(formatId(from))
                              .arg(direction)
                              .arg(typeToString(getOppositeType(type)))
                              .arg(formatId(to))
                              .arg(msgName);
}

QString FlowLogger::formatId(uint32_t id)
{
    if (id == NetConfig::HUB_ID) {
        return "HUB";
    }
    if (id == NetConfig::BROADCAST_ID) {
        return "BROADCAST";
    }

    return QString::number(id);
}

QString FlowLogger::msgTypeToString(const ProtocolMsgType msg_type)
{
    switch (msg_type) {
        case ProtocolMsgType::UserPlaneData:
            return "DATA: User Plane Payload";

        case ProtocolMsgType::Sib1:
            return "SIB1 (System Info Broadcast)";

        case ProtocolMsgType::RachPreamble:
            return "Msg1: RACH Preamble";

        case ProtocolMsgType::Rar:
            return "Msg2: Random Access Response";

        case ProtocolMsgType::RrcSetupRequest:
            return "Msg3: RRC Setup Request";

        case ProtocolMsgType::RrcSetup:
            return "Msg4: RRC Setup (Configuring SRB1)";

        case ProtocolMsgType::RrcSetupComplete:
            return "RRC Setup Complete";

        case ProtocolMsgType::RrcRelease:
            return "RRC Release (Go to Idle)";

        case ProtocolMsgType::RegistrationRequest:
            return "NAS: Registration Request";

        case ProtocolMsgType::RegistrationAccept:
            return "NAS: Registration Accept";

        case ProtocolMsgType::DeregistrationRequest:
            return "NAS: Deregistration (Detach)";

        case ProtocolMsgType::ServiceRequest:
            return "NAS: Service Request";

        case ProtocolMsgType::Paging:
            return "Paging (Search for UE)";

        case ProtocolMsgType::MeasurementReport:
            return "RRC: Measurement Report (RSRP)";

        case ProtocolMsgType::RrcReconfiguration:
            return "RRC: Reconfiguration (Handover)";

        case ProtocolMsgType::RrcReconfigurationComplete:
            return "RRC: Reconfig Complete (HO Done)";

        case ProtocolMsgType::Unknown:
        default:
            return "UNKNOWN_MSG_TYPE";
    }
}

EntityType FlowLogger::getOppositeType(EntityType senderType)
{
    return (senderType == EntityType::GNB) ? EntityType::UE : EntityType::GNB;
}
