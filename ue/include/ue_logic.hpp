#ifndef UE_LOGIC_HPP
#define UE_LOGIC_HPP

#include <chrono>

#include <QHash>
#include <QTimer>

#include "base_entity.hpp"

class UeLogic : public BaseEntity
{
    Q_OBJECT
public:
    explicit UeLogic(uint32_t id, QObject* parent = nullptr);
    void run() override;
    void sendChatMessage(uint32_t target_ue_id, const QString& text);

protected:
    void onProtocolMessageReceived(uint32_t gnb_id, ProtocolMsgType type,
                                   const QByteArray& payload) override;
    void searchingForCell();

private slots:
    void onTick();
    void onRegistrationConfirmed();

private:
    void handleSib1(uint32_t gnb_id, const QByteArray& payload);
    void sendRachPreamble();
    void handleRar(uint32_t gnb_id, const QByteArray& payload);

    void handleRegistrationAccept(const QByteArray& payload);
    void handleRrcReconfiguration(const QByteArray& payload);

    void sendRrcSetupRequest(uint32_t gnb_id);
    void handleRrcRelease(uint32_t gnb_id, const QByteArray& payload);
    void handleRrcSetup(uint32_t gnb_id, const QByteArray& payload);
    void sendRrcSetupComplete(uint32_t gnb_id);

    void sendRegistrationRequest();
    void sendMeasurementReport();

    void resetSessionContext();

    void handleUserPlaneData(const QByteArray& payload);

    UeRrcState state_;
    uint32_t target_gnb_id_;
    /* Cell Radio Network Temporary Identifier (C-RNTI).
     * 3GPP TS 38.321 — MAC protocol specification: page 146
     */
    uint16_t crnti_;
    uint16_t last_rach_ra_rnti_;
    uint64_t sent_msg3_identity_;

    QTimer* timer_ = nullptr;
    std::chrono::steady_clock::time_point last_report_time_;
    const std::chrono::milliseconds report_interval_{500};

    QList<uint32_t> peers_;
};

#endif  // UE_LOGIC_HPP
