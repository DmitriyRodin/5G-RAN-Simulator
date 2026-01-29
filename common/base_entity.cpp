#include <QDebug>
#include <QNetworkDatagram>

#include <common/base_entity.hpp>

BaseEntity::BaseEntity(uint32_t id, const EntityType& type, QObject* parrent)
    : id_(id)
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
    }

    if (!transport_->init(port)) {
        qCritical() << QString("[Entity %1 # %2] Network setup failed on port %3")
                       .arg(typeToString(type_))
                       .arg(id_)
                       .arg(port);
        return false;
    }

    connect(transport_, &UdpTransport::dataReceived,
            this, &BaseEntity::handleIncomingRawData);

    qDebug() << QString("[Entity %1 # %2] Network is UP on port %3")
                .arg(typeToString(type_))
                .arg(id_)
                .arg(transport_->localPort());

    return true;
}


void BaseEntity::registerAtHub(const QHostAddress& hub_address, quint16 hub_port)
{
    hub_address_ = hub_address;
    hub_port_ = hub_port;

    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    const uint32_t sender_id = id_;
    ds << sender_id << NetConfig::HUB_ID << static_cast<uint8_t>(SimMessageType::Registration);

    transport_->sendData(packet, hub_address_, hub_port_);
}

void BaseEntity::handleRegistrationResponse(QDataStream &ds) {
    uint8_t status;
    ds >> status;

    if (status == 1) {
        is_registered_ = true;
        qDebug() << QString("[Entity %1] Registration SUCCESS at RadioHub").arg(id_);

        emit registrationConfirmed();
    } else {
        is_registered_ = false;
        qWarning() << QString("[Entity %1] Registration FAILED at RadioHub!").arg(id_);
    }
}

void BaseEntity::sendSimData(ProtocolMsgType proto_type,
                             const QByteArray& payload,
                             uint32_t target_id)
{
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << id_ << target_id << static_cast<uint8_t>(SimMessageType::Data);
    ds << static_cast<uint8_t>(proto_type) << payload;

    sendingResult result = transport_->sendData(packet, hub_address_, hub_port_);

    if (result.is_socket_error_) {
        qWarning() << type_ << "#" << id_
                   << " failed to send UDP datagram to node #"
                   << target_id << ":" << result.toString();
    } else {
        qDebug() << type_ << "#" << id_
                 << "sent UDP datagram to node #" << target_id;
    }
}

void BaseEntity::handleIncomingRawData(const QByteArray &data, const QHostAddress &addr, quint16 port)
{
    Q_UNUSED(addr);
    Q_UNUSED(port);

    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t source_id, target_id;
    uint8_t raw_sim_type;
    ds >> source_id >> target_id >> raw_sim_type;

    if (target_id != id_ && target_id != NetConfig::BROADCAST_ID) {
        return;
    }

    SimMessageType sim_type = static_cast<SimMessageType>(raw_sim_type);

    if (sim_type == SimMessageType::RegistrationResponse) {
        handleRegistrationResponse(ds);
        return;
    }

    if (sim_type == SimMessageType::Data) {
        uint8_t raw_proto_type;
        ds >> raw_proto_type;
        ProtocolMsgType proto_type = static_cast<ProtocolMsgType>(raw_proto_type);

        int headerSize = ds.device()->pos();
        QByteArray payload = data.mid(headerSize);

        onProtocolMessageReceived(source_id, proto_type, payload);
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
