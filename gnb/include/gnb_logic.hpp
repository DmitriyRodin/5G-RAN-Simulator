#ifndef GNB_LOGIC_HPP
#define GNB_LOGIC_HPP

#include <QTimer>

#include "base_entity.hpp"
#include "settings.hpp"
#include "types.hpp"

#ifdef UNIT_TESTS
class GnbLogicTestWrapper;
#endif

class GnbLogic : public BaseEntity
{
    Q_OBJECT
public:
    GnbLogic(const uint32_t id, const GnbSettings set,
             QObject* parent = nullptr);
    void setCellConfig(const GnbCellConfig& config);
    void run() override;
    uint32_t getConnectedUeCount() const;
    double getRadius() const;
    EntityType getType() const override;
    NodeInfo getNodeInfo() const override;

protected slots:
    void onTick();

protected:
    void onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type,
                                   const QByteArray& payload) override;

    void sendBroadcastInfo();
    void handleRegistrationRequest(uint32_t ue_id, const QByteArray& payload);

    QByteArray getRegistrationPayload() const override;

private:
    void handleRachPreamble(uint32_t ueId, const QByteArray& payload);
    void handleUeData(uint32_t sender_ue_id, const QByteArray& payload);
    void handleRrcSetupRequest(uint32_t ueId, const QByteArray& payload);
    void handleRrcSetupComplete(uint32_t ue_id, const QByteArray& payload);
    void handleMeasurementReport(uint32_t ue_id, const QByteArray& payload);

    void triggerHandover(uint32_t ue_id, uint32_t target_gnb_id);
    void sendRrcRelease(uint32_t ue_id, RrcReleaseCause cause);

    void updateUeContext(uint32_t ueId, uint16_t crnti);
    GnbData getData() const;

    QTimer* main_timer_ = nullptr;
    std::chrono::steady_clock::time_point last_broadcast_;
    const std::chrono::milliseconds broadcast_interval_{200};
    uint16_t next_crnti_counter_ = 1000;
    double radius_;

protected:
    QMap<uint32_t, UeContext> ue_contexts_;
    GnbCellConfig cellConfig_;

#ifdef UNIT_TESTS
    friend class GnbLogicTestWrapper;
#endif
};

#endif  // GNB_LOGIC_HPP
