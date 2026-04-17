#include "sim_protocol.hpp"

namespace SimProtocol {

const size_t MIN_HEADER_SIZE = 10;

QByteArray buildPacket(uint32_t src, EntityType nodeType, uint32_t dst,
                       SimMessageType type, const QPointF& position,
                       const QByteArray& payload)
{
    QByteArray packet;
    packet.reserve(MIN_HEADER_SIZE + payload.size());

    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << src;
    ds << nodeType;
    ds << dst;
    ds << static_cast<uint8_t>(type);
    ds << position.x();
    ds << position.y();

    if (!payload.isEmpty()) {
        ds.writeRawData(payload.constData(), payload.size());
    }

    return packet;
}

DecodedPacket parse(const QByteArray& data)
{
    DecodedPacket result;

    if (data.size() < MIN_HEADER_SIZE) {
        result.isValid = false;
        return result;
    }

    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    uint8_t rawType;
    ds >> result.srcId;
    ds >> result.nodeType;
    ds >> result.dstId;
    ds >> rawType;
    result.type = static_cast<SimMessageType>(rawType);
    double position_x{};
    ds >> position_x;
    double position_y{};
    ds >> position_y;
    result.position = QPointF(position_x, position_y);

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
