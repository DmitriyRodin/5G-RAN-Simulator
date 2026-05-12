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

    auto context = ConfigManager::instance().getGnbContext();

    if (!context) {
        return EXIT_FAILURE;
    }

    auto gnb = std::make_unique<GnbLogic>(context->id, context->set);

    gnb->setPosition(QPointF{context->pos.X, context->pos.Y});

    if (!gnb->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
        return EXIT_FAILURE;
    }

    gnb->registerAtHub();
    gnb->run();

    return a.exec();
}
