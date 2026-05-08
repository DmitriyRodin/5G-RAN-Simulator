#include "config_manager.hpp"

#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>

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

    qDebug() << "[ConfigManager]: Network settings parsed successfully";
}

void ConfigManager::parseSimulation(const YAML::Node& node)
{
    validateSection(node, "simulation");

    const auto sim_node = node["simulation"];

    simulation_settings_.gnb_count = getRequired<int>(sim_node, "gnb_count");
    simulation_settings_.ue_count = getRequired<int>(sim_node, "ue_count");

    validateSection(node, "gnb_positions_list");

    if (sim_node["gnb_positions_list"]) {
        for (const auto& item : sim_node["gnb_positions_list"]) {
            uint32_t id = item["id"].as<uint32_t>();
            double x = item["pos"][0].as<double>();
            double y = item["pos"][1].as<double>();
            simulation_settings_.gnb_positions_[id] = {x, y};
        }
    }

    validateSection(node, "ue_positions_list");

    if (sim_node["ue_positions_list"]) {
        for (const auto& node : sim_node["ue_positions_list"]) {
            uint32_t id = node["id"].as<uint32_t>();
            double x = node["pos"][0].as<double>();
            double y = node["pos"][1].as<double>();
            simulation_settings_.ue_positions_[id] = {x, y};
        }
    }

    simulation_settings_.gnb_radius =
        getRequired<double>(sim_node, "gnb_radius");

    auto hub_virt_pos_pair =
        getRequiredPair<double>(sim_node, "hub_virtual_position");
    simulation_settings_.hub_virtual_position = {hub_virt_pos_pair.first,
                                                 hub_virt_pos_pair.second};

    qDebug() << "[ConfigManager]: Simulation settings parsed successfully";
}

void ConfigManager::parsePaths(const YAML::Node& node)
{
    validateSection(node, "paths");
    auto build_node = node["paths"];

    try {
        paths_.build_dir = getRequired<std::string>(build_node, "build_dir");

        qDebug() << "[ConfigManager]: Paths settings parsed successfully. "
                    "Build directory:"
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

Point2D ConfigManager::getGnbPosition(const uint32_t id) const
{
    auto it = simulation_settings_.gnb_positions_.find(id);
    if (it == simulation_settings_.gnb_positions_.end()) {
        throw std::runtime_error("[ConfigManager]: GNB ID# " +
                                 std::to_string(id) +
                                 " not found in configuration list.");
    }
    return it->second;
}

Point2D ConfigManager::getUePosition(const uint32_t id) const
{
    auto it = simulation_settings_.ue_positions_.find(id);
    if (it == simulation_settings_.ue_positions_.end()) {
        throw std::runtime_error("[ConfigManager]: UE ID# " +
                                 std::to_string(id) +
                                 " not found in configuration list.");
    }
    return it->second;
}

void ConfigManager::validateSection(const YAML::Node& node,
                                    const std::string& sectionName)
{
    if (!node || !node.IsDefined()) {
        throw std::runtime_error("[ConfigManager]: '" + sectionName +
                                 "' section is missing");
    }
}

bool ConfigManager::initializeFromArgs(const QString& description,
                                       const EntityType type)
{
    node_type_ = type;
    QCommandLineParser parser;
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption id_option(
        QStringList() << "i"
                      << "id",
        "Unique 32-bit ID for the " + typeToString(node_type_), "id");

    QCommandLineOption configOption({"c", "config"},
                                    "Path to the configuration file.", "file",
                                    "config.yaml");

    parser.addOption(id_option);
    parser.addOption(configOption);

    parser.process(*QCoreApplication::instance());

    if (!parser.isSet(id_option)) {
        qCritical() << "[ConfigManager]: node ID is required.";
        parser.showHelp(1);
    }

    node_id_ = parser.value(id_option).toUInt();

    QString configPath = parser.value(configOption);
    QFileInfo checkFile(configPath);

    if (!checkFile.exists() || !checkFile.isFile()) {
        qCritical() << "[ConfigManager]: CRITICAL ERROR: Configuration file "
                       "not found at:"
                    << checkFile.absoluteFilePath();
        return false;
    }

    const std::string final_path = checkFile.absoluteFilePath().toStdString();

    if (!load(final_path)) {
        return false;
    }

    qDebug() << "[ConfigManager]: SUCCESS: Config loaded from "
             << QString::fromStdString(final_path);
    return true;
}

HubSettings ConfigManager::getHubSettings() const
{
    return HubSettings{network_settings_.hub_port, network_settings_.hub_id,
                       network_settings_.broadcast_id,
                       simulation_settings_.hub_virtual_position};
}

GnbSettings ConfigManager::getGnbSettings() const
{
    return GnbSettings{simulation_settings_.gnb_radius,
                       network_settings_.radio_frame_duration,
                       network_settings_.hub_id, network_settings_.broadcast_id,
                       network_settings_.hub_port};
}

UeSettings ConfigManager::getUeSettings() const
{
    return UeSettings{network_settings_.radio_frame_duration,
                      network_settings_.hub_id, network_settings_.broadcast_id};
}

uint32_t ConfigManager::getId() const
{
    return node_id_;
}
