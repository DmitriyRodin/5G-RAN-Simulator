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

struct UeContext {
    int id;
    double last_rssi;
    QDateTime last_activity;
    bool is_attached;
    QHostAddress ip_address;
    quint16 port;
};

// MessageType from gNb to UE and from UE to gNB

enum class MessageType : uint8_t {
    // Broadcast
    Sib1 = 0,

    // Initial Access
    RachPreamble,
    Rar,

    //Mobility & Connection Management
    RegistrationRequest,
    RegistrationAccept,
    DeregistrationRequest,
    ServiceRequest,
    Paging,

    // Handover (Xn/N2 Mobility)
    MeasurementReport,
    RrcReconfiguration,
    RrcReconfigurationComplete,

    // User Plane
    UserPlaneData,

    Unknown = 255
};

MessageType parseMessageType(const QJsonObject& obj);

QDebug operator<<(QDebug stream, EntityType type);

// header and message type from gNb and UE to RaioHub ( UE and gNB connection simulation )

enum class SimMessageType : uint8_t {
    Registration = 0,
    Deregistration,
    Data,
    Unknown = 255
};

struct SimHeader {
    uint32_t source_id;
    uint32_t target_id; // 0xFFFFFFFF for Broadcast
    SimMessageType type;
};

#endif // TYPES_HPP
