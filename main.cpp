#include <QApplication>

#include "config_manager.hpp"
#include "flow_logger.hpp"
#include "mainwindow.h"
#include "simulation_controller.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    auto& config = ConfigManager::instance();
    if (!config.load("../config.yaml")) {
        qCritical() << "Failed to load config. Simulation aborted.";
        return -1;
    }

    NetworkSettings net = config.getNetworkSettings();
    SimulationSettings sim = config.getSimulationSettings();
    Paths paths = config.getPaths();

    FlowLogger::setup(net.hub_id, net.broadcast_id);

    auto controller = std::make_shared<SimulationController>(net, sim, paths);

    MainWindow w(controller);
    controller->startSimulation();
    w.show();

    return a.exec();
}
