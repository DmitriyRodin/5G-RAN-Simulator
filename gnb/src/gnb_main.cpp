#include <memory>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>

#include "config_manager.hpp"
#include "gnb_logic.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-GNB-Node");
    QCoreApplication::setApplicationVersion("1.0");

    if (!ConfigManager::instance().initializeFromArgs(
            "5G GNB node: Next Generation Base Station", EntityType::GNB)) {
        return EXIT_FAILURE;
    }

    const auto set = ConfigManager::instance().getGnbSettings();
    const auto node_id = ConfigManager::instance().getId();

    auto gnb = std::make_unique<GnbLogic>(node_id, set);

    try {
        const auto position = ConfigManager::instance().getGnbPosition(node_id);
        gnb->setPosition(QPointF{position.X, position.Y});
    } catch (const std::exception& e) {
        qWarning() << "Error: " << e.what();
        return EXIT_FAILURE;
    }

    if (gnb->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
        gnb->registerAtHub(QHostAddress::LocalHost, set.hub_port);
        gnb->run();
    }

    return a.exec();
}
