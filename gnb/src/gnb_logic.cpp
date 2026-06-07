#include "gnb_logic.hpp"

#include <QDebug>
#include <QRandomGenerator>

#include "flow_logger.hpp"

GnbLogic::GnbLogic(const uint32_t id, const GnbSettings set, QObject* parent)
    : BaseEntity(id, EntityType::GNB, set.hub, parent)
    , radius_(set.radius)
{
    main_timer_ = new QTimer(this);
    main_timer_->setInterval(set.radio.radio_frame_duration);
    connect(main_timer_, &QTimer::timeout, this, &GnbLogic::onTick);
    last_broadcast_ = std::chrono::steady_clock::now();
    connect(this, &BaseEntity::registrationAtRadioHubConfirmed, this,
            &GnbLogic::sendBroadcastInfo);
}

void GnbLogic::setCellConfig(const GnbCellConfig& config)
{
    cellConfig_ = config;
}

void GnbLogic::run()
{
    qDebug() << "GNB #" << id_ << " timer starts";
    main_timer_->start();
}

uint32_t GnbLogic::getConnectedUeCount() const
{
    return static_cast<uint32_t>(ue_contexts_.size());
}

double GnbLogic::getRadius() const
{
    return radius_;
}

EntityType GnbLogic::getType() const
{
    return type_;
}

NodeInfo GnbLogic::getNodeInfo() const
{
    return {id_, type_, QHostAddress::LocalHost, port_, position_, getData()};
}

void GnbLogic::onTick()
{
    auto now = std::chrono::steady_clock::now();

    if (now - last_broadcast_ >= broadcast_interval_) {
        sendBroadcastInfo();
        last_broadcast_ = now;
    }

    const std::chrono::seconds inactivity_timeout(30);

    QMutableMapIterator<uint32_t, UeContext> it(ue_contexts_);
    while (it.hasNext()) {
        it.next();
        UeContext& ctx = it.value();

        if (it.value().state == UeRrcState::RRC_CONNECTED) {
            auto duration_idle =
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - ctx.last_activity);

            if (duration_idle > inactivity_timeout) {
                qDebug() << "[gNB] Inactivity timeout for UE" << it.key();
                sendRrcRelease(it.key(), RrcReleaseCause::UserInactivity);

                ctx.state = UeRrcState::RRC_IDLE;
                ctx.is_attached = false;
            }
        }
    }
}

void GnbLogic::onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type,
                                         const QByteArray& payload)
{
    if (ue_contexts_.contains(ue_id)) {
        ue_contexts_[ue_id].last_activity = std::chrono::steady_clock::now();
    }

    switch (type) {
        case ProtocolMsgType::RrcSetupRequest:
            handleRrcSetupRequest(ue_id, payload);
            break;
        case ProtocolMsgType::MeasurementReport:
            handleMeasurementReport(ue_id, payload);
            break;
        case ProtocolMsgType::UserPlaneData:
            handleUeData(ue_id, payload);
            break;
        case ProtocolMsgType::Sib1:
            // ignore
            break;
        case ProtocolMsgType::RachPreamble:
            handleRachPreamble(ue_id, payload);
            break;
        case ProtocolMsgType::RrcSetupComplete:
            handleRrcSetupComplete(ue_id, payload);
            break;
        case ProtocolMsgType::RegistrationRequest:
            handleRegistrationRequest(ue_id, payload);
        default:
            qDebug() << "[gNB] Unhandled protocol type:"
                     << static_cast<uint8_t>(type);
    }
}

void GnbLogic::sendBroadcastInfo()
{
    const QByteArray broadcast_info =
        serializer_->serializeSB1Info({id_, cellConfig_});

    sendSimData(ProtocolMsgType::Sib1, broadcast_info, hub_set_.broadcast_id);
    FlowLogger::log(type_, id_, hub_set_.broadcast_id, ProtocolMsgType::Sib1,
                    false);
}

void GnbLogic::handleRachPreamble(uint32_t ue_id, const QByteArray& payload)
{
    const auto rach_opt = serializer_->deserializeRachPreamble(payload);

    if (!rach_opt.has_value()) {
        qWarning()
            << "[GNB #" << id_
            << "] RACH_PREAMBLE parsing failed (corrupted packet). Dropping.";
        return;
    }
    const RachPreambleInfo rach = rach_opt.value();

    uint16_t temp_c_rnti =
        static_cast<uint16_t>(next_crnti_counter_ + (ue_id % 9000));

    qDebug() << QString(
                    "[gNB %1] <--- Msg1 (RACH Preamble) received from UE %2. "
                    "ra_rnti = %3. tempCrnti = %4")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(rach.ra_rnti)
                    .arg(temp_c_rnti);

    updateUeContext(ue_id, temp_c_rnti);

    const ta_index_t AVERAGE_TIMING_ANVANCE = 10;
    const RarInfo rar_info = {rach.ra_rnti, temp_c_rnti,
                              AVERAGE_TIMING_ANVANCE};
    const QByteArray rar_payload = serializer_->serializeRar(rar_info);
    qDebug()
        << QString(
               "[gNB %1] ---> Msg2 (RAR) sent to UE %2. Assigned T-CRNTI: %3")
               .arg(id_)
               .arg(ue_id)
               .arg(temp_c_rnti);

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::Rar, false);
    sendSimData(ProtocolMsgType::Rar, rar_payload, ue_id);
}

void GnbLogic::updateUeContext(uint32_t ue_id, uint16_t crnti)
{
    auto now = std::chrono::steady_clock::now();
    if (!ue_contexts_.contains(ue_id)) {
        UeContext ctx;
        ctx.id = ue_id;
        ctx.crnti = crnti;
        ctx.last_activity = now;
        ctx.is_attached = false;
        ue_contexts_[ue_id] = ctx;
    } else {
        ue_contexts_[ue_id].crnti = crnti;
        ue_contexts_[ue_id].last_activity = now;
    }
}

GnbData GnbLogic::getData() const
{
    return {radius_, getConnectedUeCount()};
}

void GnbLogic::handleUeData(uint32_t sender_ue_id, const QByteArray& payload)
{
    if (!ue_contexts_.contains(sender_ue_id)) {
        qWarning() << "[gNB] Data from unknown UE:" << sender_ue_id;
        return;
    }

    const auto info_opt = serializer_->deserializeChatMessage(payload);

    if (!info_opt.has_value()) {
        qWarning() << "[GNB #" << id_
                   << "] UE_DATA parsing failed (corrupted packet). Dropping.";
        return;
    }
    const ChatMessageInfo info = info_opt.value();

    if (sender_ue_id != info.sender_ue_id) {
        qWarning() << QString(
                          "[GNB %1] UserPlane: SOURCE MISMATCH ERROR! "
                          "Network Header Sender ID (%2) does not match "
                          "Application Payload Sender ID (%3). "
                          "Target Receiver ID: %4. Dropping packet.")
                          .arg(id_)
                          .arg(sender_ue_id)
                          .arg(info.sender_ue_id)
                          .arg(info.receiver_ue_id);
        return;
    }

    if (!ue_contexts_.contains(info.receiver_ue_id)) {
        qWarning() << QString(
                          "[gNB] UE %1 tries to message offline/unknown UE %2")
                          .arg(sender_ue_id)
                          .arg(info.receiver_ue_id);
        return;
    }

    if (ue_contexts_[info.receiver_ue_id].state != UeRrcState::RRC_CONNECTED) {
        qWarning() << "[gNB] Target UE" << info.receiver_ue_id
                   << "is not in CONNECTED state";
        return;
    }

    qDebug() << QString("[gNB] Relay Chat: UE %1 -> UE %2 | Text: %3")
                    .arg(sender_ue_id)
                    .arg(info.receiver_ue_id)
                    .arg(info.text);

    sendSimData(ProtocolMsgType::UserPlaneData, payload, info.receiver_ue_id);
}

void GnbLogic::handleRegistrationRequest(uint32_t ue_id,
                                         const QByteArray& payload)
{
    const auto info_opt = serializer_->deserializeRegistrationRequest(payload);
    if (!info_opt.has_value()) {
        qWarning() << "[GNB #" << id_
                   << "] RegistrationRequest parsing failed (corrupted "
                      "packet). Dropping.";
        return;
    }

    const RegistrationRequestInfo info = info_opt.value();

    if (info.ue_id != ue_id) {
        qWarning() << QString(
                          "[gNb %1] Received RRC Connection Request from UE %2 "
                          "with wrong ue_id %3 in packet. Ignore reqquest")
                          .arg(id_)
                          .arg(ue_id)
                          .arg(info.ue_id);
        return;
    }

    qDebug() << QString(
                    "[gNB %1] Received RRC Connection Request from UE %2 and "
                    "UE capabilieties %3")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(info.ue_cap);

    bool isAccepted = true;  // Assumption: gNB has enough resources

    RegistrationAnswerInfo response;
    RegistrationStatus status;
    if (isAccepted) {
        status = RegistrationStatus::Accepted;

        qDebug() << "  -> Registration ACCEPTED. Assigned C-RNTI: 42";
    } else {
        status = RegistrationStatus::Rejected;
        response.reject_reason = QString("hasn't enough resources");
    }
    response.status = status;
    const QByteArray response_data =
        serializer_->serializeRegistrationAnswer(response);

    sendSimData(ProtocolMsgType::RrcSetup, response_data, ue_id);
}

QByteArray GnbLogic::getRegistrationPayload() const
{
    const QByteArray payload =
        serializer_->serializeRegistrationPayload(radius_);

    return payload;
}

void GnbLogic::handleMeasurementReport(uint32_t ue_id,
                                       const QByteArray& payload)
{
    if (!ue_contexts_.contains(ue_id)) {
        qWarning() << "[gNB] Measurement Report from unknown UE:" << ue_id;
        return;
    }

    UeContext& ctx = ue_contexts_[ue_id];

    const auto info_opt = serializer_->deserializeMeasurementReport(payload);
    if (!info_opt.has_value()) {
        qWarning() << "[GNB #" << id_
                   << "] MEASUREMENT_REPORT parsing failed (corrupted packet). "
                      "Dropping.";
        return;
    }

    const MeasurementReportInfo info = info_opt.value();

    ctx.last_activity = std::chrono::steady_clock::now();

    qDebug() << QString(
                    "[gNB %1] <--- Measurement Report from UE %2. Cell: %3, "
                    "RSRP: %4 dBm")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(info.reported_gnb_id)
                    .arg(info.rsrp);

    const double handover_hysteresis = 3.0;

    if (info.reported_gnb_id == this->id_) {
        ctx.last_rssi = info.rsrp;
        qDebug() << QString("[gNB %1] Serving cell update for UE %2: %3 dBm")
                        .arg(id_)
                        .arg(ue_id)
                        .arg(info.rsrp);
    } else {
        qDebug()
            << QString("[gNB %1] Neighbor report from UE %2: Cell %3 is %4 dBm")
                   .arg(id_)
                   .arg(ue_id)
                   .arg(info.reported_gnb_id)
                   .arg(info.rsrp);

        if (info.rsrp > (ctx.last_rssi + handover_hysteresis)) {
            qDebug() << QString(
                            "[gNB %1] !!! CRITICAL: Triggering Handover for UE "
                            "%2 to Cell %3")
                            .arg(id_)
                            .arg(ue_id)
                            .arg(info.reported_gnb_id);

            triggerHandover(ue_id, info.reported_gnb_id);
        }
    }

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::MeasurementReport,
                    true);
}

void GnbLogic::triggerHandover(uint32_t ue_id, uint32_t target_gnb_id)
{
    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcReconfiguration,
                    false);

    const QByteArray payload =
        serializer_->serializeTriggerHandover(HandoverInfo{target_gnb_id});

    sendSimData(ProtocolMsgType::RrcReconfiguration, payload, ue_id);
}

void GnbLogic::handleRrcSetupRequest(uint32_t ue_id, const QByteArray& payload)
{
    if (!ue_contexts_.contains(ue_id)) {
        qWarning() << QString(
                          "[gNB %1] Security Alert: Msg3 received "
                          "from unknown UE ID: %2. Ignoring.")
                          .arg(id_)
                          .arg(ue_id);
        return;
    }

    UeContext& ctx = ue_contexts_[ue_id];

    const auto info_opt = serializer_->deserializeRrcSetupRequest(payload);
    if (!info_opt.has_value()) {
        qWarning() << "[GNB #" << id_
                   << "] RRC_SETUP_REQUEST parsing failed (corrupted packet). "
                      "Dropping.";
        return;
    }

    const RrcSetupRequest info = info_opt.value();

    uint16_t assigned_crnti = ctx.crnti;

    qDebug() << QString(
                    "[gNB %1] <--- Msg3 (RRC Setup Request) from UE %2 "
                    "(C-RNTI %3). Payload Identity: %4, establishmentCause: %5")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(assigned_crnti)
                    .arg(info.ue_identity)
                    .arg(info.cause);

    // Assumption: gNB has enough resources
    const QByteArray msg4_payload = serializer_->serializeRrcSetup(
        {info.ue_identity, static_cast<uint8_t>(RrcConfig::Status::Success)});

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcSetup, false);

    sendSimData(ProtocolMsgType::RrcSetup, msg4_payload, ue_id);
}

void GnbLogic::handleRrcSetupComplete(uint32_t ue_id, const QByteArray& payload)
{
    if (!ue_contexts_.contains(ue_id)) {
        qWarning() << "[gNB] Msg5 received from unknown UE:" << ue_id;
        return;
    }

    const auto info_opt = serializer_->deserializeRrcSetupComplete(payload);
    if (!info_opt.has_value()) {
        qWarning() << "[GNB #" << id_
                   << "] RRC_SETIP_COMPLETE parsing failed (corrupted packet). "
                      "Dropping.";
        return;
    }

    const RrcSetupCompleteInfo info = info_opt.value();

    UeContext& ctx = ue_contexts_[ue_id];

    ctx.state = UeRrcState::RRC_CONNECTED;
    ctx.is_attached = true;
    ctx.selected_plmn = info.plmn;
    ctx.last_activity = std::chrono::steady_clock::now();

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcSetupComplete, true);

    qDebug() << QString(
                    "[gNB %1] <--- Msg5 Received. UE %2 is now FULLY "
                    "CONNECTED (C-RNTI %3). PLMN: mcc %4, mnc: %5")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(ctx.crnti)
                    .arg(info.plmn.mcc)
                    .arg(info.plmn.mnc);
}

void GnbLogic::sendRrcRelease(uint32_t ue_id, RrcReleaseCause cause)
{
    if (!ue_contexts_.contains(ue_id)) return;

    const QByteArray payload = serializer_->serializeRrcRelease(cause);

    qDebug() << QString("[gNB %1] ---> RRC Release to UE %2. Cause: %3")
                    .arg(id_)
                    .arg(ue_id)
                    .arg(static_cast<int>(cause));

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcRelease, false);
    sendSimData(ProtocolMsgType::RrcRelease, payload, ue_id);
}
