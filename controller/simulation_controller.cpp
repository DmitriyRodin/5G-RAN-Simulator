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
    hub_ = new RadioHub(
        HubSettings{network_settings_.hub_port, network_settings_.hub_id,
                    network_settings_.broadcast_id,
                    simulation_settings_.hub_virtual_position},
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
    for (const auto& [id, pos] : simulation_settings_.gnb_positions_) {
        auto gnb = std::make_shared<GnbLogic>(
            id, GnbSettings{simulation_settings_.gnb_radius,
                            network_settings_.radio_frame_duration,
                            network_settings_.hub_id,
                            network_settings_.broadcast_id});
        gnb->setPosition({pos.X, pos.Y});
        gnb->setTxPower(network_settings_.tx_power_db);

        GnbCellConfig config;
        config.tac = network_settings_.tracking_area_code;
        gnb->setCellConfig(config);

        if (gnb->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
            gnb->registerAtHub(QHostAddress::LocalHost,
                               network_settings_.hub_port);
            gnb->run();
            gnbs_[gnb->getId()] = gnb;
        }
    }
}

void SimulationController::setupUeDevices()
{
    for (const auto& [id, pos] : simulation_settings_.ue_positions_) {
        auto ue = std::make_shared<UeLogic>(
            id,
            UeSettings{network_settings_.radio_frame_duration,
                       network_settings_.hub_id, network_settings_.broadcast_id,
                       network_settings_.hub_port});
        ue->setPosition({pos.X, pos.Y});
        ue->setTxPower(23.0);

        if (ue->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
            ue->registerAtHub(QHostAddress::LocalHost,
                              network_settings_.hub_port);
            ue->run();
            ues_[ue->getId()] = ue;
        }
    }
}
