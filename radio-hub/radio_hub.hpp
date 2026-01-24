#ifndef RADIOHUB_HPP
#define RADIOHUB_HPP

#include <QObject>
#include <QMap>
#include "common/types.hpp"
#include "common/udp_transport.hpp"

struct NodeInfo {
    uint32_t id;
    QHostAddress address;
    quint16 port;
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
    explicit RadioHub(quint16 listen_port = 5555,
                      QObject *parent = nullptr);

private slots:
    void onDataReceived(const QByteArray &data,
                        const QHostAddress &sender_ip,
                        quint16 sender_port);

private:
    void handleRegistration(const QByteArray &payload,
                            const QHostAddress &sender_ip,
                            quint16 sender_port);
    void handleDeregistration(uint32_t src_id);
    void handleDataRouting(uint32_t src_id,
                           uint32_t target_id,
                           const QByteArray &payload);
    void deliverPacket(const NodeInfo &src,
                       const NodeInfo &dst,
                       const QByteArray &payload);

    UdpTransport* transport_;
    QMap<uint32_t, NodeInfo> nodes_;
};

#endif // RADIOHUB_HPP
