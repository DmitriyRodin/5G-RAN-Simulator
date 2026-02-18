#ifndef SIMPROTOCOL_HPP
#define SIMPROTOCOL_HPP

#include <cstdint>

#include <QByteArray>
#include <QDataStream>

#include "types.hpp"

namespace SimProtocol {

struct DecodedPacket {
    uint32_t srcId;
    uint32_t dstId;
    SimMessageType type;
    QByteArray payload;
    bool isValid = false;

    bool isForMe(uint32_t myId) const;
    bool isForHub() const;
    bool isBroadcast() const;
    bool isFromHub() const;
};

QByteArray buildPacket(uint32_t src, uint32_t dst, SimMessageType type,
                       const QByteArray& payload = QByteArray());

DecodedPacket parse(const QByteArray& data);

}  // namespace SimProtocol

#endif  // SIMPROTOCOL_HPP
