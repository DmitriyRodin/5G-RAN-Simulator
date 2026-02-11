#include "sim_protocol.hpp"

namespace SimProtocol {

QByteArray buildPacket(uint32_t src, uint32_t dst, SimMessageType type,
                       const QByteArray& payload)
{
    QByteArray packet;
    packet.reserve(9 + payload.size());

    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << src;
    ds << dst;
    ds << static_cast<uint8_t>(type);

    if (!payload.isEmpty()) {
        ds.writeRawData(payload.constData(), payload.size());
    }

    return packet;
}

DecodedPacket parse(const QByteArray& data)
{
    DecodedPacket result;

    const int MIN_HEADER_SIZE = 9;

    if (data.size() < MIN_HEADER_SIZE) {
        result.isValid = false;
        return result;
    }

    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    uint8_t rawType;

    ds >> result.srcId;
    ds >> result.dstId;
    ds >> rawType;
    result.type = static_cast<SimMessageType>(rawType);

    int headerSize = ds.device()->pos();
    result.payload = data.mid(headerSize);

    result.isValid = true;
    return result;
}

bool DecodedPacket::isForMe(uint32_t myId) const
{
    if (!isValid) {
        return false;
    }
    return (dstId == myId || dstId == NetConfig::BROADCAST_ID);
}

bool DecodedPacket::isBroadcast() const
{
    return isValid && (dstId == NetConfig::BROADCAST_ID);
}

bool DecodedPacket::isFromHub() const
{
    return isValid && (srcId == NetConfig::HUB_ID);
}

bool DecodedPacket::isForHub() const
{
    return isValid && (dstId == NetConfig::HUB_ID);
}

}  // namespace SimProtocol
