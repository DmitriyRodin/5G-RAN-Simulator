#include "config_manager.hpp"

#include <QDebug>

bool ConfigManager::load(const std::string& filename)
{
    try {
        YAML::Node root = YAML::LoadFile(filename);

        if (!root.IsDefined() || root.IsNull()) {
            qWarning() << "[ConfigManager]: Config file is empty or invalid";
            return false;
        }

        parseNetwork(root);
        parseSimulation(root);
        parsePaths(root);

        qDebug() << "[ConfigManager]: Configuration successfully mapped to "
                    "structures";
        return true;

    } catch (const std::exception& e) {
        qWarning() << "[ConfigManager]: Error parsing YAML:" << e.what();
        return false;
    }
}

void ConfigManager::parseNetwork(const YAML::Node& node)
{
    validateSection(node, "network");

    const auto net_node = node["network"];

    network_settings_.hub_port = getRequired<uint16_t>(net_node, "hub_port");
    network_settings_.gnb_id_start =
        getRequired<uint32_t>(net_node, "gnb_id_start");
    network_settings_.ue_id_start =
        getRequired<uint32_t>(net_node, "ue_id_start");
    network_settings_.tracking_area_code =
        getRequired<uint32_t>(net_node, "tracking_area_code");

    network_settings_.tx_power_db = net_node["tx_power_db"].as<double>(43.0);
    network_settings_.radio_frame_duration =
        net_node["radio_frame_duration"].as<int>(10);

    network_settings_.hub_id = net_node["hub_id"].as<uint32_t>(0);
    network_settings_.broadcast_id =
        net_node["broadcast_id"].as<uint32_t>(0xFFFFFFFF);

    qDebug() << "[ConfigManager] Network settings parsed successfully";
}

void ConfigManager::parseSimulation(const YAML::Node& node)
{
    validateSection(node, "simulation");

    const auto sim_node = node["simulation"];

    simulation_settings_.gnb_count = getRequired<int>(sim_node, "gnb_count");
    simulation_settings_.ue_count = getRequired<int>(sim_node, "ue_count");
    simulation_settings_.ue_per_gnb = getRequired<int>(sim_node, "ue_per_gnb");

    getRequiredGnbPositions(sim_node);

    simulation_settings_.gnb_radius =
        getRequired<double>(sim_node, "gnb_radius");

    auto ue_position_boundary_pair =
        getRequiredPair<int>(sim_node, "ue_position_boundaries");
    simulation_settings_.ue_position_boundary.min =
        ue_position_boundary_pair.first;
    simulation_settings_.ue_position_boundary.max =
        ue_position_boundary_pair.second;

    auto ue_map_boundary_pair =
        getRequiredPair<int>(sim_node, "ue_map_boundary");
    simulation_settings_.ue_map_boundary.min = ue_map_boundary_pair.first;
    simulation_settings_.ue_map_boundary.max = ue_map_boundary_pair.second;

    auto hub_virt_pos_pair =
        getRequiredPair<double>(sim_node, "hub_virtual_position");
    simulation_settings_.hub_virtual_position = {hub_virt_pos_pair.first,
                                                 hub_virt_pos_pair.second};

    qDebug() << "[ConfigManager] Simulation settings parsed successfully";
}

void ConfigManager::parsePaths(const YAML::Node& node)
{
    validateSection(node, "paths");
    auto build_node = node["paths"];

    try {
        paths_.build_dir = getRequired<std::string>(build_node, "build_dir");

        qDebug()
            << "[Config] Paths settings parsed successfully. Build directory:"
            << QString::fromStdString(paths_.build_dir);

    } catch (const std::exception& e) {
        throw std::runtime_error("Paths config error: " +
                                 std::string(e.what()));
    }
}

const NetworkSettings& ConfigManager::getNetworkSettings() const
{
    return network_settings_;
}
const SimulationSettings& ConfigManager::getSimulationSettings() const
{
    return simulation_settings_;
}

const Paths& ConfigManager::getPaths() const
{
    return paths_;
}

void ConfigManager::validateSection(const YAML::Node& node,
                                    const std::string& sectionName)
{
    if (!node || !node.IsDefined()) {
        throw std::runtime_error("Critical error: '" + sectionName +
                                 "' section is missing");
    }
}

void ConfigManager::getRequiredGnbPositions(const YAML::Node& node)
{
    const std::string key = "gnb_positions";
    const auto& pos_gnb_node = node[key];
    if (!pos_gnb_node) {
        throw std::runtime_error(
            "Critical error: missing mandatory parameter: : '" + key + "'");
    }
    if (!pos_gnb_node.IsSequence()) {
        throw std::runtime_error("Parameter : '" + key +
                                 "' must be a YAML sequence (list)");
    }

    auto& target = simulation_settings_.gnb_positions;
    target.clear();
    target.reserve(pos_gnb_node.size());

    size_t index = 0;
    for (const auto& item : pos_gnb_node) {
        if (!item.IsSequence()) {
            throw std::runtime_error("gnb_positions[" + std::to_string(index) +
                                     "] must be a sequence like [x, y]");
        }
        if (item.size() < 2) {
            throw std::runtime_error("gnb_positions[" + std::to_string(index) +
                                     "] is too short. Expected [x, y]");
        }

        try {
            target.push_back({item[0].as<double>(), item[1].as<double>()});
        } catch (const std::exception& e) {
            throw std::runtime_error("gnb_positions[" + std::to_string(index) +
                                     "] invalid numeric format: " + e.what());
        }
        index++;
    }
}
