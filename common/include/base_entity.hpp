#ifndef BASE_ENTITY_HPP
#define BASE_ENTITY_HPP

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPoint>
#include <QUdpSocket>

#include "types.hpp"
#include "udp_transport.hpp"

class BaseEntity : public QObject
{
    Q_OBJECT
public:
    BaseEntity(uint32_t id, const EntityType& type, QObject* parent = nullptr);
    virtual ~BaseEntity();

    void stop();
    virtual void run() = 0;
    bool setupNetwork(quint16 port);
    void registerAtHub(const QHostAddress& hub_address, quint16 hub_port);
    void handleRegistrationResponse(QDataStream& ds);

    EntityType getType();
    quint16 localPort() const;

    void setPosition(QPointF pos);
    QPointF position() const;
    void setTxPower(double power);
    double txPower() const;

signals:
    void registrationAtRadioHubConfirmed();

protected:
    void sendSimData(ProtocolMsgType protoType, const QByteArray& payload,
                     uint32_t targetId);

    uint32_t id_;
    EntityType type_;

    UdpTransport* transport_ = nullptr;

    QHostAddress hub_address_;
    quint16 hub_port_;

    bool is_registered_;
    QPointF position_;
    double tx_power_dbm_;

    virtual void onProtocolMessageReceived(uint32_t source_id,
                                           ProtocolMsgType type,
                                           const QByteArray& payload) = 0;

public slots:
    void handleIncomingRawData(const QByteArray& data, const QHostAddress& addr,
                               quint16 port);
};

#endif  // BASE_ENTITY_HPP
