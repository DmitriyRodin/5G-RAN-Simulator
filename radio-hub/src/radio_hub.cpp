#include "radio_hub.hpp"

#include <QDataStream>
#include <QDebug>
#include <QLine>

RadioHub::RadioHub(quint16 listen_port, QObject* parent)
    : QObject(parent)
    , transport_(nullptr)
{
    transport_ = new UdpTransport(this);

    if (transport_->init(listen_port)) {
        connect(transport_, &UdpTransport::dataReceived, this,
                &RadioHub::onDataReceived);
        qDebug() << "[RadioHub] Core started. Listening on port:"
                 << listen_port;
    }
}

void RadioHub::onDataReceived(const QByteArray& raw_data,
                              const QHostAddress& sender_ip,
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

    updatePosition(packet.srcId, packet.nodeType, packet.position);

    if (packet.isBroadcast()) {
        broadcastFromGbn(raw_data, packet.srcId);
        return;
    }

    forwardToNode(raw_data, packet.dstId, packet.srcId);
}

void RadioHub::handleRegistration(const uint32_t node_id,
                                  const QHostAddress& sender_ip,
                                  quint16 sender_port, const EntityType type,
                                  const QPointF& position)
{
    uint8_t reg_status = HubResponse::REG_DENIED;

    if (node_id == NetConfig::HUB_ID || node_id == NetConfig::BROADCAST_ID) {
        qWarning() << "Registration REJECTED: Invalid Reserved ID";
        reg_status = HubResponse::REG_DENIED;
    } else if (ues_.contains(node_id) || gnbs_.contains(node_id)) {
        qWarning() << "Registration stoped: the node with "
                      "this ID already registred";
        reg_status = HubResponse::REG_DENIED;
    } else {
        switch (type) {
            case EntityType::UE: {
                ues_[node_id] = {node_id, EntityType::UE, sender_ip,
                                 sender_port, position};
                qDebug() << QString("[RadioHub] UE %1 registered").arg(node_id);
                reg_status = HubResponse::REG_ACCEPTED;
                break;
            }
            case EntityType::GNB: {
                gnbs_[node_id] = {
                    node_id,
                    EntityType::GNB,
                    sender_ip,
                    sender_port,
                    position,
                    GnbParameters{NetConfig::GNB_DEFAULT_COVERAGE_RADIUS}};
                qDebug()
                    << QString("[RadioHub] GNB %1 registered").arg(node_id);
                reg_status = HubResponse::REG_ACCEPTED;
                break;
            }
            case EntityType::RadioHub: {
                qWarning() << QString(
                    "[RadioHub] Registration Error: Oops. Some other RadioHub "
                    "tries "
                    "to register, but we have only one RadioHub in our "
                    "architecture");
                break;
            }
            case EntityType::UNKNOWN: {
                qWarning() << QString(
                                  "[RadioHub] Registration REJECTED: Unknown "
                                  "EntityType for ID %1")
                                  .arg(node_id);
                reg_status = HubResponse::REG_DENIED;
                break;
            }
        }
    }

    sendRegistrationResponse(node_id, reg_status, sender_ip, sender_port);
}

void RadioHub::sendRegistrationResponse(uint32_t node_id, uint8_t status,
                                        const QHostAddress& ip, quint16 port)
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << status;

    QByteArray response =
        SimProtocol::buildPacket(NetConfig::HUB_ID, EntityType::RadioHub,
                                 node_id, SimMessageType::RegistrationResponse,
                                 NetConfig::HUB_VIRTUAL_POS, payload);

    transport_->sendData(response, ip, port);
}

void RadioHub::handleHubMessage(const SimProtocol::DecodedPacket& packet,
                                const QHostAddress& sender_ip,
                                quint16 sender_port)
{
    switch (packet.type) {
        case SimMessageType::Registration: {
            handleRegistration(packet.srcId, sender_ip, sender_port,
                               packet.nodeType, packet.position);
            break;
        }
        case SimMessageType::Deregistration: {
            handleDeregistration(packet.srcId, packet.nodeType);
            break;
        }
        default: {
            qWarning() << "[RadioHub] unknown SimMessageType";
        }
    }
}

void RadioHub::broadcastFromGbn(const QByteArray& raw_data, uint32_t src_id)
{
    const NodeInfo* gnb = findNode(src_id);
    if (!gnb || gnb->type != EntityType::GNB || !gnb->gnbData.has_value()) {
        return;
    }

    const double radius = gnb->gnbData->radius;
    QPointF gnbPos = gnb->position;

    for (auto it = ues_.begin(); it != ues_.end(); ++it) {
        const NodeInfo& ue = it.value();
        const double distance = calculateDistance(gnbPos, ue.position);

        if (distance <= radius) {
            transport_->sendData(raw_data, ue.address, ue.port);
        }
    }
}

void RadioHub::forwardToNode(const QByteArray& raw_data, const uint32_t dst_id,
                             const uint32_t src_id)
{
    const NodeInfo* target = findNode(dst_id);
    const NodeInfo* source = findNode(src_id);

    if (!source) {
        qWarning() << "[RadioHub] Forwarding FAILED: Source Node" << src_id
                   << "not registered";
        return;
    }
    if (!target) {
        qWarning() << "[RadioHub] Forwarding FAILED: Destination Node" << dst_id
                   << "not registered";
        return;
    }

    if (areWithinCoverageArea(source, target)) {
        transport_->sendData(raw_data, target->address, target->port);
        qDebug() << "[RadioHub] Packet delivered from" << src_id << "to"
                 << dst_id;
    } else {
        qDebug() << "[RadioHub] Packet LOST: Distance between " << src_id
                 << " and " << dst_id << " exceeds coverage";
    }
}

const NodeInfo* RadioHub::findNode(uint32_t id) const
{
    auto itUe = ues_.find(id);
    if (itUe != ues_.end()) {
        return &itUe.value();
    }

    auto itGnb = gnbs_.find(id);
    if (itGnb != gnbs_.end()) {
        return &itGnb.value();
    }

    return nullptr;
}

double RadioHub::calculateDistance(const QPointF& position_1,
                                   const QPointF& position_2)
{
    return QLineF(position_1, position_2).length();
}

bool RadioHub::areWithinCoverageArea(const NodeInfo* source,
                                     const NodeInfo* target)
{
    const double distance =
        calculateDistance(source->position, target->position);

    if (source->gnbData.has_value()) {
        return distance <= source->gnbData->radius;
    }

    if (target->gnbData.has_value()) {
        return distance <= target->gnbData->radius;
    }

    return false;
}

void RadioHub::updatePosition(const uint32_t& id, const EntityType& type,
                              const QPointF& position)
{
    if (type == EntityType::UE) {
        auto it = ues_.find(id);
        if (it != ues_.end()) {
            it.value().position = position;
        }
    } else if (type == EntityType::GNB) {
        auto it = gnbs_.find(id);
        if (it != gnbs_.end()) {
            it.value().position = position;
        }
    }
}

void RadioHub::handleDeregistration(uint32_t src_id, EntityType type)
{
    bool removed = false;
    QString typeStr;

    switch (type) {
        case EntityType::UE:
            removed = (ues_.remove(src_id) > 0);
            typeStr = "UE";
            break;

        case EntityType::GNB:
            removed = (gnbs_.remove(src_id) > 0);
            typeStr = "gNB";
            break;

        default:
            qWarning() << "[RadioHub] Deregistration FAILED: Unknown "
                          "EntityType for ID"
                       << src_id;
            return;
    }

    if (removed) {
        qDebug() << QString(
                        "[RadioHub] %1 %2 successfully deregistered and "
                        "removed.")
                        .arg(typeStr)
                        .arg(src_id);
    } else {
        qWarning() << QString(
                          "[RadioHub] Attempted to deregister unknown %1 "
                          "with ID %2.")
                          .arg(typeStr)
                          .arg(src_id);
    }
}
