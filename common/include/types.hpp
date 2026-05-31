#ifndef TYPES_HPP
#define TYPES_HPP

#include <QDateTime>
#include <QDebug>
#include <QHostAddress>
#include <QPoint>

enum class EntityType : uint8_t {
    UE,
    GNB,
    RadioHub,
    UNKNOWN = 255
};

QDebug operator<<(QDebug stream, EntityType type);

QString typeToString(EntityType type);

enum class RrcEstablishmentCause : uint8_t {
    EMERGENCY = 0,
    HIGH_PRIORITY_ACCESS = 1,
    MT_ACCESS = 2,      // Mobile Terminated
    MO_SIGNALLING = 3,  // Mobile Originated Signalling
    MO_DATA = 4,
    MO_VOICE_CALL = 5,
    MO_VIDEO_CALL = 6,
    MO_SMS = 7,
    MPS_PRIORITY_ACCESS = 8,
    MCS_PRIORITY_ACCESS = 9
};

inline QString toString(RrcEstablishmentCause cause)
{
    switch (cause) {
        case RrcEstablishmentCause::EMERGENCY:
            return "RrcEstablishmentCause::EMERGENCY";
        case RrcEstablishmentCause::HIGH_PRIORITY_ACCESS:
            return "RrcEstablishmentCause::HIGH_PRIORITY_ACCESS";
        case RrcEstablishmentCause::MO_SIGNALLING:
            return "mo-Signalling";
        case RrcEstablishmentCause::MO_DATA:
            return "mo-Data";

        default:
            return "unknown";
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

using rnti_t = uint16_t;
using ta_index_t = uint16_t;

struct RarInfo {
    rnti_t ra_rnti;
    rnti_t temp_c_rnti;
    ta_index_t timing_advance;
};

struct RrcSetupRequest {
    uint64_t ue_identity;
    uint8_t cause;
};

struct RrcSetupInfo {
    quint64 received_identity;
    uint8_t config_status;
};

struct PlmnIdentity {
    uint32_t mcc = 255;  // Mobile Country Code
    uint32_t mnc = 1;    // Mobile Network Code

    bool operator==(const PlmnIdentity& other) const
    {
        return (mcc == other.mcc) && (mnc == other.mnc);
    }
};

struct RrcSetupCompleteInfo {
    PlmnIdentity plmn;
};

struct RegistrationRequestInfo {
    uint32_t ue_id;
    QString ue_cap;
};

struct RegistrationAnswerInfo {
    RegistrationStatus status;
    std::optional<QString> reject_reason = std::nullopt;
};

struct RrcReconfigurationInfo {
    uint32_t gnb_id;
};

struct ChatMessageInfo {
    uint32_t receiver_ue_id;
    uint32_t sender_ue_id;
    QString text;
};

struct GnbCellConfig {
    uint16_t tac = 100;  // Tracking Area Code
    std::vector<PlmnIdentity> plmns;
    uint8_t plmns_size = 0;
    int16_t minRxLevel = -115;
    double txPowerDb = 43.0;
    GnbCellConfig(std::vector<PlmnIdentity> plmns_ident, const uint8_t size)
        : plmns(plmns_ident)
        , plmns_size(size)
    {
        for (const auto& [mcc, mnc] : plmns) {
            qDebug() << "mcc: " << mcc << ", mnc: " << mnc;
        }
    }
    GnbCellConfig()
    {
    }
};

struct SIB1Info {
    uint32_t gnb_id;
    GnbCellConfig cell_config;
};

struct RachPreambleInfo {
    rnti_t ra_rnti;
};

QDebug operator<<(QDebug stream, ChatMessageInfo chat_info);

struct UeContext {
    uint32_t id;
    rnti_t crnti;
    PlmnIdentity selected_plmn;

    UeRrcState state;
    RrcEstablishmentCause establishmentCause;
    bool is_attached;

    QHostAddress ip_address;
    quint16 port;

    double last_rssi;
    QDateTime last_activity;

    UeContext(uint32_t ue_id, rnti_t new_crnti, const QHostAddress& addr,
              quint16 ip_port)
        : id(ue_id)
        , crnti(new_crnti)
        , selected_plmn(PlmnIdentity{})
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , ip_address(addr)
        , port(ip_port)
        , last_rssi(0.0)
        , last_activity(QDateTime::currentDateTime())
    {
    }

    UeContext()
        : id(0)
        , crnti(0)
        , selected_plmn(PlmnIdentity{})
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , port(0)
        , last_rssi(0.0)
        , last_activity(QDateTime::currentDateTime())
    {
    }
};

// header and message type from gNb and UE to RaioHub(UE and gNB connection
// simulation)

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
const uint8_t REG_DENIED = 0;
const uint8_t REG_ACCEPTED = 1;
}  // namespace HubResponse

namespace RrcConfig {
enum class Status : uint8_t {
    Failure = 0,
    Success = 1,
    ReconfigurationWithSync = 2
};
}  // namespace RrcConfig

enum class RrcReleaseCause : uint8_t {
    Other = 0,
    UserInactivity = 1,
    RrcConnectionFailure = 2,
    LoadBalancing = 3,
    CellReselection = 4,
    VoiceFallback = 5
};

inline QString toString(RrcReleaseCause cause)
{
    switch (cause) {
        case RrcReleaseCause::Other:
            return "Other";
        case RrcReleaseCause::UserInactivity:
            return "UserInactivity";
        case RrcReleaseCause::RrcConnectionFailure:
            return "RrcConnectionFailure";
        case RrcReleaseCause::LoadBalancing:
            return "LoadBalancing";
        case RrcReleaseCause::CellReselection:
            return "CellReselection";
        case RrcReleaseCause::VoiceFallback:
            return "VoiceFallback";
        default:
            return "Unknown";
    }
}

namespace NetworkParam {
inline constexpr quint16 EPHEMERAL_PORT = 0;
};

#endif  // TYPES_HPP
