#include "mainwindow.h"

#include <QApplication>

#include "common/base_entity.hpp"
#include "gnb/gnb_logic.hpp"
#include "ue/src/ue_logic.hpp"
#include <controller/simulation_controller.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    SimulationController controller;
    controller.startSimulation();

    w.show();
    return a.exec();
}
