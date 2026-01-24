#ifndef BASE_ENTITY_HPP
#define BASE_ENTITY_HPP

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUdpSocket>

#include <common/types.hpp>
#include <common/udp_transport.hpp>

class BaseEntity: public QObject {
    Q_OBJECT
public:
    BaseEntity(int id, const EntityType& type, QObject* parent = nullptr);
    virtual ~BaseEntity();

    void stop();
    virtual void run() = 0;
    EntityType getType();

protected:
    void setup_network();
    void sendSimData(const QByteArray& data,
                  const QHostAddress& receiver_ip,
                  quint16 receiver_port,
                  int ue_id);

    int id_;
    EntityType type_;
    UdpTransport* transport_;
    QHostAddress hub_address_;
    quint16 hub_port_;

private:
    virtual void processIncoming(const QByteArray& data,
                         const QHostAddress& sender_ip,
                         quint16 sender_port) = 0;
};

#endif // BASE_ENTITY_HPP
