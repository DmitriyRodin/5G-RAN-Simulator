#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <cmath>

#include "ue_logic.hpp"

UeLogic::UeLogic(int id, QObject *parent)
    : BaseEntity(id, EntityType::UE, parent)
{
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &UeLogic::onTick);
    last_report_time_ = std::chrono::steady_clock::now();
}

void UeLogic::run() {
    qDebug() << "UE #" << id_ << " started";
    timer_->start(50);
}

void UeLogic::processIncoming(const QByteArray& data,
                              const QHostAddress& sender_ip,
                              quint16 sender_port) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "SIB1") {
        handleSib1(obj, sender_ip, sender_port);
    } else if (type == "REGISTRATION_ACCEPT") {
        handleRegistrationAccept(obj);
    } else if (type == "RRC_RECONFIGURATION") {
        handleRrcReconfiguration(obj);
    }
}

void UeLogic::handleSib1(const QJsonObject& obj,
                         const QHostAddress& gnb_ip,
                         quint16 gnb_port) {
    int gnb_id = obj["gnb_id"].toInt();

    known_gNBs_[gnb_id] = {gNB_ip_, gNB_port_};

    if (!is_attached_ && connected_gNB_id_ == -1) {
        connected_gNB_id_ = gnb_id;
        gNB_ip_ = gnb_ip;
        gNB_port_ = gnb_port;
        sendRegistrationRequest();
    }
}

void UeLogic::sendRegistrationRequest() {
    QJsonObject request;
    request["type"] = "REGISTRATION_REQUEST";
    request["ue_id"] = id_;
    qDebug() << "UE #" << id_ << "sent REGISTRATION_REQUEST to GNB #"
             << connected_gNB_id_ << ". The result of sending:";
    sendSimData(QJsonDocument(request).toJson(), gNB_ip_, gNB_port_, id_);
}

void UeLogic::handleRegistrationAccept(const QJsonObject& obj) {
    is_attached_ = true;
    qDebug() << "UE #" << id_ << " successfully REGISTRED to GNB #"
             << obj["gnb_id"].toInt();
}

void UeLogic::onTick() {
    auto now = std::chrono::steady_clock::now();
    if (is_attached_ && (now - last_report_time_ >= report_interval_)) {
        sendMeasurementReport();
        last_report_time_ = now;
    }
}

void UeLogic::sendMeasurementReport() {
    QJsonObject report;
    report["type"] = "MEASUREMENT_REPORT";
    report["ue_id"] = id_;

    QJsonArray measurements;
    for (auto it = known_gNBs_.begin(); it != known_gNBs_.end(); ++it) {
        const double sim_distance_multiplier = 10.0;
        const double dist = sim_distance_multiplier * id_;

        double rssi = -30.0 - 20.0 * std::log10(dist + 1.0);

        QJsonObject m;
        m["gnb_id"] = it.key();
        m["rssi"] = rssi;
        measurements.append(m);
    }

    report["measurements"] = measurements;
    sendSimData(QJsonDocument(report).toJson(), gNB_ip_, gNB_port_, id_);
}

void UeLogic::handleRrcReconfiguration(const QJsonObject& obj) {
    if (obj["action"].toString() == "HANDOVER") {
        int target_id = obj["target_gnb_id"].toInt();
        qDebug() << "UE #" << id_ << " EXECUTING HANDOVER to GNB #" << target_id;

        if (known_gNBs_.contains(target_id)) {
            connected_gNB_id_= target_id;
            gNB_ip_ = known_gNBs_[target_id].ip_;
            gNB_port_ = known_gNBs_[target_id].port_;

            qDebug() << "UE #" << id_ << " switched to new cell frequency.";
        }
    }
}
