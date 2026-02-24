#ifndef GNB_LOGIC_HPP
#define GNB_LOGIC_HPP

#include <QTimer>

#include "base_entity.hpp"

#ifdef UNIT_TESTS
class GnbLogicTestWrapper;
#endif

struct GnbCellConfig {
    uint16_t tac = 100;  // Tracking Area Code
    uint16_t mcc = 255;  // Mobile Country Code
    uint16_t mnc = 1;    // Mobile Network Code
    int16_t minRxLevel = -115;
    double txPowerDb = 43.0;
};

class GnbLogic : public BaseEntity
{
    Q_OBJECT
public:
    GnbLogic(uint32_t id, QObject* parent = nullptr);
    void setCellConfig(const GnbCellConfig& config);
    void run();

protected slots:
    void onTick();

protected:
    void onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type,
                                   const QByteArray& payload);

    void sendBroadcastInfo();
    void handleRegistrationRequest(uint32_t ue_id, const QByteArray& payload);

private:
    void handleRachPreamble(uint32_t ueId, const QByteArray& payload);
    void handleUeData(uint32_t sender_ue_id, const QByteArray& payload);
    void handleRrcSetupRequest(uint32_t ueId, const QByteArray& payload);
    void handleRrcSetupComplete(uint32_t ue_id, const QByteArray& payload);
    void handleMeasurementReport(uint32_t ue_id, const QByteArray& payload);

    void triggerHandover(uint32_t ue_id, uint32_t target_Gnb_id);
    void sendRrcRelease(uint32_t ue_id, RrcReleaseCause cause);

    void updateUeContext(uint32_t ueId, uint16_t crnti);

    QTimer* main_timer_ = nullptr;
    std::chrono::steady_clock::time_point last_broadcast_;
    const std::chrono::milliseconds broadcast_interval_{200};
    uint16_t next_crnti_counter_ = 1000;

protected:
    QMap<uint32_t, UeContext> ue_contexts_;
    GnbCellConfig cellConfig_;

#ifdef UNIT_TESTS
    friend class GnbLogicTestWrapper;
#endif
};

#endif  // GNB_LOGIC_HPP
