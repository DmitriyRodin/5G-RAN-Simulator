#ifndef GNB_LOGIC_HPP
#define GNB_LOGIC_HPP

#include <QTimer>

#include <unordered_map>

#include "common/base_entity.hpp"

class GnbLogic : public BaseEntity {
    Q_OBJECT
public:
    GnbLogic(int id, QObject* parent = nullptr);
    void run();

private slots:
    void onTick();

private:
    void onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type, const QByteArray &payload);
    void sendBroadcastInfo();
    void handleUeData(uint32_t ue_id, const QByteArray& payload);
    void handleRegistrationRequest(uint32_t ue_id, const QByteArray& payload);
    void handleMeasurementReport(const QJsonObject &obj);
    void triggerHandover(int ue_id, int target_Gnb_id);

    QTimer* main_timer_ = nullptr;
    std::chrono::steady_clock::time_point last_broadcast_;
    const std::chrono::milliseconds broadcast_interval_{200};
    std::unordered_map<uint, UeContext> ue_contexts_;
};

#endif // GNB_LOGIC_HPP
