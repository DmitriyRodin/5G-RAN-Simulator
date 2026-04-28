#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <string>
#include <vector>

struct Point2D {
    double X;
    double Y;
};

struct NetworkSettings {
    uint16_t hub_port;

    uint32_t gnb_id_start;
    uint32_t ue_id_start;

    uint16_t tracking_area_code;

    double tx_power_db;
    int radio_frame_duration;

    uint32_t hub_id;
    uint32_t broadcast_id;
};

struct SimulationSettings {
    int gnb_count;
    int ue_count;
    int ue_per_gnb;

    std::vector<Point2D> gnb_positions;

    double gnb_radius;

    struct UePositionBoundary {
        int16_t min;
        int16_t max;
    };

    UePositionBoundary ue_position_boundary;

    struct UeMapBoundary {
        int16_t min;
        int16_t max;
    };

    UeMapBoundary ue_map_boundary;

    Point2D hub_virtual_position;
};

struct Paths {
    std::string build_dir;
};

#endif  // SETTINGS_HPP
