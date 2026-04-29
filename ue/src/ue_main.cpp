#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

#include "config_manager.hpp"
#include "ue_logic.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-UE-Node");
    QCoreApplication::setApplicationVersion("1.0");

    if (!ConfigManager::instance().initializeFromArgs(
            "5G User Equipment Simulator Node", EntityType::UE)) {
        return EXIT_FAILURE;
    }

    const auto set = ConfigManager::instance().getUeSettings();
    const auto node_id = ConfigManager::instance().getId();

    auto ue = std::make_unique<UeLogic>(node_id, set);
    try {
        const auto position = ConfigManager::instance().getUePosition(node_id);
        ue->setPosition(QPointF{position.X, position.Y});
    } catch (const std::exception& e) {
        qWarning() << "Error: " << e.what();
        return EXIT_FAILURE;
    }

    if (ue->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
        ue->registerAtHub(QHostAddress::LocalHost, set.hub_port);
        ue->run();
    }

    return a.exec();
}
