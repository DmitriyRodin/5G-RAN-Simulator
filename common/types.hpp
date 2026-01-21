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

enum class MessageType {
    Sib1,
    AttachRequest,
    AttachAccept,
    MeasurementReport,
    RRCReconfiguration,
    DataTransfer,
    Unknown = 255
};

QDebug operator<<(QDebug stream, EntityType type);

MessageType parseMessageType(const QJsonObject& obj);

#endif // TYPES_HPP
