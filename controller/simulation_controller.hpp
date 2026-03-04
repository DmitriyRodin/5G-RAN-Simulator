#ifndef SIMULATION_CONTROLLER_HPP
#define SIMULATION_CONTROLLER_HPP

#include <memory>

#include <QList>

#include "gnb_logic.hpp"
#include "radio_hub.hpp"
#include "ue_logic.hpp"

class SimulationController : public QObject
{
    Q_OBJECT
public:
    explicit SimulationController(QObject* parent = nullptr);

    void startSimulation();
    QList<std::shared_ptr<GnbLogic>> getGnbs() const;
    QList<std::shared_ptr<UeLogic>> getUes() const;

private:
    void setupGnbStations();
    void setupUeDevices();

    RadioHub* hub_ = nullptr;
    QHash<uint32_t, std::shared_ptr<GnbLogic>> gnbs_;
    QHash<uint32_t, std::shared_ptr<UeLogic>> ues_;

    const quint16 HUB_PORT = 5555;
    const int GNB_COUNT = 3;
    const int UE_COUNT = 16;
    const double COVERAGE_RADIUS = 600.0;
    const uint16_t Tracking_Area_Code = 100;
};

#endif  // SIMULATION_CONTROLLER_HPP
