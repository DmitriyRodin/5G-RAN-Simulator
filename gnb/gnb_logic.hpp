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
    void processIncoming(const QByteArray& data, const QHostAddress& sender_ip, quint16 sender_port);
    void sendBroadcastInfo();
    void handleUeData(const QJsonObject& obj);
    void handleAttachRequest(const QJsonObject& obj);
    void handleMeasurementReport(const QJsonObject& obj);
    void triggerHandover(int ue_id, int target_Gnb_id);
    void send_message(int ue_id, QJsonObject &payload);

    QTimer* main_timer_;
    std::chrono::steady_clock::time_point last_broadcast_;
    const std::chrono::milliseconds broadcast_interval_{200};
    std::unordered_map<uint, UeContext> ue_contexts_;
};

#endif // GNB_LOGIC_HPP
