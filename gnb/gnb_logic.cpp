#include <QDebug>

#include "gnb_logic.hpp"

GnbLogic::GnbLogic(int id, QObject* parent )
    : BaseEntity(id, EntityType::GNB, parent)
{
    main_timer_ = new QTimer(this);
    connect(main_timer_, &QTimer::timeout, this, &GnbLogic::onTick);
    last_broadcast_ = std::chrono::steady_clock::now();
    connect(this, &BaseEntity::registrationConfirmed, this,&GnbLogic::sendBroadcastInfo);
}

void GnbLogic::run() {
    qDebug() << "GNB #" << id_ << "is running";
    main_timer_->start(20);
}

void GnbLogic::onTick() {
    auto now = std::chrono::steady_clock::now();

    if (now - last_broadcast_ >= broadcast_interval_) {
        sendBroadcastInfo();
        last_broadcast_ = now;
    }
}

void GnbLogic::onProtocolMessageReceived(uint32_t ue_id, ProtocolMsgType type, const QByteArray &payload)
{
    // UPDATE UeContext data

    switch (type) {
        case ProtocolMsgType::RrcSetupRequest:
            // handle RrcSetupRequest
            break;

        case ProtocolMsgType::MeasurementReport:
            // Handle Measurement Report
            break;

        case ProtocolMsgType::UserPlaneData:
            handleUeData(ue_id, payload);
            break;
        // ...
        default:
            qDebug() << "[gNB] Unhandled protocol type:" << static_cast<int>(type);
    }
}

void GnbLogic::sendBroadcastInfo()
{
    // SEND SIB info

    qDebug() << "GNB #" << id_ << " sents SIB1 broadcast";
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

    qDebug() << "GNB #" << id_ << " RRC info: Received Measurement Report from UE #" << ue_id;

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
