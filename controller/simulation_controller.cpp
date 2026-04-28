#include "simulation_controller.hpp"

#include <QPoint>
#include <QRandomGenerator>

SimulationController::SimulationController(const NetworkSettings& net,
                                           const SimulationSettings& sim,
                                           const Paths& paths, QObject* parent)
    : QObject(parent)
    , network_settings_(net)
    , simulation_settings_(sim)
    , paths_(paths)
{
    hub_ = new RadioHub(network_settings_.hub_port, network_settings_.hub_id,
                        network_settings_.broadcast_id,
                        QPointF(simulation_settings_.hub_virtual_position.X,
                                simulation_settings_.hub_virtual_position.Y),
                        this);
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
    std::vector<Point2D> gnb_positions = simulation_settings_.gnb_positions;

    for (int i = 0; i < simulation_settings_.gnb_count; ++i) {
        auto gnb = std::make_shared<GnbLogic>(
            i + network_settings_.gnb_id_start, simulation_settings_.gnb_radius,
            network_settings_.radio_frame_duration, network_settings_.hub_id,
            network_settings_.broadcast_id);

        gnb->setPosition({gnb_positions[i].X, gnb_positions[i].Y});
        gnb->setTxPower(network_settings_.tx_power_db);

        GnbCellConfig config;
        config.tac = network_settings_.tracking_area_code;
        gnb->setCellConfig(config);

        if (gnb->setupNetwork(0)) {
            gnb->registerAtHub(QHostAddress::LocalHost,
                               network_settings_.hub_port);
            gnb->run();
            gnbs_[gnb->getId()] = gnb;
        }
    }
}

void SimulationController::setupUeDevices()
{
    int ue_created = 0;

    for (auto it = gnbs_.begin(); it != gnbs_.end(); ++it) {
        const QPointF gnb_positions = it.value()->position();

        const int ue_count_per_gnb = simulation_settings_.ue_per_gnb;
        for (int i = 0; i < ue_count_per_gnb; ++i) {
            auto ue = std::make_shared<UeLogic>(
                ue_created + network_settings_.ue_id_start,
                network_settings_.radio_frame_duration,
                network_settings_.hub_id, network_settings_.broadcast_id);

            double angle =
                QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            double dist = QRandomGenerator::global()->bounded(
                simulation_settings_.ue_position_boundary.min,
                simulation_settings_.ue_position_boundary.max);
            const QPointF ue_position(gnb_positions.x() + dist * cos(angle),
                                      gnb_positions.y() + dist * sin(angle));

            ue->setPosition(ue_position);
            ue->setTxPower(23.0);

            if (ue->setupNetwork(0)) {
                ue->registerAtHub(QHostAddress::LocalHost,
                                  network_settings_.hub_port);
                ues_[ue->getId()] = ue;
            }
            ue_created++;
        }
    }

    while (ue_created < simulation_settings_.ue_count) {
        auto ue = std::make_shared<UeLogic>(
            ue_created + network_settings_.ue_id_start,
            network_settings_.radio_frame_duration, network_settings_.hub_id,
            network_settings_.broadcast_id);

        const double random_x =
            QRandomGenerator::global()->bounded(-3000, 3000);
        const double random_y =
            QRandomGenerator::global()->bounded(-3000, 3000);

        ue->setPosition({random_x, random_y});
        ue->setTxPower(23.0);

        if (ue->setupNetwork(0)) {
            ue->registerAtHub(QHostAddress::LocalHost,
                              network_settings_.hub_port);
            ue->run();
            ues_[ue->getId()] = ue;
        }
        ue_created++;
    }
}
