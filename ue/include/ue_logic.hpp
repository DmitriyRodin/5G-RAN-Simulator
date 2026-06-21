#ifndef UE_LOGIC_HPP
#define UE_LOGIC_HPP

#include <chrono>

#include <QHash>
#include <QTimer>

#include "base_entity.hpp"
#include "settings.hpp"

#ifdef UNIT_TESTS
class UeLogicTestWrapper;
#endif

class UeLogic : public BaseEntity
{
    Q_OBJECT
public:
    explicit UeLogic(const uint32_t id, const UeSettings set,
                     std::unique_ptr<ISerializer> serializer,
                     QObject* parent = nullptr);
    void run() override;
    void sendChatMessage(const ChatMessageInfo& info);
    bool isConnected() const;
    QString stateString() const;
    uint32_t getTargetGnb() const;
    NodeInfo getNodeInfo() const override;

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
    bool checkPlmnValidity(const SIB1Info& sib1);

    void handleUserPlaneData(const QByteArray& payload);
    UeData getData() const;

    PlmnIdentity plmn_;
    bool is_connected_ = false;
    bool is_rf_receiver_locked_;

    /**
     * @brief Current state of the L3 Control Plane protocol engine.
     * * Tracks high-level RRC states according to 3GPP TS 38.331. Dictates the
     * availability of signaling and data radio bearers (SRBs/DRBs). Functions
     * independently of transient L1 physical link spikes.
     * * @see UeRrcState
     * @see 3GPP TS 38.331 Section 4.2
     */
    UeRrcState state_;
    /**
     * @brief Current status of the L1/AS physical cell tracking engine.
     * * Manages low-level RF carrier sweeping, synchronization, and SIB1
     * evaluation according to 3GPP TS 38.304. Functions orthogonally to
     * #state_, allowing background cell reselection during link drops without
     * corrupting L3 protocol states.
     * * @see CellSearchStatus
     * @see 3GPP TS 38.304 Section 5.2
     */
    CellSearchStatus cell_status_;

    uint32_t target_gnb_id_;

    /**
     * @brief Cell Radio Network Temporary Identifier (C-RNTI).
     * * @see 3GPP TS 38.321 — MAC protocol specification: page 146
     */
    rnti_t crnti_;
    uint16_t last_rach_ra_rnti_;
    uint64_t sent_msg3_identity_;

    QTimer* timer_ = nullptr;
    std::chrono::steady_clock::time_point last_report_time_;
    const std::chrono::milliseconds report_interval_{500};

    QList<uint32_t> peers_;

#ifdef UNIT_TESTS
    friend class UeLogicTestWrapper;
#endif
};

#endif  // UE_LOGIC_HPP
