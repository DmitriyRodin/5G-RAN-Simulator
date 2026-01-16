#include <QDebug>
#include <QNetworkDatagram>

#include <common/base_entity.hpp>

BaseEntity::BaseEntity(int id, const EntityType& type, QObject* parrent)
    : id_(id)
    , type_(type)
{
    socket_ = new QUdpSocket(this);
    socket_->bind(QHostAddress::Any, 0);
    connect(socket_, &QUdpSocket::readyRead, this, &BaseEntity::readPendingDatagrams);
}

BaseEntity::~BaseEntity() {
}

void BaseEntity::stop() {
}

void BaseEntity::setup_network(){
}

void BaseEntity::send_message(const std::string &payload) {
}

void BaseEntity::readPendingDatagrams() {
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        const QHostAddress sender_ip = datagram.senderAddress();
        const quint16 sender_port = datagram.senderPort();
        processIncoming(datagram.data(), sender_ip, sender_port );
    }
}
