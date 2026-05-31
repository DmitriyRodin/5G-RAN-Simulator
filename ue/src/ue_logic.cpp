#include "ue_logic.hpp"

#include <cmath>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "flow_logger.hpp"

UeLogic::UeLogic(const uint32_t id, const UeSettings set, QObject* parent)
    : BaseEntity(id, EntityType::UE, set.hub, parent)
    , state_(UeRrcState::DETACHED)
    , target_gnb_id_(0)
    , crnti_(0)
    , last_rach_ra_rnti_(0)
    , sent_msg3_identity_(0)
{
    qDebug() << "[UE #" << id_
             << "] Created. "
                "Initial State: DETACHED";
    timer_ = new QTimer(this);
    timer_->setInterval(set.radio.radio_frame_duration);

    connect(timer_, &QTimer::timeout, this, &UeLogic::onTick);
    last_report_time_ = std::chrono::steady_clock::now();
    connect(this, &BaseEntity::registrationAtRadioHubConfirmed, this,
            &UeLogic::onRegistrationConfirmed);
}

void UeLogic::run()
{
    if (timer_->isActive()) {
        return;
    }
    qDebug() << "UE #" << id_ << " started";
    timer_->start();
}

void UeLogic::resetSessionContext()
{
    is_connected_ = false;
    target_gnb_id_ = 0;
    crnti_ = 0;
    last_rach_ra_rnti_ = 0;
    sent_msg3_identity_ = 0;
    last_report_time_ = std::chrono::steady_clock::now();
}

bool UeLogic::checkPlmnValidity(const SIB1Info& sib1)
{
    for (const auto& plmn_identity : sib1.cell_config.plmns) {
        if (plmn_identity == plmn_) {
            return true;
        }
    }
    return false;
}

void UeLogic::onRegistrationConfirmed()
{
    qInfo() << QString(
                   "[UE %1] L1/L2: Registered in RadioHub (Power On Success)")
                   .arg(id_);

    searchingForCell();
}

void UeLogic::searchingForCell()
{
    resetSessionContext();
    state_ = UeRrcState::DETACHED;

    const int scan_delay = 2000;

    qDebug() << QString(
                    "[UE %1] Connection lost/released. Waiting %2ms for "
                    "frequency scan...")
                    .arg(id_)
                    .arg(scan_delay);

    QTimer::singleShot(scan_delay, this, [this]() {
        state_ = UeRrcState::SEARCHING_FOR_CELL;
        qDebug() << QString("[UE %1] Receiver active. Listening for SIB1...")
                        .arg(id_);
    });
}

void UeLogic::onProtocolMessageReceived(uint32_t gnb_id, ProtocolMsgType type,
                                        const QByteArray& payload)
{
    switch (type) {
        case ProtocolMsgType::Sib1:
            handleSib1(gnb_id, payload);
            break;

        case ProtocolMsgType::Rar:
            handleRar(gnb_id, payload);
            break;

        case ProtocolMsgType::RrcSetup:
            handleRrcSetup(gnb_id, payload);
            break;

        case ProtocolMsgType::RrcRelease:
            handleRrcRelease(gnb_id, payload);
            break;

        case ProtocolMsgType::RegistrationAccept:
            handleRegistrationAccept(payload);
            break;

        case ProtocolMsgType::RrcReconfiguration:
            handleRrcReconfiguration(payload);
            break;

        case ProtocolMsgType::UserPlaneData: {
            handleUserPlaneData(payload);
            break;
        }

        default:
            qDebug() << "[UE #" << id_ << "] Unknown protocol message from gNB"
                     << gnb_id << "Type:" << static_cast<int>(type);
            break;
    }
}

void UeLogic::handleSib1(uint32_t gnb_id, const QByteArray& payload)
{
    if (state_ != UeRrcState::SEARCHING_FOR_CELL) {
        return;
    }

    const auto sib1_info = serializer_->deserializeSB1Info(payload);

    qDebug() << "[UE #" << id_ << "] Found Cell! gNB #" << gnb_id;
    // Maybe: CHECK signal level (RSRP)
    if (!checkPlmnValidity(sib1_info)) {
        qDebug() << QString(
                        "[UE %1] This GNB %2 doesn support our mobile operator")
                        .arg(id_)
                        .arg(gnb_id);
        qDebug() << QString("[UE %1] PlmnIdentity: mmc: %2, mnc: %3")
                        .arg(id_)
                        .arg(plmn_.mcc)
                        .arg(plmn_.mnc);
        qDebug() << QString("[gNB %1] PlmnIdentity:").arg(gnb_id);
        for (const auto& [mcc, mnc] : sib1_info.cell_config.plmns) {
            qDebug() << QString("plms: mcc: %1, mnc^ %2").arg(mcc).arg(mnc);
        }
        return;
    }

    target_gnb_id_ = gnb_id;
    state_ = UeRrcState::RRC_IDLE;

    qDebug() << QString("[UE %1] Camped on Cell #%2. State: RRC_IDLE")
                    .arg(id_)
                    .arg(gnb_id);

    sendRachPreamble();
}

void UeLogic::sendRachPreamble()
{
    FlowLogger::log(type_, id_, target_gnb_id_, ProtocolMsgType::RachPreamble,
                    false);

    uint16_t ra_rnti = static_cast<uint16_t>(id_ % 65535);
    last_rach_ra_rnti_ = ra_rnti;

    state_ = UeRrcState::RRC_CONNECTING;

    QByteArray payload = serializer_->serializeRachPreamble(last_rach_ra_rnti_);

    sendSimData(ProtocolMsgType::RachPreamble, payload, target_gnb_id_);
}

void UeLogic::handleRar(uint32_t gnb_id, const QByteArray& payload)
{
    if (state_ != UeRrcState::RRC_CONNECTING) {
        qDebug() << "UeLogic::handleRar   =>>  state_ != "
                    "UeRrcState::RRC_CONNECTING -> RETURN";
        return;
    }

    auto rar_info = serializer_->deserializeRar(payload, last_rach_ra_rnti_);
    if (!rar_info.has_value()) {
        qDebug() << QString(
                        "[UE %1] RAR ignored: RA-RNTI "
                        "mismatch (Got: %2, Expected: %3)")
                        .arg(id_)
                        .arg(rar_info->ra_rnti)
                        .arg(last_rach_ra_rnti_);
        return;
    }

    crnti_ = rar_info->temp_c_rnti;

    FlowLogger::log(type_, id_, gnb_id, ProtocolMsgType::Rar, true);

    qDebug() << QString(
                    "[UE %1] <--- Msg2 (RAR) received. Assigned T-CRNTI: %2")
                    .arg(id_)
                    .arg(crnti_);

    sendRrcSetupRequest(gnb_id);
}

void UeLogic::sendRrcSetupRequest(uint32_t gnb_id)
{
    state_ = UeRrcState::RRC_CONNECTING;

    /* InitialUE-Identity ::= CHOICE {
     *     ng-5G-S-TMSI-Part1          BIT STRING (SIZE (39)),
     *     randomValue                 BIT STRING (SIZE (39))
     *}
     * 3GPP TS 38.331 - Radio Resource Control (RRC) protocol specification
     */
    const QByteArray request_data =
        serializer_->serializeRrcSetupRequest(RrcSetupRequest{
            sent_msg3_identity_,
            static_cast<uint8_t>(RrcEstablishmentCause::MO_SIGNALLING)});

    FlowLogger::log(type_, id_, gnb_id, ProtocolMsgType::RrcSetupRequest,
                    false);

    sendSimData(ProtocolMsgType::RrcSetupRequest, request_data, gnb_id);
}

void UeLogic::handleRrcSetup(uint32_t gnb_id, const QByteArray& payload)
{
    if (state_ != UeRrcState::RRC_CONNECTING) {
        qWarning() << "[UE] Ignored RrcSetup: Invalid State"
                   << toString(state_);
        return;
    }

    if (gnb_id != target_gnb_id_) {
        qWarning() << "[UE] Ignored RrcSetup: Wrong gNB ID" << gnb_id
                   << "(Expected:" << target_gnb_id_ << ")";
        return;
    }

    const RrcSetupInfo rrc_setup_info =
        serializer_->deserializeRrcSetup(payload);

    if (rrc_setup_info.received_identity !=
        static_cast<quint64>(sent_msg3_identity_)) {
        qWarning() << QString(
                          "[UE %1] Contention Resolution FAILED! Winner ID: "
                          "%2, My ID: %3")
                          .arg(id_)
                          .arg(rrc_setup_info.received_identity)
                          .arg(sent_msg3_identity_);

        state_ = UeRrcState::RRC_IDLE;
        crnti_ = 0;
        return;
    }

    state_ = UeRrcState::RRC_CONNECTED;
    is_connected_ = true;

    qDebug() << "[UE #" << id_ << "] Connected to gNB" << gnb_id
             << ". C-RNTI:" << crnti_;
    sendRrcSetupComplete(target_gnb_id_);
}

void UeLogic::sendRrcSetupComplete(uint32_t gnb_id)
{
    const QByteArray payload =
        serializer_->serializeRrcSetupComplete(RrcSetupCompleteInfo{plmn_});

    FlowLogger::log(type_, id_, gnb_id, ProtocolMsgType::RrcSetupComplete,
                    false);

    sendSimData(ProtocolMsgType::RrcSetupComplete, payload, gnb_id);
}

void UeLogic::handleRrcRelease(uint32_t gnb_id, const QByteArray& payload)
{
    if (gnb_id != target_gnb_id_) {
        qWarning() << "[UE #" << id_
                   << "] Received RRC Release from unknown gNB" << gnb_id;
        return;
    }

    const RrcReleaseCause cause = serializer_->deserializeRrcRelease(payload);

    qDebug() << QString("[UE %1] <--- RRC Release received. Cause: %2 (%3)")
                    .arg(id_)
                    .arg(static_cast<int>(cause))
                    .arg(toString(cause));

    resetSessionContext();

    state_ = UeRrcState::RRC_IDLE;

    FlowLogger::log(EntityType::GNB, id_, gnb_id, ProtocolMsgType::RrcRelease,
                    true);

    qDebug() << QString("[UE %1] Connection closed. Restarting lifecycle...")
                    .arg(id_);

    searchingForCell();
}

void UeLogic::sendRegistrationRequest()
{
    if (state_ != UeRrcState::RRC_CONNECTED) {
        return;
    }

    const QByteArray payload = serializer_->serializeRegistrationRequest(
        {id_, QString("UE-Capabilities-Model-X")});

    qDebug() << "[UE #" << id_ << "] Sending NAS Registration Request via gNB"
             << target_gnb_id_;
    sendSimData(ProtocolMsgType::RegistrationRequest, payload, target_gnb_id_);
}

void UeLogic::handleRegistrationAccept(const QByteArray& payload)
{
    const RegistrationAnswerInfo payload_info =
        serializer_->deserializeRegistrationAnswer(payload);

    if (payload_info.status == RegistrationStatus::Accepted) {
        state_ = UeRrcState::RRC_CONNECTED;
        is_connected_ = true;

        sendRrcSetupComplete(target_gnb_id_);

    } else if (payload_info.status == RegistrationStatus::Rejected) {
        qWarning() << QString(
                          "[UE %1] NAS: Connection REJECTED by gNB #%2. "
                          "Reason: \"%3\"")
                          .arg(id_)
                          .arg(target_gnb_id_)
                          .arg(payload_info.reject_reason.value_or(
                              "No reason given"));

        is_connected_ = false;

        searchingForCell();

    } else {
        qCritical() << QString(
                           "[UE %1] L3: Critical error. Received unknown "
                           "RegistrationStatus byte: %2")
                           .arg(id_)
                           .arg(static_cast<uint8_t>(payload_info.status));
        searchingForCell();
    }
}

void UeLogic::onTick()
{
    auto now = std::chrono::steady_clock::now();

    bool canSendReports = (state_ == UeRrcState::RRC_CONNECTED);

    if (canSendReports && (now - last_report_time_ >= report_interval_)) {
        sendMeasurementReport();
        last_report_time_ = now;
    }
}

void UeLogic::sendMeasurementReport()
{
    if (state_ != UeRrcState::RRC_CONNECTED) {
        return;
    }

    const double rsrp = -90.0 + QRandomGenerator::global()->bounded(10);
    const QByteArray report =
        serializer_->serializeMeasurementReport(rsrp, target_gnb_id_);

    qDebug() << "[UE #" << id_ << "] Sending Measurement Report. RSRP:" << rsrp
             << "dBm";
    sendSimData(ProtocolMsgType::MeasurementReport, report, target_gnb_id_);
}

void UeLogic::handleRrcReconfiguration(const QByteArray& payload)
{
    const auto info = serializer_->deserializeRrcReconfiguration(payload);
    if (!info.has_value()) {
        return;
    }
    const auto target_gnb_id = info.value().gnb_id;
    qDebug() << QString(
                    "[UE %1] <--- RRC Reconfiguration received! Switching from "
                    "gNB %2 to gNB %3")
                    .arg(id_)
                    .arg(target_gnb_id_)
                    .arg(target_gnb_id);

    state_ = UeRrcState::RRC_CONNECTING;

    target_gnb_id_ = target_gnb_id;

    qDebug() << QString("[UE %1] Initiating RACH on target gNB %2...")
                    .arg(id_)
                    .arg(target_gnb_id);

    last_report_time_ = std::chrono::steady_clock::now();

    sendRachPreamble();
}

void UeLogic::sendChatMessage(const ChatMessageInfo& info)
{
    if (state_ != UeRrcState::RRC_CONNECTED) {
        qWarning() << "[UE #" << id_ << "] Cannot send message: Not connected!";
        return;
    }

    const QByteArray data = serializer_->serializeChatMessage(info);
    qDebug() << "[UE #" << id_ << "] Sending text message to UE #"
             << info.receiver_ue_id << ":" << info.text;

    sendSimData(ProtocolMsgType::UserPlaneData, data, target_gnb_id_);
}

void UeLogic::handleUserPlaneData(const QByteArray& payload)
{
    ChatMessageInfo info = serializer_->deserializeChatMessage(payload);

    qDebug() << QString("[UE %1] [CHAT] From UE %2: %3")
                    .arg(id_)
                    .arg(info.sender_ue_id)
                    .arg(info.text);
}

bool UeLogic::isConnected() const
{
    return state_ == UeRrcState::RRC_CONNECTED;
}

QString UeLogic::stateString() const
{
    return toString(state_);
}

uint32_t UeLogic::getTargetGnb() const
{
    return target_gnb_id_;
}

UeData UeLogic::getData() const
{
    return UeData{is_connected_, toString(state_), target_gnb_id_};
}

NodeInfo UeLogic::getNodeInfo() const
{
    return {id_, type_, QHostAddress::LocalHost, port_, position_, getData()};
}
