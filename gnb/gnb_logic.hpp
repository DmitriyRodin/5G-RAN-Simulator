#ifndef GNB_LOGIC_HPP
#define GNB_LOGIC_HPP

#include <QTimer>

#include <unordered_map>

#include "common/base_entity.hpp"

struct GnbCellConfig {
    uint16_t tac = 100;          // Tracking Area Code
    uint16_t mcc = 255;          // Mobile Country Code
    uint16_t mnc = 01;           // Mobile Network Code
    int16_t minRxLevel = -115;
    double txPowerDb = 43.0;
};

class GnbLogic : public BaseEntity {
    Q_OBJECT
public:
    GnbLogic(uint32_t id, QObject* parent = nullptr);
    void setCellConfig(const GnbCellConfig& config);
    void run();

private slots:
    void onTick();

private:
    void onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type, const QByteArray &payload);
    void sendBroadcastInfo();
    void handleUeData(uint32_t ue_id, const QByteArray& payload);
    void handleRrcSetupRequest(uint32_t ueId,
                               const QByteArray& payload,
                               const QHostAddress &senderIp,
                               quint16 senderPort);
    void handleRegistrationRequest(uint32_t ue_id, const QByteArray& payload);
    void handleMeasurementReport(const QJsonObject &obj);
    void triggerHandover(int ue_id, int target_Gnb_id);

    QTimer* main_timer_ = nullptr;
    std::chrono::steady_clock::time_point last_broadcast_;
    const std::chrono::milliseconds broadcast_interval_{200};
    std::unordered_map<uint, UeContext> ue_contexts_;

    GnbCellConfig cellConfig_;
};

#endif // GNB_LOGIC_HPP
