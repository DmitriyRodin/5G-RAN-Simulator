#include <QApplication>

#include "mainwindow.h"
#include "simulation_controller.hpp"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    auto controller = std::make_shared<SimulationController>();

    MainWindow w(controller);
    controller->startSimulation();
    w.show();

    return a.exec();
}
