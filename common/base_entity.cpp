#include <QDebug>
#include <QNetworkDatagram>

#include <common/base_entity.hpp>

BaseEntity::BaseEntity(int id, const EntityType& type, QObject* parrent)
    : id_(id)
    , type_(type)
{
    transport_ = new UdpTransport(this);
    connect(transport_, &UdpTransport::dataReceived, this, &BaseEntity::processIncoming);
}

BaseEntity::~BaseEntity() {
}

void BaseEntity::stop() {
}

EntityType BaseEntity::getType()
{
    return type_;
}

void BaseEntity::setup_network(){
}

void BaseEntity::sendSimData(const QByteArray& data,
                          const QHostAddress& receiver_ip,
                          quint16 receiver_port,
                          int receiver_id)
{
    sendingResult result = transport_->sendData(data, receiver_ip, receiver_port);

    if (result.is_socket_error_) {
        qWarning() << type_ << "#" << id_
                   << " failed to send UDP datagram to node #"
                   << receiver_id << ":" << result.toString();
    } else {
        qDebug() << type_ << "#" << id_
                 << "sent UDP datagram to node #" << receiver_id;
    }
}
