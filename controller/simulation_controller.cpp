#include "simulation_controller.hpp"

#include <QPoint>
#include <QRandomGenerator>

#include "gnb_logic.hpp"
#include "ue_logic.hpp"

SimulationController::SimulationController(SettingsPack pack, QObject* parent)
    : QObject(parent)
    , set_pack_(std::move(pack))
{
    hub_ = new RadioHub(set_pack_.hub, this);
}

void SimulationController::startSimulation()
{
    if (!hub_ || !hub_->run()) {
        qCritical() << "[SimController]: Fatal error - RadioHub failed to "
                       "start. Aborting.";
        return;
    }

    setupConnections();

    if (set_pack_.getMode() == DeployMode::Monolithic) {
        qInfo() << "[SimController]: Mode: MONOLITHIC. Let's deploy internal "
                   "nodes: ues and gnbs";
        setupGnbStations();
        setupUeDevices();
        return;
    }

    qInfo() << "[SimController]: Mode: DISTRIBUTED. Waiting for external "
               "services to run.";
}

const QHash<uint32_t, std::shared_ptr<INetworkNode>>&
SimulationController::getGnbs() const
{
    return gnbs_;
}

const QHash<uint32_t, std::shared_ptr<INetworkNode>>&
SimulationController::getUes() const
{
    return ues_;
}

void SimulationController::setupGnbStations()
{
    for (const auto& [id, pos] : set_pack_.positions.gnbs) {
        auto gnb = std::make_shared<GnbLogic>(id, set_pack_.gnb);
        gnb->setPosition({pos.X, pos.Y});
        gnb->setTxPower(set_pack_.gnb.radio.tx_power_db);

        GnbCellConfig config;
        config.tac = set_pack_.gnb.cell.tracking_area_code;
        gnb->setCellConfig(config);

        if (gnb->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
            gnb->registerAtHub();
            gnb->run();
            gnbs_[gnb->getId()] = gnb;
        }
    }
}

void SimulationController::setupUeDevices()
{
    for (const auto& [id, pos] : set_pack_.positions.ues) {
        auto ue = std::make_shared<UeLogic>(id, set_pack_.ue);
        ue->setPosition({pos.X, pos.Y});
        ue->setTxPower(23.0);

        if (ue->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
            ue->registerAtHub();
            ue->run();
            ues_[ue->getId()] = ue;
        }
    }
}

void SimulationController::setupConnections()
{
    connect(hub_, &RadioHub::nodeRegistered, this,
            &SimulationController::onNodeRegistered);
}

void SimulationController::onNodeRegistered(NodeInfo node_info)
{
    if (set_pack_.getMode() == DeployMode::Distributed) {
        if (node_info.type == EntityType::GNB) {
            auto gnb = std::make_shared<RemoteGnbProxy>(node_info);
            gnbs_[node_info.id] = gnb;
        } else {
            auto ue = std::make_shared<RemoteUeProxy>(node_info);
            ues_[node_info.id] = ue;
        }
        emit dataUpdated();
    }
}
