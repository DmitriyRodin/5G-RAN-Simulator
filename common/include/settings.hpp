#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Point2D {
    double X;
    double Y;
};

struct Paths {
    std::string build_dir;
};

struct HubSettings {
    uint16_t port;
    uint32_t id;
    uint32_t broadcast_id;
    Point2D virt_pos;
    std::string address;

    HubSettings() = delete;

    HubSettings(uint16_t p, uint32_t hid, uint32_t bid, Point2D vhp,
                std::string add);
};

struct Cell {
    uint16_t tracking_area_code;

    Cell() = delete;
};

struct RadioSettings {
    int radio_frame_duration;
    double tx_power_db;

    RadioSettings() = delete;
};

struct NodeSettings {
    HubSettings hub;
    RadioSettings radio;
    Cell cell;

    NodeSettings() = delete;

    NodeSettings(HubSettings hub_set, RadioSettings radio_set, Cell cell_set);
};

struct GnbSettings : NodeSettings {
    double radius;

    GnbSettings() = delete;
    GnbSettings(HubSettings h, RadioSettings r_set, Cell c, double r);
};

struct UeSettings : NodeSettings {
    UeSettings() = delete;
    UeSettings(HubSettings hub_set, RadioSettings radio_set, Cell cell_set);
};

struct BaseNodeContext {
    uint32_t id;
    Point2D pos;

    BaseNodeContext() = delete;
    BaseNodeContext(uint32_t node_id, Point2D position);
};

struct GnbRuntimeContext : BaseNodeContext {
    GnbSettings set;

    GnbRuntimeContext() = delete;

    GnbRuntimeContext(const uint32_t node_id, const GnbSettings settings,
                      const Point2D pos);
};

struct UeRuntimeContext : BaseNodeContext {
    UeSettings set;

    UeRuntimeContext() = delete;
    UeRuntimeContext(uint32_t node_id, UeSettings settings, Point2D pos);
};

struct Positions {
    std::unordered_map<uint32_t, Point2D> gnbs;
    std::unordered_map<uint32_t, Point2D> ues;
};

struct SimulationSettings {
    bool is_monolithic = false;

    uint32_t gnb_count;
    uint32_t ue_count;

    uint32_t gnb_id_start;
    uint32_t ue_id_start;

    SimulationSettings() = delete;
};

struct FlowLoggerSetupInfo;

struct SettingsPack {
    HubSettings hub;
    UeSettings ue;
    GnbSettings gnb;
    SimulationSettings sim;
    Positions positions;
    Paths paths;

    SettingsPack() = delete;

    SettingsPack(HubSettings h, UeSettings u, GnbSettings g,
                 SimulationSettings s, Positions pos, Paths p);

    FlowLoggerSetupInfo getFlowLoggerInfo() const;

    uint32_t getBroadcast() const;
};

#endif  // SETTINGS_HPP
