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

        HubSettings hub_set = parseHub(root);
        UeSettings ue_set = parseUe(root, hub_set);
        GnbSettings gnb_set = parseGnb(root, hub_set);
        SimulationSettings sim = parseSimulation(root);
        Paths paths = parsePaths(root);
        Positions pos = parsePositions(root);

        pack_.emplace(std::move(hub_set), std::move(ue_set), std::move(gnb_set),
                      std::move(sim), std::move(pos), std::move(paths));

        qDebug() << "[ConfigManager]: Configuration successfully mapped to "
                    "structures";
        return true;

    } catch (const std::exception& e) {
        qWarning() << "[ConfigManager]: Error parsing YAML:" << e.what();
        return false;
    }
}

HubSettings ConfigManager::parseHub(const YAML::Node& node)
{
    validateSection(node, "hub_settings");

    const auto hub_node = node["hub_settings"];

    const uint32_t id = hub_node["id"].as<uint32_t>(0);
    const uint16_t port = getRequired<uint16_t>(hub_node, "port");

    const uint32_t broadcast_id =
        hub_node["broadcast_id"].as<uint32_t>(0xFFFFFFFF);

    const auto virt_pos_pair =
        getRequiredPair<double>(hub_node, "virtual_position");
    const Point2D virt_pos = {virt_pos_pair.first, virt_pos_pair.second};

    const std::string address = getRequired<std::string>(hub_node, "address");

    HubSettings hub_set(port, id, broadcast_id, virt_pos, address);

    qDebug() << "[ConfigManager]: Network settings parsed successfully";

    return hub_set;
}

UeSettings ConfigManager::parseUe(const YAML::Node& node,
                                  const HubSettings hub_set)
{
    validateSection(node, "ue_settings");
    const auto ue_node = node["ue_settings"];
    validateSection(ue_node, "node_settings");
    const auto node_set = ue_node["node_settings"];
    validateSection(node_set, "cell");
    const auto cell_node = node_set["cell"];
    const uint16_t tac = getRequired<uint16_t>(cell_node, "tracking_area_code");

    validateSection(node_set, "radio");
    const auto radio_node = node_set["radio"];
    const int rfd = getRequired<int>(radio_node, "radio_frame_duration");
    const double tx_power_db = getRequired<double>(radio_node, "tx_power_db");

    UeSettings ue_set{hub_set, RadioSettings{rfd, tx_power_db}, Cell{tac}};

    return ue_set;
}

GnbSettings ConfigManager::parseGnb(const YAML::Node& root,
                                    const HubSettings hub_set)
{
    validateSection(root, "gnb_settings");
    const auto node = root["gnb_settings"];
    const double radius = getRequired<double>(node, "radius");
    qDebug() << "parseGnb: radius = " << radius;
    const auto node_set = node["node_settings"];
    validateSection(node_set, "cell");
    const auto cell_node = node_set["cell"];
    const uint16_t tac = getRequired<uint16_t>(cell_node, "tracking_area_code");

    validateSection(node_set, "radio");
    const auto radio_node = node_set["radio"];
    const int rfd = getRequired<int>(radio_node, "radio_frame_duration");
    const double tx_power_db = getRequired<double>(radio_node, "tx_power_db");

    GnbSettings gnb_set{hub_set, RadioSettings{rfd, tx_power_db}, Cell{tac},
                        radius};

    return gnb_set;
}

SimulationSettings ConfigManager::parseSimulation(const YAML::Node& node)
{
    validateSection(node, "simulation");
    const auto sim_node = node["simulation"];

    const uint8_t raw_deploy_mode =
        getRequired<uint8_t>(sim_node, "deploy_mode");
    DeployMode deploy_mode;

    if (raw_deploy_mode == 0) {
        deploy_mode = DeployMode::Monolithic;
    } else {
        deploy_mode = DeployMode::Distributed;
    }

    const uint32_t gnb_count = getRequired<uint32_t>(sim_node, "gnb_count");
    const uint32_t ue_count = getRequired<uint32_t>(sim_node, "ue_count");

    const uint32_t gnb_id_start =
        getRequired<uint32_t>(sim_node, "gnb_id_start");
    const uint32_t ue_id_start = getRequired<uint32_t>(sim_node, "ue_id_start");

    qDebug() << "[ConfigManager]: Simulation settings parsed successfully";
    return SimulationSettings{deploy_mode, gnb_count, ue_count, gnb_id_start,
                              ue_id_start};
}

Positions ConfigManager::parsePositions(const YAML::Node& node)
{
    validateSection(node, "positions");
    const auto pos_node = node["positions"];
    validateSection(pos_node, "gnb_positions_list");
    validateSection(pos_node, "ue_positions_list");

    Positions positions;

    if (pos_node["gnb_positions_list"]) {
        for (const auto& item : pos_node["gnb_positions_list"]) {
            uint32_t id = item["id"].as<uint32_t>();
            double x = item["pos"][0].as<double>();
            double y = item["pos"][1].as<double>();
            positions.gnbs[id] = {x, y};
        }
    }

    if (pos_node["ue_positions_list"]) {
        for (const auto& node : pos_node["ue_positions_list"]) {
            uint32_t id = node["id"].as<uint32_t>();
            double x = node["pos"][0].as<double>();
            double y = node["pos"][1].as<double>();
            positions.ues[id] = {x, y};
        }
    }

    return positions;
}

Paths ConfigManager::parsePaths(const YAML::Node& node)
{
    validateSection(node, "paths");
    auto build_node = node["paths"];

    Paths paths;

    try {
        paths.build_dir = getRequired<std::string>(build_node, "build_dir");

        qDebug() << "[ConfigManager]: Paths settings parsed successfully. "
                    "Build directory:"
                 << QString::fromStdString(paths.build_dir);
        return paths;

    } catch (const std::exception& e) {
        throw std::runtime_error("Paths config error: " +
                                 std::string(e.what()));
    }
}

const SimulationSettings& ConfigManager::getSimulationSettings() const
{
    return pack_->sim;
}

const Paths& ConfigManager::getPaths() const
{
    return pack_->paths;
}

std::optional<Point2D> ConfigManager::getGnbPosition(const uint32_t id) const
{
    auto it = pack_->positions.gnbs.find(id);
    if (it == pack_->positions.gnbs.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Point2D> ConfigManager::getUePosition(const uint32_t id) const
{
    auto it = pack_->positions.ues.find(id);
    if (it == pack_->positions.ues.end()) {
        return std::nullopt;
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

std::optional<SettingsPack> ConfigManager::getSettingsPack() const
{
    return pack_;
}

HubSettings ConfigManager::getHubSettings() const
{
    return pack_->hub;
}

GnbSettings ConfigManager::getGnbSettings() const
{
    return pack_->gnb;
}

UeSettings ConfigManager::getUeSettings() const
{
    return pack_->ue;
}

Positions ConfigManager::getPositions() const
{
    return pack_->positions;
}

uint32_t ConfigManager::getId() const
{
    return node_id_;
}

std::optional<GnbRuntimeContext> ConfigManager::getGnbContext() const
{
    const uint32_t node_id = getId();

    const auto pos = getGnbPosition(node_id);
    if (pos) {
        return GnbRuntimeContext{node_id, getGnbSettings(), pos.value()};
    }

    qCritical() << "[ConfigManager]: CRITICAL - No position found for GNB ID:"
                << node_id;
    return std::nullopt;
}

std::optional<UeRuntimeContext> ConfigManager::getUeContext() const
{
    const uint32_t node_id = getId();

    const auto pos = getUePosition(node_id);
    if (pos) {
        return UeRuntimeContext{node_id, getUeSettings(), pos.value()};
    }

    qCritical() << "[ConfigManager]: CRITICAL - No position found for UE ID:"
                << node_id;
    return std::nullopt;
}
