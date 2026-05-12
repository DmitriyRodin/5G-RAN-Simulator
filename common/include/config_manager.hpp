#ifndef CONFIGMANAGER_HPP
#define CONFIGMANAGER_HPP

#include <types.hpp>
#include <yaml-cpp/yaml.h>

#include "settings.hpp"

class ConfigManager
{
public:
    static ConfigManager& instance()
    {
        static ConfigManager inst;
        return inst;
    }

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    bool initializeFromArgs(const QString& description, const EntityType type);

    std::optional<SettingsPack> getSettingsPack() const;
    HubSettings getHubSettings() const;
    std::optional<GnbRuntimeContext> getGnbContext() const;
    std::optional<UeRuntimeContext> getUeContext() const;

    bool load(const std::string& filename);
    const SimulationSettings& getSimulationSettings() const;
    const Paths& getPaths() const;

    std::optional<Point2D> getGnbPosition(const uint32_t id) const;
    std::optional<Point2D> getUePosition(const uint32_t id) const;

private:
    ConfigManager() = default;

    uint32_t node_id_;
    EntityType node_type_;
    std::optional<SettingsPack> pack_;

    HubSettings parseHub(const YAML::Node& node);
    UeSettings parseUe(const YAML::Node& node, const HubSettings hub_set);
    GnbSettings parseGnb(const YAML::Node& root, const HubSettings hub_set);
    SimulationSettings parseSimulation(const YAML::Node& node);
    Positions parsePositions(const YAML::Node& node);
    Paths parsePaths(const YAML::Node& node);

    void validateSection(const YAML::Node& node,
                         const std::string& sectionName);
    uint32_t getId() const;

    GnbSettings getGnbSettings() const;
    UeSettings getUeSettings() const;
    Positions getPositions() const;

    template <typename T>
    T getRequired(const YAML::Node& node, const std::string& key)
    {
        if (!node[key]) {
            throw std::runtime_error(
                "Critical error: missing mandatory parameter: '" + key + "'");
        }
        try {
            return node[key].as<T>();
        } catch (const std::exception& e) {
            throw std::runtime_error("Type conversion error for key '" + key +
                                     "': " + e.what());
        }
    }

    template <typename T>
    std::pair<T, T> getRequiredPair(const YAML::Node& node,
                                    const std::string& key)
    {
        if (!node[key]) {
            throw std::runtime_error(
                "Critical error: missing mandatory parameter: '" + key + "'");
        }
        try {
            return node[key].as<std::pair<T, T>>();
        } catch (const std::exception& e) {
            throw std::runtime_error("Type conversion error for key '" + key +
                                     "': " + e.what());
        }
    }
};

#endif  // CONFIGMANAGER_HPP
