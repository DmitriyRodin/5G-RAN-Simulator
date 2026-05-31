#ifndef BASE_ENTITY_HPP
#define BASE_ENTITY_HPP

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPoint>
#include <QUdpSocket>

#include "iserializer.hpp"
#include "network_node.hpp"
#include "settings.hpp"
#include "types.hpp"
#include "udp_transport.hpp"

class BaseEntity : public QObject, public INetworkNode
{
    Q_OBJECT
public:
    BaseEntity(uint32_t id, const EntityType& type, HubSettings hub_set,
               QObject* parent = nullptr);
    virtual ~BaseEntity();

    void stop();
    virtual void run() = 0;
    bool setupNetwork(quint16 port);
    void registerAtHub();
    void handleRegistrationResponse(QDataStream& ds);

    uint32_t getId() const override;
    EntityType getType() const override;
    QPointF position() const override;
    void setPosition(QPointF pos) override;
    quint16 port() const override;
    void setPort(quint16 port) override;
    NodeInfo getNodeInfo() const override;

    void setTxPower(double power);
    double txPower() const;

signals:
    void registrationAtRadioHubConfirmed();

protected:
    virtual void sendSimData(ProtocolMsgType protoType,
                             const QByteArray& payload, uint32_t targetId);
    virtual QByteArray getRegistrationPayload() const;

    uint32_t id_;
    EntityType type_;
    QPointF position_;
    quint16 port_;

    UdpTransport* transport_ = nullptr;
    HubSettings hub_set_;
    bool is_registered_;

    double tx_power_dbm_;
    std::unique_ptr<ISerializer> serializer_;

    virtual void onProtocolMessageReceived(uint32_t source_id,
                                           ProtocolMsgType type,
                                           const QByteArray& payload) = 0;

public slots:
    void handleIncomingRawData(const QByteArray& data, const QHostAddress& addr,
                               quint16 port);
};

#endif  // BASE_ENTITY_HPP
