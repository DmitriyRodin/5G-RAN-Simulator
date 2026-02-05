#include <QPoint>
#include <QRandomGenerator>

#include "simulation_controller.hpp"

SimulationController::SimulationController(QObject *parent) : QObject(parent)
{
    hub_ = new RadioHub(HUB_PORT, this);
}

void SimulationController::startSimulation()
{
    setupGnbStations();
    setupUeDevices();
}

void SimulationController::setupGnbStations()
{
    QList<QPointF> gnbPositions = {
        QPointF(0, 0),
        QPointF(1500, 0),
        QPointF(750, 1300)
    };

    for (int i = 0; i < GNB_COUNT; ++i) {
        auto* gnb = new GnbLogic(i + NetConfig::GNB_ID_START , this);
        gnb->setPosition(gnbPositions[i]);
        gnb->setTxPower(43.0);
        GnbCellConfig config;
        config.tac = Tracking_Area_Code;
        gnb->setCellConfig(config);

        if (gnb->setupNetwork(0)) {
            gnb->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
            gnb->run();
            gnbs_.append(gnb);
        }
    }
}

void SimulationController::setupUeDevices()
{
    int ue_created = 1;

    for (int g = 0; g < gnbs_.size(); ++g) {
        QPointF gnbPos = gnbs_[g]->position();

        for (int i = 1; i < 6; ++i) {
            auto* ue = new UeLogic(ue_created + NetConfig::UE_ID_START, this);

            double angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            double dist = QRandomGenerator::global()->bounded(100, 500);
            QPointF uePos(gnbPos.x() + dist * cos(angle), gnbPos.y() + dist * sin(angle));

            ue->setPosition(uePos);
            ue->setTxPower(23.0);

            if (ue->setupNetwork(0)) {
                ue->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
                ues_.append(ue);
            }
            ue_created++;
        }
    }

    while (ue_created <= UE_COUNT ) {
        auto* ue = new UeLogic(ue_created + NetConfig::UE_ID_START, this);

        ue->setPosition({-3000.0, -3000.0});

        if (ue->setupNetwork(0)) {
            ue->registerAtHub(QHostAddress::LocalHost, HUB_PORT);
            ues_.append(ue);
        }
        ue_created++;
    }
}
