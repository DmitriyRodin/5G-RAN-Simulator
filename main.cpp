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

    auto pack = config.getSettingsPack();
    if (!pack) {
        qCritical() << "SettingsPack is empty";
        return EXIT_FAILURE;
    }

    FlowLogger::setup(pack->getFlowLoggerInfo());

    auto controller = std::make_shared<SimulationController>(pack.value());

    MainWindow w(controller);
    controller->startSimulation();
    w.show();

    return a.exec();
}
