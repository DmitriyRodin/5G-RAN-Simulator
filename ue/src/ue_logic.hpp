#ifndef UE_LOGIC_HPP
#define UE_LOGIC_HPP

#include <QTimer>
#include <QHash>

#include <chrono>

#include "common/base_entity.hpp"

class UeLogic : public BaseEntity {
    Q_OBJECT
public:
    explicit UeLogic(int id, QObject *parent = nullptr);

    void run() override;

protected:
    void processIncoming(const QByteArray& data,
                         const QHostAddress& sender_ip,
                         quint16 sender_port) override;

private slots:
    void onTick();

private:
    void handleSib1(const QJsonObject& obj,
                    const QHostAddress& gnb_ip,
                    quint16 gnb_port);
    void handleRegistrationAccept(const QJsonObject& obj);
    void handleRrcReconfiguration(const QJsonObject& obj);

    void sendRegistrationRequest();
    void sendMeasurementReport();

    int connected_gNB_id_ = -1;
    bool is_attached_ = false;

    QHostAddress gNB_ip_;
    quint16 gNB_port_ = 0;

    QTimer* timer_;
    std::chrono::steady_clock::time_point last_report_time_;
    const std::chrono::milliseconds report_interval_{500};

    struct KnownGnb {
        QHostAddress ip_;
        quint16 port_;
    };
    QHash<int, KnownGnb> known_gNBs_;
};

#endif // UE_LOGIC_HPP
