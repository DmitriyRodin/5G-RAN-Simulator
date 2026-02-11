#include "base_entity.hpp"

#include <QDebug>
#include <QNetworkDatagram>

#include "common/sim_protocol.hpp"

BaseEntity::BaseEntity(uint32_t id, const EntityType& type, QObject* parent)
    : QObject(parent)
    , id_(id)
    , type_(type)
    , is_registered_(false)
{
}

BaseEntity::~BaseEntity()
{
}

void BaseEntity::stop()
{
}

EntityType BaseEntity::getType()
{
    return type_;
}

quint16 BaseEntity::localPort() const
{
    return transport_->localPort();
}

bool BaseEntity::setupNetwork(quint16 port)
{
    if (!transport_) {
        transport_ = new UdpTransport(this);
        transport_->setObjectName(
            QString("transport-%1-%2").arg(typeToString(type_)).arg(id_));
    }

    if (!transport_->init(port)) {
        qCritical() << QString(
                           "[Entity %1 # %2] Network setup failed on port %3")
                           .arg(typeToString(type_))
                           .arg(id_)
                           .arg(port);
        return false;
    }

    auto connection =
        connect(transport_, &UdpTransport::dataReceived, this,
                &BaseEntity::handleIncomingRawData, Qt::DirectConnection);

    if (!connection) {
        qCritical() << "FATAL ERROR: Failed to connect UdpTransport signal!";
    } else {
        qDebug() << "Connection established successfully";
    }

    qDebug() << QString("[Entity %1 # %2] Network is UP on port %3")
                    .arg(typeToString(type_))
                    .arg(id_)
                    .arg(transport_->localPort());

    return true;
}

void BaseEntity::registerAtHub(const QHostAddress& hub_address,
                               quint16 hub_port)
{
    hub_address_ = hub_address;
    hub_port_ = hub_port;

    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    const uint32_t sender_id = id_;
    ds << sender_id << NetConfig::HUB_ID
       << static_cast<uint8_t>(SimMessageType::Registration);

    transport_->sendData(packet, hub_address_, hub_port_);
}

void BaseEntity::handleRegistrationResponse(QDataStream& ds)
{
    uint8_t status;
    ds >> status;

    if (status == 1) {
        is_registered_ = true;
        qDebug()
            << QString("[Entity %1] Registration SUCCESS at RadioHub").arg(id_);

        emit registrationAtRadioHubConfirmed();
    } else {
        is_registered_ = false;
        qWarning()
            << QString("[Entity %1] Registration FAILED at RadioHub!").arg(id_);
    }
}

void BaseEntity::sendSimData(ProtocolMsgType proto_type,
                             const QByteArray& payload, uint32_t target_id)
{
    QByteArray protocolPayload;

    protocolPayload.append(static_cast<char>(proto_type));
    protocolPayload.append(payload);

    QByteArray finalPacket = SimProtocol::buildPacket(
        id_, target_id, SimMessageType::Data, protocolPayload);

    sendingResult result =
        transport_->sendData(finalPacket, hub_address_, hub_port_);

    if (result.is_socket_error_) {
        qWarning() << QString("[%1 #%2] Send Error to %3: %4")
                          .arg(typeToString(type_))
                          .arg(id_)
                          .arg(target_id)
                          .arg(result.toString());
    }
}

void BaseEntity::handleIncomingRawData(const QByteArray& data,
                                       const QHostAddress& addr, quint16 port)
{
    Q_UNUSED(addr);
    Q_UNUSED(port);

    auto decoded = SimProtocol::parse(data);

    if (!decoded.isValid) {
        return;
    }

    if (!decoded.isForMe(id_) && !decoded.isBroadcast()) {
        return;
    }

    switch (decoded.type) {
        case SimMessageType::RegistrationResponse: {
            QDataStream ds(decoded.payload);
            ds.setByteOrder(QDataStream::BigEndian);
            handleRegistrationResponse(ds);
            break;
        }

        case SimMessageType::Data: {
            if (decoded.payload.isEmpty()) return;

            QDataStream ds(decoded.payload);
            ds.setByteOrder(QDataStream::BigEndian);

            uint8_t raw_proto_type;
            ds >> raw_proto_type;
            ProtocolMsgType proto_type =
                static_cast<ProtocolMsgType>(raw_proto_type);

            QByteArray actualPayload = decoded.payload.mid(sizeof(uint8_t));

            onProtocolMessageReceived(decoded.srcId, proto_type, actualPayload);
            break;
        }

        default:
            qWarning() << QString(
                              "[Entity %1] Received unknown SimMessageType: %2")
                              .arg(id_)
                              .arg(static_cast<int>(decoded.type));
            break;
    }
}

void BaseEntity::setPosition(QPointF pos)
{
    position_ = pos;
}

QPointF BaseEntity::position() const
{
    return position_;
}
void BaseEntity::setTxPower(double power)
{
    tx_power_dbm_ = power;
}

double BaseEntity::txPower() const
{
    return tx_power_dbm_;
}
