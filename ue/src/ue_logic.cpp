#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

#include <cmath>

#include "ue_logic.hpp"

UeLogic::UeLogic(uint32_t id, QObject *parent)
    : BaseEntity(id, EntityType::UE, parent)
    , state_(UeRrcState::DETACHED)
    , target_gnb_id_(0)
    , crnti_(0)
{
    qDebug() << "[UE #" << id_ << "] Created. Initial State: DETACHED";
    timer_ = new QTimer(this);
    timer_->setInterval(SimConfig::RADIO_FRAME_DURATION_MS);

    connect(timer_, &QTimer::timeout, this, &UeLogic::onTick);
    last_report_time_ = std::chrono::steady_clock::now();
    connect(this, &BaseEntity::registrationAtRadioHubConfirmed,
            this, &UeLogic::searchingForCell);
}

void UeLogic::run() {
    qDebug() << "UE #" << id_ << " started";
    timer_->start();
}

void UeLogic::searchingForCell() {
    state_ = UeRrcState::SEARCHING_FOR_CELL;

    target_gnb_id_ = 0;
    qDebug() << "[UE #" << id_ << "] Registered in Hub. State: SEARCHING_FOR_CELL";
}

void UeLogic::onProtocolMessageReceived(uint32_t gnb_id,
                                        ProtocolMsgType type,
                                        const QByteArray &payload)
{
    switch (type) {
        case ProtocolMsgType::Sib1:
        qDebug() << "UE#" << id_ << "ProtocolMsgType::Sib1";
            handleSib1(gnb_id, payload);
            break;

        case ProtocolMsgType::RrcSetup:
            handleRrcSetup(gnb_id, payload);
            break;

        case ProtocolMsgType::RrcRelease:
            handleRrcRelease(gnb_id);
            break;

        case ProtocolMsgType::RegistrationAccept:
            handleRegistrationAccept(payload);
            break;

        case ProtocolMsgType::RrcReconfiguration:
            handleRrcReconfiguration(payload);
            break;

        case ProtocolMsgType::UserPlaneData: {
            QDataStream ds(payload);
            uint32_t ue_sender;
            QString text;
            ds >> ue_sender >> text;
            qDebug() << "[UE #" << id_ << "] received from UE #" << ue_sender << " a message: " << text;
            break;
        }

        default:
            qDebug() << "[UE #" << id_ << "] Unknown protocol message from gNB" << gnb_id
                     << "Type:" << static_cast<int>(type);
            break;
    }
}

void UeLogic::handleSib1(uint32_t gnb_id, const QByteArray &payload) {
    if (state_ != UeRrcState::SEARCHING_FOR_CELL) {
        return;
    }

    qDebug() << "[UE #" << id_ << "] Found Cell! gNB #" << gnb_id;

    // Maybe: CHECK signal level (RSRP)
    state_ = UeRrcState::RRC_IDLE;
    target_gnb_id_ = gnb_id;

    sendRrcSetupRequest(target_gnb_id_);
}

void UeLogic::sendRrcSetupRequest(uint32_t gnb_id) {
    state_ = UeRrcState::RRC_CONNECTING;

    QByteArray requestData;
    QDataStream ds(&requestData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<uint8_t>(0x01); // Cause: mo-Signalling

    sendSimData(ProtocolMsgType::RrcSetupRequest, requestData, gnb_id);
}

void UeLogic::handleRrcSetup(uint32_t gnb_id, const QByteArray &payload) {
    if (state_ != UeRrcState::RRC_CONNECTING || gnb_id != target_gnb_id_) {
        return;
    }

    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    ds >> crnti_;

    state_ = UeRrcState::RRC_CONNECTED;
    qDebug() << "[UE #" << id_ << "] Connected to gNB"
             << gnb_id << ". C-RNTI:" << crnti_;

    sendSimData(ProtocolMsgType::RrcSetupComplete, QByteArray(), gnb_id);
}

void UeLogic::handleRrcRelease(uint32_t gnb_id) {
    if (gnb_id != target_gnb_id_) {
        return;
    }

    qDebug() << "[UE #" << id_ << "] RRC Release received. "
                                  "Moving back to IDLE/SEARCHING...";
    state_ = UeRrcState::RRC_IDLE;
    crnti_ = 0;
}

void UeLogic::sendRegistrationRequest() {
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

void UeLogic::handleRegistrationAccept(const QByteArray& payload) {
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    QString message;
    ds >> message;

    qDebug() << "[UE #" << id_ << "] NAS Registration Accepted! Network says:"
             << message;
}

void UeLogic::onTick() {
    if (state_ == UeRrcState::IDLE || state_ == UeRrcState::DETACHED) {
        return;
    }

    switch (state_) {
        case UeRrcState::DETACHED:
        case UeRrcState::RRC_IDLE:
        case UeRrcState::RRC_INACTIVE:
            // ...
            return;

        case UeRrcState::SEARCHING_FOR_CELL:
                // ...
                return;

            case UeRrcState::RRC_CONNECTED:
                // ...
                break;

            case UeRrcState::RRC_CONNECTING:
                return;
        }
    auto now = std::chrono::steady_clock::now();
    if (now - last_report_time_ >= report_interval_) {
        sendMeasurementReport();
        last_report_time_ = now;
    }
}

void UeLogic::sendMeasurementReport() {
    if (state_ != UeRrcState::RRC_CONNECTED) {
        return;
    }

    QByteArray report;
    QDataStream ds(&report, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    double rsrp = -90.0 + QRandomGenerator::global()->bounded(10);
    ds << target_gnb_id_ << rsrp;

    qDebug() << "[UE #" << id_ << "] Sending Measurement Report. RSRP:"
             << rsrp << "dBm";
    sendSimData(ProtocolMsgType::MeasurementReport, report, target_gnb_id_);
}

void UeLogic::handleRrcReconfiguration(const QByteArray& payload) {
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t new_gnb_id;
    ds >> new_gnb_id;

    qDebug() << "[UE #" << id_ << "] RRC Reconfiguration: Handover command to gNB #"
             << new_gnb_id;

    target_gnb_id_ = new_gnb_id;
}

void UeLogic::sendChatMessage(uint32_t target_ue_id, const QString& text) {
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
