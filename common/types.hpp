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

QString typeToString(EntityType type);

struct UeContext {
    int id;
    double last_rssi;
    QDateTime last_activity;
    bool is_attached;
    QHostAddress ip_address;
    quint16 port;
};

namespace NetConfig {
    const uint32_t HUB_ID = 0;
    const uint32_t BROADCAST_ID = 0xFFFFFFFF;
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

// ProtocolMsgType from gNb to UE and from UE to gNB

enum class ProtocolMsgType : uint8_t {
    // Broadcast
    Sib1 = 0,

    // RACH: Initial Access
    RachPreamble,
    Rar,

    RrcSetup,
    RrcSetupRequest,
    RrcSetupComplete,
    RrcRelease,

    //Mobility & Connection Management
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

QDebug operator<<(QDebug stream, EntityType type);

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
    uint32_t target_id; // NetConfig::BROADCAST_ID = 0xFFFFFFFF
    SimMessageType type;
};

#endif // TYPES_HPP
