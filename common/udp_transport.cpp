#include <QNetworkDatagram>

#include "udp_transport.hpp"
#include "common/types.hpp"

bool sendingResult::sendingResult::ok() const { return is_socket_error_; }

QString sendingResult::sendingResult::toString() const {
    if (ok()) {
        return QString("Sent %1 bytes").arg(bytes_);
    }
    return QString("Error %1").arg(socket_error_);
}


UdpTransport::UdpTransport(QObject* parent)
    : QObject(parent)
{
}

sendingResult UdpTransport::sendData(const QByteArray& data,
                            const QHostAddress& receiver_ip,
                            quint16 receiver_port)
{
    sendingResult sending_result;

    sending_result.bytes_ = socket_->writeDatagram(
        data,
        receiver_ip,
        receiver_port
    );
    if (sending_result.bytes_ < 0) {
        sending_result.is_socket_error_ = true;
        sending_result.socket_error_ = socket_->errorString();
    }

    return sending_result;
}

bool UdpTransport::init(quint16 listen_port)
{
    if (!socket_) {
        socket_ = new QUdpSocket(this);
    }

    qRegisterMetaType<QHostAddress>("QHostAddress");

    if (socket_->bind(QHostAddress::Any, listen_port)) {
        connect(socket_, &QUdpSocket::readyRead, this, &UdpTransport::readPendingDatagrams);
        qDebug() << "UdpTransport: successfully listening on port" << socket_->localPort();
        return true;
    } else {
        qWarning() << "UdpTransport: failed to bind to port" << listen_port
                   << "Error:" << socket_->errorString();
        return false;
    }
}

void UdpTransport::readPendingDatagrams()
{
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        const QHostAddress sender_ip = datagram.senderAddress();
        const quint16 sender_port = datagram.senderPort();

        emit dataReceived(datagram.data(), sender_ip, sender_port );
    }
}

quint16 UdpTransport::localPort() const
{
    return socket_ ? socket_->localPort() : 0;
}

