#include <memory>

#include <QCoreApplication>
#include <QDebug>

#include "config_manager.hpp"
#include "radio_hub.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-RADIO_HUB-Node");
    QCoreApplication::setApplicationVersion("1.0");

    if (!ConfigManager::instance().initializeFromArgs(
            "5G RADIO_HUB node: RADIO SPACE SIMULATOR", EntityType::RadioHub)) {
        return EXIT_FAILURE;
    }

    const auto set = ConfigManager::instance().getHubSettings();

    auto radio_hub = std::make_unique<RadioHub>(set);

    return a.exec();
}
