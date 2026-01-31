#ifndef UDP_TRANSPORT_H
#define UDP_TRANSPORT_H

#include <QString>
#include <QUdpSocket>

struct sendingResult {
    qint64 bytes_;
    bool is_socket_error_ = false;
    QString socket_error_;
    bool ok() const;
    QString toString() const;
};

/**
 * @brief unique class for asynchronous UDP connection
 * Used by all nodes: UE, gNB
 */
class UdpTransport : public QObject
{
    Q_OBJECT
public:
    UdpTransport(QObject* parent = nullptr);
    sendingResult sendData(const QByteArray& data,
                  const QHostAddress& receiver_ip,
                  quint16 receiver_port);

    bool init(quint16 listen_port);
    quint16 localPort() const;

signals:
    void dataReceived(const QByteArray& data,
                      const QHostAddress& addr,
                      quint16 port);

private:
    void readPendingDatagrams();

    QUdpSocket* socket_= nullptr;
};

#endif // UDP_TRANSPORT_H
