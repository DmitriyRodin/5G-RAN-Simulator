#include <QDebug>
#include <QRandomGenerator>

#include "gnb_logic.hpp"
#include "common/logging/flow_logger.hpp"

GnbLogic::GnbLogic(uint32_t id, QObject* parent )
    : BaseEntity(id, EntityType::GNB, parent)
{
    main_timer_ = new QTimer(this);
    main_timer_->setInterval(SimConfig::RADIO_FRAME_DURATION_MS);
    connect(main_timer_, &QTimer::timeout, this, &GnbLogic::onTick);
    last_broadcast_ = std::chrono::steady_clock::now();
    connect(this, &BaseEntity::registrationAtRadioHubConfirmed,
            this,&GnbLogic::sendBroadcastInfo);
}

void GnbLogic::setCellConfig(const GnbCellConfig& config)
{
    cellConfig_ = config;
}

void GnbLogic::run() {
    qDebug() << "GNB #" << id_ << " timer starts";
    main_timer_->start();
}

void GnbLogic::onTick() {
    auto now = std::chrono::steady_clock::now();

    if (now - last_broadcast_ >= broadcast_interval_) {
        sendBroadcastInfo();
        last_broadcast_ = now;
    }
}

void GnbLogic::onProtocolMessageReceived(uint32_t ue_id,
                                         ProtocolMsgType type,
                                         const QByteArray &payload)
{
    // UPDATE UeContext data

    switch (type) {
        case ProtocolMsgType::RrcSetupRequest:
            handleRrcSetupRequest(ue_id, payload);
            break;

        case ProtocolMsgType::MeasurementReport:
            // Handle Measurement Report
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
        default:
            qDebug() << "[gNB] Unhandled protocol type:"
                     << static_cast<int>(type);
    }
}

void GnbLogic::sendBroadcastInfo()
{
    QByteArray broadcast_info;
    QDataStream ds(&broadcast_info, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<uint32_t>(id_);
    ds << static_cast<uint16_t>(cellConfig_.tac);
    ds << static_cast<int16_t>(cellConfig_.minRxLevel);
    ds << static_cast<int16_t>(cellConfig_.mcc);
    ds << static_cast<int16_t>(cellConfig_.mnc);

    sendSimData(ProtocolMsgType::Sib1, broadcast_info, NetConfig::BROADCAST_ID);
    FlowLogger::log(type_, id_, NetConfig::BROADCAST_ID,
                    ProtocolMsgType::Sib1, false);
}

void GnbLogic::handleRachPreamble(uint32_t ueId, const QByteArray &payload)
{
    qDebug() << QString("[gNB %1] <--- Msg1 (RACH Preamble) "
                        "received from UE %2.").arg(id_).arg(ueId);

    QDataStream in(payload);
    in.setByteOrder(QDataStream::BigEndian);
    uint16_t ra_rnti;
    in >> ra_rnti;

    uint16_t tempCrnti = static_cast<uint16_t>(next_crnti_counter_ + (ueId % 9000));


    qDebug() << QString("[gNB %1] <--- Msg1 (RACH Preamble) received from UE %2. ra_rnti = %3. tempCrnti = %4").arg(id_).arg(ueId).arg(ra_rnti).arg(tempCrnti);

    updateUeContext(ueId, tempCrnti);

    QByteArray rarPayload;
    QDataStream out(&rarPayload, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    out << ra_rnti;
    out << tempCrnti;

    uint16_t timingAdvance = 0;
    out << timingAdvance;

    qDebug() << QString("[gNB %1] ---> Msg2 (RAR) sent to UE %2. Assigned T-CRNTI: %3")
                .arg(id_).arg(ueId).arg(tempCrnti);

    FlowLogger::log(type_, id_, ueId, ProtocolMsgType::Rar, false);
    sendSimData(ProtocolMsgType::Rar, rarPayload, ueId);
}

void GnbLogic::updateUeContext(uint32_t ueId, uint16_t crnti)
{
    if (!ue_contexts_.contains(ueId)) {
        UeContext ctx;
        ctx.id = ueId;
        ctx.crnti = crnti;
        ctx.last_activity = QDateTime::currentDateTime();
        ctx.is_attached = false;
        ue_contexts_[ueId] = ctx;
    } else {
        ue_contexts_[ueId].crnti = crnti;
        ue_contexts_[ueId].last_activity = QDateTime::currentDateTime();
    }
}

void GnbLogic::handleUeData(uint32_t ue_id, const QByteArray& payload)
{
    // HANDLE UE data logic
}

void GnbLogic::handleRegistrationRequest(uint32_t ue_id, const QByteArray& payload)
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    qDebug() << QString("[gNB %1] Received RRC Connection Request from UE %2")
                .arg(id_).arg(ue_id);

    bool isAccepted = true; // Assumption: the base station has enough resources

    QByteArray response_data;
    QDataStream out(&response_data, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    if (isAccepted) {
        out << static_cast<uint8_t>(RegistrationStatus::Accepted);
        const uint16_t new_crnti = 42;
        out << new_crnti;

        qDebug() << "  -> Registration ACCEPTED. Assigned C-RNTI: 42";
    } else {
        out << static_cast<uint8_t>(RegistrationStatus::Rejected);
    }

    sendSimData(ProtocolMsgType::RrcSetup, response_data, ue_id);
}

void GnbLogic::handleMeasurementReport(const QJsonObject &obj)
{
    int ue_id = obj["ue_id"].toInt();
    QJsonArray measurements = obj["measurements"].toArray();

    qDebug() << "GNB #" << id_ << " RRC info: Received Measurement Report from UE #"
             << ue_id;

    int best_target_Gnb_id = -1;
    double best_rssi = -1000.0;
    double serving_rssi = -1000.0;

    for (int i = 0; i < measurements.size(); ++i) {
        QJsonObject m = measurements[i].toObject();
        int target_id = m["gnb_id"].toInt();
        double rssi = m["rssi"].toDouble();

        if (target_id == id_) {
            serving_rssi = rssi;
        } else if (rssi > best_rssi) {
            best_rssi = rssi;
            best_target_Gnb_id = target_id;
        }
    }

    const double handover_hysteresis = 3.0;

    if (best_target_Gnb_id != -1 && (best_rssi > (serving_rssi + handover_hysteresis))) {
        qDebug() << "GNB #" << id_ << " has CRITICAL situation: Triggering Handover for UE #" << ue_id
                 << "to GNB #" << best_target_Gnb_id
                 << "(Serving:" << serving_rssi << "dBm, Target: " << best_rssi << "dBm)";

        triggerHandover(ue_id, best_target_Gnb_id);
    }
}

void GnbLogic::triggerHandover(int ue_id, int target_Gnb_id) {
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<uint32_t>(target_Gnb_id);
    sendSimData(ProtocolMsgType::RrcReconfiguration, payload, ue_id);
}

void GnbLogic::handleRrcSetupRequest(uint32_t ue_id,
                                     const QByteArray& payload)
{
    if (!ue_contexts_.contains(ue_id)) {
        qWarning() << QString("[gNB %1] Security Alert: Msg3 received "
                              "from unknown UE ID: %2. Ignoring.")
                      .arg(id_).arg(ue_id);
        return;
    }

    UeContext& ctx = ue_contexts_[ue_id];

    QDataStream in(payload);
    in.setByteOrder(QDataStream::BigEndian);

    quint64 received_identity = 0;)
    uint8_t establishmentCause;
    in >> received_identity >> establishmentCause;

    uint16_t assigned_crnti = ctx.crnti;

    qDebug() << QString("[gNB %1] <--- Msg3 (RRC Setup Request) from UE %2 (C-RNTI %3). Payload Identity: %4, establishmentCause: %5")
                .arg(id_).arg(ue_id).arg(assigned_crnti).arg(received_identity).arg(establishmentCause);

    QByteArray msg4Payload;
    QDataStream out(&msg4Payload, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << received_identity;

    // Assumption: gNB has enough resources
    out << static_cast<uint8_t>(RrcConfig::Status::Success);

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcSetup, false);

    sendSimData(ProtocolMsgType::RrcSetup, msg4Payload, ue_id);
}

void GnbLogic::handleRrcSetupComplete(uint32_t ue_id, const QByteArray &payload)
{
    if (!ue_contexts_.contains(ue_id)) {
        qWarning() << "[gNB] Msg5 received from unknown UE:" << ue_id;
        return;
    }

    QDataStream in(payload);
    in.setByteOrder(QDataStream::BigEndian);

    uint32_t selected_plmn;
    in >> selected_plmn;

    UeContext& ctx = ue_contexts_[ue_id];

    ctx.state = UeRrcState::RRC_CONNECTED;
    ctx.is_attached = true;
    ctx.selected_plmn = selected_plmn;

    FlowLogger::log(type_, id_, ue_id, ProtocolMsgType::RrcSetupComplete, true);

    qDebug() << QString("[gNB %1] <--- Msg5 Received. UE %2 is now FULLY CONNECTED (C-RNTI %3). PLMN: %4")
                .arg(id_).arg(ue_id).arg(ctx.crnti).arg(selected_plmn);
}
