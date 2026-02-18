#include "ue_logic.hpp"

#include <cmath>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "flow_logger.hpp"

UeLogic::UeLogic(uint32_t id, QObject* parent)
    : BaseEntity(id, EntityType::UE, parent)
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
    timer_->setInterval(SimConfig::RADIO_FRAME_DURATION_MS);

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
    target_gnb_id_ = 0;
    crnti_ = 0;
    last_rach_ra_rnti_ = 0;
    sent_msg3_identity_ = 0;
    last_report_time_ = std::chrono::steady_clock::now();
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

    qDebug() << "[UE #" << id_ << "] Found Cell! gNB #" << gnb_id;
    // Maybe: CHECK signal level (RSRP)
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

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << ra_rnti;

    sendSimData(ProtocolMsgType::RachPreamble, payload, target_gnb_id_);
}

void UeLogic::handleRar(uint32_t gnb_id, const QByteArray& payload)
{
    if (state_ != UeRrcState::RRC_CONNECTING) {
        qDebug() << "UeLogic::handleRar   =>>  state_ != "
                    "UeRrcState::RRC_CONNECTING -> RETURN";
        return;
    }

    QDataStream in(payload);
    in.setByteOrder(QDataStream::BigEndian);

    uint16_t rx_ra_rnti;
    uint16_t temp_c_rnti;
    uint16_t timing_advance;

    in >> rx_ra_rnti >> temp_c_rnti >> timing_advance;

    uint16_t expected_ra_rnti = static_cast<uint16_t>(id_ % 65535);
    if (rx_ra_rnti != last_rach_ra_rnti_) {
        qDebug() << QString(
                        "[UE %1] RAR ignored: RA-RNTI "
                        "mismatch (Got: %2, Expected: %3)")
                        .arg(id_)
                        .arg(rx_ra_rnti)
                        .arg(expected_ra_rnti);
        return;
    }

    crnti_ = temp_c_rnti;

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

    QByteArray requestData;
    QDataStream ds(&requestData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    /* InitialUE-Identity ::= CHOICE {
     *     ng-5G-S-TMSI-Part1          BIT STRING (SIZE (39)),
     *     randomValue                 BIT STRING (SIZE (39))
     *}
     * 3GPP TS 38.331 - Radio Resource Control (RRC) protocol specification
     */
    sent_msg3_identity_ = static_cast<uint64_t>(id_);
    ds << static_cast<quint64>(sent_msg3_identity_);

    ds << static_cast<uint8_t>(RrcEstablishmentCause::MO_SIGNALLING);

    FlowLogger::log(type_, id_, gnb_id, ProtocolMsgType::RrcSetupRequest,
                    false);

    sendSimData(ProtocolMsgType::RrcSetupRequest, requestData, gnb_id);
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

    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    quint64 received_identity;
    uint8_t config_status;

    ds >> received_identity;
    ds >> config_status;

    if (received_identity != static_cast<quint64>(sent_msg3_identity_)) {
        qWarning() << QString(
                          "[UE %1] Contention Resolution FAILED! Winner ID: "
                          "%2, My ID: %3")
                          .arg(id_)
                          .arg(received_identity)
                          .arg(sent_msg3_identity_);

        state_ = UeRrcState::RRC_IDLE;
        crnti_ = 0;
        return;
    }

    state_ = UeRrcState::RRC_CONNECTED;

    qDebug() << "[UE #" << id_ << "] Connected to gNB" << gnb_id
             << ". C-RNTI:" << crnti_;
    sendRrcSetupComplete(target_gnb_id_);
}

void UeLogic::sendRrcSetupComplete(uint32_t gnb_id)
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t selected_plmn = 1;
    ds << selected_plmn;

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

    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    uint8_t cause_raw;
    ds >> cause_raw;
    RrcReleaseCause cause = static_cast<RrcReleaseCause>(cause_raw);

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

    QByteArray nasData;
    QDataStream ds(&nasData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << QString("UE-Capabilities-Model-X") << static_cast<uint16_t>(id_);

    qDebug() << "[UE #" << id_ << "] Sending NAS Registration Request via gNB"
             << target_gnb_id_;
    sendSimData(ProtocolMsgType::RegistrationRequest, nasData, target_gnb_id_);
}

void UeLogic::handleRegistrationAccept(const QByteArray& payload)
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    QString message;
    ds >> message;

    qDebug() << "[UE #" << id_
             << "] NAS Registration Accepted! Network says:" << message;
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

    QByteArray report;
    QDataStream ds(&report, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    double rsrp = -90.0 + QRandomGenerator::global()->bounded(10);
    ds << target_gnb_id_ << rsrp;

    qDebug() << "[UE #" << id_ << "] Sending Measurement Report. RSRP:" << rsrp
             << "dBm";
    sendSimData(ProtocolMsgType::MeasurementReport, report, target_gnb_id_);
}

void UeLogic::handleRrcReconfiguration(const QByteArray& payload)
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t target_gnb_id;

    if (ds.atEnd()) {
        return;
    }
    ds >> target_gnb_id;

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

void UeLogic::sendChatMessage(uint32_t target_ue_id, const QString& text)
{
    if (state_ != UeRrcState::RRC_CONNECTED) {
        qWarning() << "[UE #" << id_ << "] Cannot send message: Not connected!";
        return;
    }

    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << target_ue_id << text;
    qDebug() << "[UE #" << id_ << "] Sending text message to UE #"
             << target_ue_id << ":" << text;

    sendSimData(ProtocolMsgType::UserPlaneData, data, target_gnb_id_);
}

void UeLogic::handleUserPlaneData(const QByteArray& payload)
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    uint32_t ue_sender;
    QString text;
    ds >> ue_sender >> text;

    qDebug() << QString("[UE %1] [CHAT] From UE %2: %3")
                    .arg(id_)
                    .arg(ue_sender)
                    .arg(text);
}
