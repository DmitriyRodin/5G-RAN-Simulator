#include "simulation_controller.hpp"

#include <QPoint>
#include <QRandomGenerator>

SimulationController::SimulationController(QObject* parent)
    : QObject(parent)
{
    hub_ = new RadioHub(HUB_PORT, this);
}

void SimulationController::startSimulation()
{
    setupGnbStations();
    setupUeDevices();
}

QList<std::shared_ptr<GnbLogic>> SimulationController::getGnbs() const
{
    return gnbs_.values();
}

QList<std::shared_ptr<UeLogic>> SimulationController::getUes() const
{
    return ues_.values();
}

void SimulationController::setupGnbStations()
{
    QList<QPointF> gnb_positions = {QPointF(-1800, -1000), QPointF(1800, -1400),
                                    QPointF(0, 1300)};

    for (int i = 0; i < GNB_COUNT; ++i) {
        auto gnb =
            std::make_shared<GnbLogic>(i + NetConfig::GNB_ID_START,
                                       NetConfig::GNB_DEFAULT_COVERAGE_RADIUS);

        gnb->setPosition(gnb_positions[i]);
        gnb->setTxPower(43.0);

        GnbCellConfig config;
        config.tac = Tracking_Area_Code;
        gnb->setCellConfig(config);

        if (gnb->setupNetwork(0)) {
            gnb->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
            gnb->run();
            gnbs_[gnb->getId()] = gnb;
        }
    }
}

void SimulationController::setupUeDevices()
{
    int ue_created = 1;

    for (auto it = gnbs_.begin(); it != gnbs_.end(); ++it) {
        const QPointF gnb_positions = it.value()->position();

        const int ue_count_per_gnb = 4;
        for (int i = 1; i < ue_count_per_gnb; ++i) {
            auto ue =
                std::make_shared<UeLogic>(ue_created + NetConfig::UE_ID_START);

            double angle =
                QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            double dist = QRandomGenerator::global()->bounded(200, 2000);
            const QPointF ue_position(gnb_positions.x() + dist * cos(angle),
                                      gnb_positions.y() + dist * sin(angle));

            ue->setPosition(ue_position);
            ue->setTxPower(23.0);

            if (ue->setupNetwork(0)) {
                ue->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
                ues_[ue->getId()] = ue;
            }
            ue_created++;
        }
    }

    while (ue_created <= UE_COUNT) {
        auto ue =
            std::make_shared<UeLogic>(ue_created + NetConfig::UE_ID_START);

        const double random_x =
            QRandomGenerator::global()->bounded(-3000, 3000);
        const double random_y =
            QRandomGenerator::global()->bounded(-3000, 3000);

        ue->setPosition({random_x, random_y});
        ue->setTxPower(23.0);

        if (ue->setupNetwork(0)) {
            ue->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
            ue->run();
            ues_[ue->getId()] = ue;
        }
        ue_created++;
    }
}
