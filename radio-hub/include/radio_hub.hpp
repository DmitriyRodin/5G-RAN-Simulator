#ifndef RADIOHUB_HPP
#define RADIOHUB_HPP

#include <QMap>
#include <QObject>

#include "sim_protocol.hpp"
#include "udp_transport.hpp"

struct GnbParameters {
    double radius;
};

struct NodeInfo {
    uint32_t id;
    EntityType type;
    QHostAddress address;
    quint16 port;
    QPointF position;

    std::optional<GnbParameters> gnbData;
};

/**
 * @brief The RadioHub class acts as a central orchestrator
 * for the 5G RAN simulation environment.
 * * It manages network entity registration (UEs and gNBs)
 * and performs packet routing via UDP.
 */
class RadioHub : public QObject
{
    Q_OBJECT

public:
    explicit RadioHub(quint16 listen_port, const uint32_t hub_id,
                      const uint32_t broadcast_id, const QPointF virt_hub_pos,
                      QObject* parent = nullptr);

private slots:
    void onDataReceived(const QByteArray& data, const QHostAddress& sender_ip,
                        quint16 sender_port);

private:
    void handleHubMessage(const SimProtocol::DecodedPacket& packet,
                          const QHostAddress& sender_ip, quint16 sender_port);
    void broadcastFromGbn(const QByteArray& raw_data, uint32_t src_id);
    void forwardToNode(const QByteArray& raw_data, const uint32_t dst_id,
                       const uint32_t src_id);

    void handleRegistration(const uint32_t node_id,
                            const QHostAddress& sender_ip, quint16 sender_port,
                            const EntityType type, const QPointF& coordinates,
                            const double& radius);
    const NodeInfo* findNode(uint32_t id) const;
    double calculateDistance(const QPointF& position_1,
                             const QPointF& position_2);
    void handleDeregistration(uint32_t src_id, EntityType type);
    void sendRegistrationResponse(uint32_t node_id, uint8_t status,
                                  const QHostAddress& ip, quint16 port);
    bool areWithinCoverageArea(const NodeInfo* first, const NodeInfo* second);
    void updatePosition(const uint32_t& id, const EntityType& type,
                        const QPointF& position);

    UdpTransport* transport_ = nullptr;
    QMap<uint32_t, NodeInfo> gnbs_;
    QMap<uint32_t, NodeInfo> ues_;

    const uint32_t hub_id_;
    const uint32_t broadcast_id_;
    const QPointF position_;
};

#endif  // RADIOHUB_HPP
