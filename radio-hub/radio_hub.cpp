#include <QDebug>
#include <QDataStream>

#include "radio_hub.hpp"

RadioHub::RadioHub(quint16 listen_port, QObject *parent)
    : QObject(parent)
{
    transport_ = new UdpTransport(this);

    if (transport_->init(listen_port)) {
        connect(transport_, &UdpTransport::dataReceived, this, &RadioHub::onDataReceived);
        qDebug() << "[RadioHub] Core started. Listening on port:" << listen_port;
    }
}

void RadioHub::onDataReceived(const QByteArray &data,
                              const QHostAddress &sender_ip,
                              quint16 sender_port)
{
    if (data.size() < static_cast<int>(sizeof(SimHeader))) {
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
        handleRegistration(payload, sender_ip, sender_port);
    } else if (header.type == SimMessageType::Data) {
        handleDataRouting(header.source_id, header.target_id, payload);
    } else if (header.type == SimMessageType::Deregistration) {
        handleDeregistration(header.source_id);
    } else {
       qWarning() << QString("[RadioHub] Uknown SimMessageType from %1 received")
                       .arg(header.source_id);
    }
}

void RadioHub::handleRegistration(const QByteArray &payload,
                                  const QHostAddress &sender_ip,
                                  quint16 sender_port)
{
    QDataStream ds(payload);
    uint32_t id;
    ds >> id;

    if (!ds.atEnd()) {
        qWarning() << "Warning: payload contains more data than just sender ID";
    }

    nodes_[id] = {id, sender_ip, sender_port};
    qDebug() << QString("[RadioHub] Node %1 registered")
                .arg(id);
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
        return;
    }

    const NodeInfo &src = nodes_[src_id];

    if (target_id == 0xFFFFFFFF) {
        for (const NodeInfo &dst : nodes_.values()) {
            if (dst.id != src_id) {
                deliverPacket(src, dst, payload);
            }
        }
    }
    else if (nodes_.contains(target_id)) {
        deliverPacket(src, nodes_[target_id], payload);
    }
}

void RadioHub::deliverPacket(const NodeInfo &src, const NodeInfo &dst, const QByteArray &payload)
{
    transport_->sendData(payload, dst.address, dst.port);
}
