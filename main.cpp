#include <controller/simulation_controller.hpp>

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    SimulationController controller;
    controller.startSimulation();

    w.show();
    return a.exec();
}
