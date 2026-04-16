#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

#include "ue_logic.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("5G-UE-Node");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("5G User Equipment Simulator Node");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption id_option(QStringList() << "i"
                                               << "id",
                                 "Unique 32-bit ID for the UE.", "id");
    parser.addOption(id_option);

    QCommandLineOption port_option(QStringList() << "p"
                                                 << "port",
                                   "Local port to bind UE socket.", "port",
                                   "0");
    parser.addOption(port_option);

    QCommandLineOption x_option("posX", "X coordinate.", "x", "0");
    QCommandLineOption y_option("posY", "Y coordinate.", "y", "0");
    QCommandLineOption radio_hub_addr_option(
        "hub-addr", "IP address or hostname of the Radio Hub.", "address",
        "127.0.0.1");

    parser.addOption(x_option);
    parser.addOption(y_option);
    parser.addOption(radio_hub_addr_option);

    QCommandLineOption radio_hub_port_option(
        "hub-port", "Target port of the Radio Hub.", "port", "5555");
    parser.addOption(radio_hub_port_option);

    parser.process(a);

    if (!parser.isSet(id_option)) {
        qCritical() << "Error: UE ID (-i/--id) is required.";
        parser.showHelp(1);
    }

    const uint32_t id = parser.value(id_option).toUInt();
    const quint16 port =
        static_cast<quint16>(parser.value(port_option).toUInt());
    const double pos_X = parser.value(x_option).toDouble();
    const double pos_Y = parser.value(y_option).toDouble();

    QHostAddress radio_hub_address(parser.value(radio_hub_addr_option));
    const quint16 radio_hub_port =
        static_cast<quint16>(parser.value(radio_hub_port_option).toUInt());

    if (radio_hub_address.isNull()) {
        qCritical() << "Error: Invalid Hub Address"
                    << parser.value(radio_hub_addr_option);
        return -1;
    }

    qInfo().noquote() << QString(
                             "UE [%1] starting on port %2. Pos: (%3, %4). "
                             "Connecting to Hub %5:%6")
                             .arg(id)
                             .arg(port)
                             .arg(pos_X)
                             .arg(pos_Y)
                             .arg(radio_hub_address.toString())
                             .arg(radio_hub_port);

    auto ue = std::make_unique<UeLogic>(id);
    ue->setPosition(QPointF{pos_X, pos_Y});
    if (ue->setupNetwork(port)) {
        ue->registerAtHub(radio_hub_address, radio_hub_port);
        ue->run();
    }

    return a.exec();
}
