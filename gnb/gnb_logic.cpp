#include <QDebug>

#include "gnb_logic.hpp"

GnbLogic::GnbLogic(int id, QObject* parent )
    : BaseEntity(id, EntityType::GNB, parent)
{
    main_timer_ = new QTimer(this);
    connect(main_timer_, &QTimer::timeout, this, &GnbLogic::onTick);

    last_broadcast_ = std::chrono::steady_clock::now();
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

void GnbLogic::processIncoming(const QByteArray& data,
                               const QHostAddress& sender_ip,
                               quint16 sender_port) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "GNB #" << id_
                   << " failed to parse JSON:" << error.errorString();
        return;
    }

    const QJsonObject obj = doc.object();
    const MessageType msg_type = parseMessageType(obj);;
    const int ue_id = obj["ue_id"].toInt(-1);

    if (ue_id < 0 || (msg_type == MessageType::Unknown)) {
        qWarning() << "Ooops, GNB #" << id_ << " received invalid message from UE# " << ue_id;
        return;
    }

    const auto now = QDateTime::currentDateTime();
    auto it = ue_contexts_.find(ue_id);

    if (it == ue_contexts_.end()) {
        if (msg_type != MessageType::AttachRequest) {
            qWarning() << "GNB #" << id_
                       << "ignores the message from unknown UE #" << ue_id << "because this UE isn't attached";
            return;
        }

        UeContext ctx{};
        ctx.id = ue_id;
        ctx.is_attached = true;
        ctx.last_activity = now;
        ctx.last_rssi = obj.contains("rssi") ? obj["rssi"].toDouble() : -100.0;
        ctx.ip_address = sender_ip;
        ctx.port = sender_port;

        ue_contexts_.emplace(ue_id, ctx);

        qDebug() << "GNB #" << id_
                 << " attached new UE # " << ue_id
                 << sender_ip.toString() << sender_port;

        handleAttachRequest(obj);
        return;
    }

    UeContext& ctx = it->second;

    ctx.ip_address = sender_ip;
    ctx.port = sender_port;

    if (msg_type == MessageType::MeasurementReport) {
        ctx.last_activity = now;

        if (obj.contains("rssi")) {
            ctx.last_rssi = obj["rssi"].toDouble();
        }

        handleMeasurementReport(obj);
    }
    else if (msg_type == MessageType::DataTransfer) {
        ctx.last_activity = now;
        handleUeData(obj);
    }
    else if (msg_type == MessageType::AttachRequest) {
        ctx.last_activity = now;
        handleAttachRequest(obj);
    }
    else {
        qDebug() << "GNB #" << id_ << " unknown message type";
    }
}

void GnbLogic::sendBroadcastInfo()
{
    QJsonObject sib;
    sib["type"] = "SIB1";
    sib["gnb_id"] = id_;

    // SEND SIB info

    qDebug() << "GNB #" << id_ << " sents SIB1 broadcast";
}

void GnbLogic::handleUeData(const QJsonObject &obj)
{
    // HANDLE UE data logic
}

void GnbLogic::handleAttachRequest(const QJsonObject& obj)
{
    const int ue_id = obj["ue_id"].toInt();

    auto it = ue_contexts_.find(ue_id);
    if (it == ue_contexts_.end()) {
        qWarning() << "handleAttachRequest called for unknown UE #" << ue_id;
        return;
    }

    UeContext& ctx = it->second;

    qDebug() << "GNB #" << id_
             << " is processing ATTACH REQUEST from UE #" << ue_id;

    // RRC logic

    ctx.is_attached = true;
    ctx.last_activity = QDateTime::currentDateTime();

    QJsonObject response;
    response["type"] = "ATTACH_ACCEPT";
    response["ue_id"] = ue_id;
    response["cell_id"] = id_;
    response["tac"] = 42;

    QJsonDocument doc(response);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    sendSimData(data, ctx.ip_address, ctx.port, ue_id);
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
    QJsonObject hoCommand;
    hoCommand["type"] = "RRC_RECONFIGURATION";
    hoCommand["ue_id"] = ue_id;
    hoCommand["target_gnb_id"] = target_Gnb_id;
    hoCommand["action"] = "HANDOVER";

    sendData(ue_id, hoCommand);
}

void GnbLogic::sendData(int ue_id, QJsonObject &payload)
{
    auto it = ue_contexts_.find(ue_id);

    if (it == ue_contexts_.end()) {
        qWarning() << "Ooops, GNB #" << id_
                   << " is trying to send payload to unknown UE #" << ue_id;
        return;
    }

    const UeContext& ctx = it->second;
    payload["ue_id"] = ue_id;
    QJsonDocument doc(payload);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    sendSimData(data, ctx.ip_address, ctx.port, ue_id);
}
