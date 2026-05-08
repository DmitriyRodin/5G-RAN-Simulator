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

struct NetworkSettings {
    uint16_t hub_port;
    uint32_t hub_id;
    uint32_t broadcast_id;

    uint32_t gnb_id_start;
    uint32_t ue_id_start;

    uint16_t tracking_area_code;

    double tx_power_db;
    int radio_frame_duration;
};

struct SimulationSettings {
    int gnb_count;
    int ue_count;

    std::unordered_map<uint32_t, Point2D> gnb_positions_;
    std::unordered_map<uint32_t, Point2D> ue_positions_;

    double gnb_radius;

    Point2D hub_virtual_position;
};

struct Paths {
    std::string build_dir;
};

struct HubSettings {
    uint16_t hub_port;
    uint32_t hub_id;
    uint32_t broadcast_id;
    Point2D virt_hub_pos;
};

struct GnbSettings {
    double gnb_radius;
    int radio_frame_duration;
    uint32_t hub_id;
    uint32_t broadcast_id;
    uint16_t hub_port;
};

struct UeSettings {
    int radio_frame_duration;
    uint32_t hub_id;
    uint32_t broadcast_id;
    uint16_t hub_port;
};

#endif  // SETTINGS_HPP
