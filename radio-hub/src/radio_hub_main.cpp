#include <memory>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>

#include "config_manager.hpp"
#include "radio_hub.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-RADIO_HUB-Node");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "5G RADIO_HUB node: RADIO SPACE SIMULATOR");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOption({"c", "config"},
                                    "Path to the configuration file.", "file",
                                    "config.yaml");

    parser.addOption(configOption);
    parser.process(a);

    QString configPath = parser.value(configOption);

    QFileInfo checkFile(configPath);
    if (!checkFile.exists() || !checkFile.isFile()) {
        qCritical() << "CRITICAL ERROR: Configuration file not found at:"
                    << checkFile.absoluteFilePath();
        return 1;
    }

    const std::string final_path = checkFile.absoluteFilePath().toStdString();

    if (!ConfigManager::instance().load(final_path)) {
        return 1;
    }

    qDebug() << "SUCCESS: Config loaded from"
             << QString::fromStdString(final_path);

    const auto net = ConfigManager::instance().getNetworkSettings();
    const auto sim = ConfigManager::instance().getSimulationSettings();

    auto radio_hub = std::make_unique<RadioHub>(
        net.hub_port, net.hub_id, net.broadcast_id,
        QPointF(sim.hub_virtual_position.X, sim.hub_virtual_position.Y));

    return a.exec();
}
