#ifndef TYPES_HPP
#define TYPES_HPP

#include <QDateTime>
#include <QHostAddress>

enum class EntityType {
    UE,
    GNB,
    UNKNOWN
};

struct UeContext {
    int id;
    double last_rssi;
    QDateTime last_activity;
    bool is_attached;
    QHostAddress ip_address;
    quint16 port;
};

enum class MessageType {
    Sib1,
    AttachRequest,
    AttachAccept,
    MeasurementReport,
    RRCReconfiguration,
    DataTransfer,
    Unknown
};

MessageType parseMessageType(const QJsonObject& obj);

#endif // TYPES_HPP
