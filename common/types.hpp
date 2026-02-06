#ifndef TYPES_HPP
#define TYPES_HPP

#include <QDateTime>
#include <QDebug>
#include <QHostAddress>

enum class EntityType {
    UE,
    GNB,
    UNKNOWN = 255
};

QDebug operator<<(QDebug stream, EntityType type);

QString typeToString(EntityType type);

enum class RrcEstablishmentCause : uint8_t {
    EMERGENCY = 0,
    HIGH_PRIORITY_ACCESS = 1,
    MT_ACCESS = 2,            // Mobile Terminated
    MO_SIGNALLING = 3,        // Mobile Originated Signalling
    MO_DATA = 4,
    MO_VOICE_CALL = 5,
    MO_VIDEO_CALL = 6,
    MO_SMS = 7,
    MPS_PRIORITY_ACCESS = 8,
    MCS_PRIORITY_ACCESS = 9
};

inline QString toString(RrcEstablishmentCause cause) {
    switch (cause) {
        case RrcEstablishmentCause::EMERGENCY:
            return "RrcEstablishmentCause::EMERGENCY";
        case RrcEstablishmentCause::HIGH_PRIORITY_ACCESS:
            return "RrcEstablishmentCause::HIGH_PRIORITY_ACCESS";
        case RrcEstablishmentCause::MO_SIGNALLING:
            return "mo-Signalling";
        case RrcEstablishmentCause::MO_DATA: return "mo-Data";

        default: return "unknown";
    }
}

enum class UeRrcState : uint8_t {
    // Simulation:
    DETACHED = 0,
    SEARCHING_FOR_CELL = 1,

    // 5G states (3GPP TS 38.331)
    RRC_IDLE = 2,
    RRC_CONNECTING = 3,
    RRC_CONNECTED = 4,
    RRC_INACTIVE = 5
};

QString toString(UeRrcState ue_rrc_state);

struct UeContext {
    uint32_t id;
    uint16_t crnti;
    uint32_t selected_plmn;


    UeRrcState state;
    RrcEstablishmentCause establishmentCause;
    bool is_attached;


    QHostAddress ip_address;
    quint16 port;

    double last_rssi;
    QDateTime last_activity;

    UeContext(uint32_t ue_id,
              uint16_t new_crnti,
              const QHostAddress& addr,
              quint16 ip_port)
        : id(ue_id)
        , crnti(new_crnti)
        , selected_plmn(0)
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , ip_address(addr)
        , port(ip_port)
        , last_rssi(0.0)
        , last_activity(QDateTime::currentDateTime())
    {}


    UeContext()
        : id(0)
        , crnti(0)
        , selected_plmn(0)
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , port(0)
        , last_rssi(0.0)
        , last_activity(QDateTime::currentDateTime())
    {}
};

namespace NetConfig {
    const uint32_t HUB_ID = 0;
    const uint32_t BROADCAST_ID = 0xFFFFFFFF;

    constexpr uint32_t GNB_ID_START = 1;
    constexpr uint32_t UE_ID_START  = 500;
}

namespace SimConfig {
    constexpr int RADIO_FRAME_DURATION_MS = 10;
}

// ProtocolMsgType from gNb to UE and from UE to gNB

enum class ProtocolMsgType : uint8_t {
    // Broadcast
    Sib1 = 0,

    // RACH: Initial Access
    RachPreamble,
    Rar,

    // RRC Connection
    RrcSetup,
    RrcSetupRequest,
    RrcSetupComplete,
    RrcRelease,

    // NAS: Mobility & Connection Management
    RegistrationRequest,
    RegistrationAccept,
    DeregistrationRequest,
    ServiceRequest,
    Paging,

    // Xn/N2 Mobility: Handover
    MeasurementReport,
    RrcReconfiguration,
    RrcReconfigurationComplete,

    // User Plane
    UserPlaneData,

    Unknown = 255
};

enum class RegistrationStatus : uint8_t {
    Rejected = 0,
    Accepted = 1,
    Pending = 2
};

QString toString(RegistrationStatus reg_status);

// header and message type from gNb and UE to RaioHub ( UE and gNB connection simulation )

enum class SimMessageType : uint8_t {
    Registration = 0,
    RegistrationResponse,
    Deregistration,
    Data,
    Unknown = 255
};

struct SimHeader {
    uint32_t source_id;
    uint32_t target_id;
    SimMessageType type;
};

namespace HubResponse {
    const uint8_t REG_DENIED   = 0;
    const uint8_t REG_ACCEPTED = 1;
}

namespace RrcConfig {
    enum class Status : uint8_t {
        Failure = 0,
        Success = 1,
        ReconfigurationWithSync = 2
    };
}

#endif // TYPES_HPP
