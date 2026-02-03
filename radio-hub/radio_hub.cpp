#include <QDebug>
#include <QDataStream>

#include "radio_hub.hpp"

RadioHub::RadioHub(quint16 listen_port, QObject *parent)
    : QObject(parent)
{
    transport_ = new UdpTransport(this);

    if (transport_->init(listen_port)) {
        connect(transport_, &UdpTransport::dataReceived,
                this, &RadioHub::onDataReceived);
        qDebug() << "[RadioHub] Core started. Listening on port:"
                 << listen_port;
    }
}

void RadioHub::onDataReceived(const QByteArray &data,
                              const QHostAddress &sender_ip,
                              quint16 sender_port)
{
    const int MIN_HEADER_SIZE = 9;

    if (data.size() < MIN_HEADER_SIZE) {
        qWarning() << "RadioHub: Packet too small!" << data.size();
        return;
    }

    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    SimHeader header;
    uint8_t raw_type;
    ds >> header.source_id >> header.target_id >> raw_type;
    header.type = static_cast<SimMessageType>(raw_type);

    QByteArray payload = data.mid(sizeof(SimHeader));

    if (header.type == SimMessageType::Registration) {
        handleRegistration(header.source_id,
                           sender_ip,
                           sender_port);
    } else if (header.type == SimMessageType::Data) {
        handleDataRouting(header.source_id, header.target_id, payload);
    } else if (header.type == SimMessageType::Deregistration) {
        handleDeregistration(header.source_id);
    } else {
       qWarning() << QString("[RadioHub] Uknown SimMessageType from %1 received")
                       .arg(header.source_id);
    }
}

void RadioHub::handleRegistration(const uint32_t node_id,
                                  const QHostAddress &sender_ip,
                                  quint16 sender_port)
{
    uint8_t reg_status = HubResponse::REG_DENIED;

    if (node_id == NetConfig::HUB_ID || node_id == NetConfig::BROADCAST_ID) {
        qWarning() << "Registration REJECTED: Invalid Reserved ID";
        reg_status = HubResponse::REG_DENIED;
    } else if (nodes_.contains(node_id)) {
        qWarning() << "Registration stoped: the node with this ID already registred";
        reg_status = HubResponse::REG_DENIED;
    } else {
        nodes_[node_id] = {node_id, sender_ip, sender_port};
        qDebug() << QString("[RadioHub] Node %1 registered")
                    .arg(node_id);
        reg_status = HubResponse::REG_ACCEPTED;
    }

    QByteArray(response);
    QDataStream ds(&response, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << NetConfig::HUB_ID << node_id
       << static_cast<uint8_t>(SimMessageType::RegistrationResponse)
       << reg_status;

     transport_->sendData(response, sender_ip, sender_port);
}

void RadioHub::handleDeregistration(uint32_t src_id)
{
    if (nodes_.remove(src_id) > 0) {
        qDebug() << QString("[RadioHub] Node %1 successfully "
                            "deregistered and removed from the map.")
                    .arg(src_id);
    } else {
        qWarning() << QString("[RadioHub] Attempted to deregister "
                              "unknown Node %1.")
                      .arg(src_id);
    }
}

void RadioHub::handleDataRouting(uint32_t src_id,
                                 uint32_t target_id,
                                 const QByteArray &payload)
{
    if (!nodes_.contains(src_id)) {
        qWarning() << "Src_id doesn't registred";
        return;
    }

    const NodeInfo &src = nodes_[src_id];

    if (target_id == NetConfig::BROADCAST_ID) {
        for (const NodeInfo &dst : nodes_.values()) {
            if (dst.id != src_id) {
                deliverPacket(src, dst, payload);
            }
        }
    }
    else if (nodes_.contains(target_id)) {
        "RadioHub::handleDataRouting - > call deliverePacket()";
        deliverPacket(src, nodes_[target_id], payload);
    }
}

void RadioHub::deliverPacket(const NodeInfo &src,
                             const NodeInfo &dst,
                             const QByteArray &payload)
{
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    out << static_cast<uint32_t>(src.id) << static_cast<uint32_t>(dst.id)
        << static_cast<uint8_t>(SimMessageType::Data);

    packet.append(payload);

    sendingResult res = transport_->sendData(packet, dst.address, dst.port);
    if (res.is_socket_error_) {
        qWarning() << "[RadioHub] Failed to deliver packet to"
                   << dst.id << dst.address << dst.port << ":"
                   << res.toString();
    } else {
        qDebug() << "[RadioHub] Delivered packet to" << dst.id
                 << dst.address << dst.port << "bytes:" << res.bytes_;
    }
}

