#ifndef CONFIGMANAGER_HPP
#define CONFIGMANAGER_HPP

#include <utility>
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

    bool load(const std::string& filename);

    const NetworkSettings& getNetworkSettings() const;
    const SimulationSettings& getSimulationSettings() const;
    const Paths& getPaths() const;

private:
    ConfigManager() = default;

    NetworkSettings network_settings_;
    SimulationSettings simulation_settings_;
    Paths paths_;

    void parseNetwork(const YAML::Node& node);
    void parseSimulation(const YAML::Node& node);
    void parsePaths(const YAML::Node& node);

    void validateSection(const YAML::Node& node,
                         const std::string& sectionName);

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

    void getRequiredGnbPositions(const YAML::Node& node);
};

#endif  // CONFIGMANAGER_HPP
