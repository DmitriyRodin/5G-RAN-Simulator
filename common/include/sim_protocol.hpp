#ifndef SIMPROTOCOL_HPP
#define SIMPROTOCOL_HPP

#include <cstdint>

#include <QByteArray>
#include <QDataStream>
#include <QPoint>

#include "types.hpp"

namespace SimProtocol {

struct DecodedPacket {
    uint32_t srcId;
    uint32_t dstId;
    SimMessageType type;
    EntityType nodeType;
    QPointF position;
    QByteArray payload;
    bool isValid = false;

    bool isForMe(uint32_t myId) const;
    bool isForHub() const;
    bool isBroadcast() const;
    bool isFromHub() const;
};

QByteArray buildPacket(uint32_t src, EntityType entity_type, uint32_t dst,
                       SimMessageType type, const QPointF& position,
                       const QByteArray& payload = QByteArray());

DecodedPacket parse(const QByteArray& data);
}  // namespace SimProtocol

#endif  // SIMPROTOCOL_HPP
