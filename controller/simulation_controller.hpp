#ifndef SIMULATION_CONTROLLER_HPP
#define SIMULATION_CONTROLLER_HPP

#include <QList>
#include "gnb/gnb_logic.hpp"
#include "ue/src/ue_logic.hpp"
#include <radio-hub/radio_hub.hpp>

class SimulationController : public QObject
{
    Q_OBJECT
public:
    explicit SimulationController(QObject *parent = nullptr);

    void startSimulation();

private:
    void setupGnbStations();
    void setupUeDevices();

    RadioHub* hub_ = nullptr;
    QList<GnbLogic*> gnbs_;
    QList<UeLogic*> ues_;

    const quint16 HUB_PORT = 5555;
    const int GNB_COUNT = 3;
    const int UE_COUNT = 20;
    const double COVERAGE_RADIUS = 600.0;
};

#endif // SIMULATION_CONTROLLER_HPP
