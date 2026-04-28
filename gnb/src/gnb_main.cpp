#include <memory>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>

#include "gnb_logic.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-GNB-Node");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "5G GNB node: Next Generation Base Station");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption id_option(QStringList() << "i"
                                               << "id",
                                 "Unique 32-bit ID for the GNB.", "id");
    parser.addOption(id_option);

    QCommandLineOption port_option(QStringList() << "p"
                                                 << "port",
                                   "Local port to bind GNB socket.", "port",
                                   "0");
    parser.addOption(port_option);

    QCommandLineOption radius_option(QStringList() << "r"
                                                   << "radius",
                                     "Coverage radius in meters.", "meters",
                                     QString::number(1200));

    parser.addOption(radius_option);

    QCommandLineOption x_option("posX", "X coordinate.", "x", "0");
    QCommandLineOption y_option("posY", "Y coordinate.", "y", "0");
    QCommandLineOption radio_hub_addr_option("hub-addr", "Hub IP.", "address",
                                             "127.0.0.1");
    QCommandLineOption radio_hub_port_option("hub-port", "Hub Port.", "port",
                                             "5555");
    QCommandLineOption radio_frame_duration_option(
        "radio-frame-duration", "Radio Frame Duration", "radioframe", "10");
    QCommandLineOption hub_id_option("hub-id", "Hub ID", "hubId", "0");
    QCommandLineOption broadcast_id_option("broadcast-id",
                                           "Broadcast id option",
                                           "BroadcastIdOption", "4294967295");

    parser.addOption(x_option);
    parser.addOption(y_option);
    parser.addOption(radio_hub_addr_option);
    parser.addOption(radio_hub_port_option);
    parser.addOption(radio_frame_duration_option);
    parser.addOption(hub_id_option);
    parser.addOption(broadcast_id_option);

    parser.process(a);

    if (!parser.isSet(id_option)) {
        qCritical() << "Error: gNB ID is required.";
        parser.showHelp(1);
    }

    const uint32_t id = parser.value(id_option).toUInt();
    const double radius = parser.value(radius_option).toDouble();
    const quint16 port =
        static_cast<quint16>(parser.value(port_option).toUInt());
    double pos_X = parser.value(x_option).toDouble();
    const double pos_Y = parser.value(y_option).toDouble();
    QHostAddress radio_hub_address(parser.value(radio_hub_addr_option));
    const quint16 hub_port =
        static_cast<quint16>(parser.value(radio_hub_port_option).toUInt());
    const int radio_frame_duration =
        parser.value(radio_frame_duration_option).toInt();
    const uint32_t hub_id =
        static_cast<uint32_t>(parser.value(hub_id_option).toUInt());
    const uint32_t broadcast_id =
        static_cast<uint32_t>(parser.value(broadcast_id_option).toUInt());

    qInfo().noquote() << QString(
                             "gNB [%1] Initializing. Radius: %2m, Pos: (%3,    "
                             "                       "
                             "%4). Connecting to Hub %5:%6")
                             .arg(id)
                             .arg(radius)
                             .arg(pos_X)
                             .arg(pos_Y);

    auto gnb = std::make_unique<GnbLogic>(id, radius, radio_frame_duration,
                                          hub_id, broadcast_id);
    gnb->setPosition(QPointF{pos_X, pos_Y});

    if (gnb->setupNetwork(port)) {
        gnb->registerAtHub(radio_hub_address, hub_port);
        gnb->run();
    }

    return a.exec();
}
