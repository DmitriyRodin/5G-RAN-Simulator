#ifndef SIMULATION_CONTROLLER_HPP
#define SIMULATION_CONTROLLER_HPP

#include <memory>

#include <QList>

#include "radio_hub.hpp"
#include "settings.hpp"

class SimulationController : public QObject
{
    Q_OBJECT
public:
    explicit SimulationController(SettingsPack pack, QObject* parent = nullptr);

    void startSimulation();
    const QHash<uint32_t, std::shared_ptr<INetworkNode>>& getGnbs() const;
    const QHash<uint32_t, std::shared_ptr<INetworkNode>>& getUes() const;

signals:
    void dataUpdated();

private slots:
    void onNodeRegistered(NodeInfo node_info);

private:
    void setupGnbStations();
    void setupUeDevices();

    void setupConnections();

    SettingsPack set_pack_;

    RadioHub* hub_ = nullptr;
    QHash<uint32_t, std::shared_ptr<INetworkNode>> gnbs_;
    QHash<uint32_t, std::shared_ptr<INetworkNode>> ues_;
};

#endif  // SIMULATION_CONTROLLER_HPP
