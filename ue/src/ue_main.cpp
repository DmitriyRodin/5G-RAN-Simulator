#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

#include "config_manager.hpp"
#include "serializer_factory.hpp"
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

    auto context = ConfigManager::instance().getUeContext();

    if (!context) {
        return EXIT_FAILURE;
    }

    auto serializer_type = ConfigManager::instance().getSerializerType();
    auto serializer = SerializerFactory::create(serializer_type);

    auto ue = std::make_unique<UeLogic>(context->id, context->set,
                                        std::move(serializer));

    ue->setPosition(QPointF{context->pos.X, context->pos.Y});

    if (!ue->setupNetwork(NetworkParam::EPHEMERAL_PORT)) {
        return EXIT_FAILURE;
    }

    ue->registerAtHub();
    ue->run();

    return a.exec();
}
