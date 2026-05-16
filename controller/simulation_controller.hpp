#ifndef SIMULATION_CONTROLLER_HPP
#define SIMULATION_CONTROLLER_HPP

#include <memory>

#include <QList>

#include "gnb_logic.hpp"
#include "radio_hub.hpp"
#include "settings.hpp"
#include "ue_logic.hpp"

class SimulationController : public QObject
{
    Q_OBJECT
public:
    explicit SimulationController(SettingsPack pack, QObject* parent = nullptr);

    void startSimulation();
    QList<std::shared_ptr<GnbLogic>> getGnbs() const;
    QList<std::shared_ptr<UeLogic>> getUes() const;

signals:
    void dataUpdated();

private slots:
    void onNodeRegistered(uint32_t id, EntityType type, QPointF pos);

private:
    void setupGnbStations();
    void setupUeDevices();

    void setupConnections();

    SettingsPack set_pack_;

    RadioHub* hub_ = nullptr;
    QHash<uint32_t, std::shared_ptr<GnbLogic>> gnbs_;
    QHash<uint32_t, std::shared_ptr<UeLogic>> ues_;
};

#endif  // SIMULATION_CONTROLLER_HPP
