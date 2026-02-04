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

void RadioHub::onDataReceived(const QByteArray &raw_data,
                              const QHostAddress &sender_ip,
                              quint16 sender_port)
{
    auto packet = SimProtocol::parse(raw_data);

    if (!packet.isValid) {
        return;
    }

    if (packet.isForHub()) {
        handleHubMessage(packet, sender_ip, sender_port);
        return;
    }

    if (packet.isBroadcast()) {
        broadcastToAll(raw_data, packet.srcId);
        return;
    }

    forwardToNode(raw_data, packet.dstId);
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
        qWarning() << "Registration stoped: the node with "
                      "this ID already registred";
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

void RadioHub::handleHubMessage(const SimProtocol::DecodedPacket &packet,
                                const QHostAddress &sender_ip,
                                quint16 sender_port)
{
    switch(packet.type) {
        case SimMessageType::Registration: {
            handleRegistration(packet.srcId, sender_ip, sender_port);
            break;
        }
        case SimMessageType::Deregistration: {
            handleDeregistration(packet.srcId);
            break;
        }
        default: {
            qWarning() << "[RadioHub] unknown SimMessageType";
        }
    }
}

void RadioHub::broadcastToAll(const QByteArray &raw_data,
                              uint32_t src_id)
{
    uint32_t sent_count = 0;
    for (const auto& node: nodes_) {
        if (node.id == src_id) {
            continue;
        }
        transport_->sendData(raw_data, node.address, node.port);
        ++sent_count;
    }
    qDebug() << QString("[RadioHub] Broadcast from ID:%1 | "
                        "Size:%2 bytes | Delivered to:%3 nodes")
                    .arg(src_id)
                    .arg(raw_data.size())
                    .arg(sent_count);
}

void RadioHub::forwardToNode(const QByteArray &raw_data, uint32_t dst_id)
{
    auto it = nodes_.find(dst_id);
    if (it != nodes_.end()) {
        transport_->sendData(raw_data, it.value().address, it.value().port);
    } else {
        qWarning() << "[RadioHub]: destination node doesn't registred";
    }
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
