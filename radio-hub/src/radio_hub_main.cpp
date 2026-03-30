#include <memory>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

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

    QCommandLineOption port_option(QStringList() << "p"
                                                 << "port",
                                   "Port to bind RADIO-HUB socket.", "port",
                                   "0");
    parser.addOption(port_option);

    parser.process(a);

    if (!parser.isSet(port_option)) {
        qCritical() << "Error: RADIO_HUB PORT is required.";
        parser.showHelp(1);
    }

    const quint16 port =
        static_cast<quint16>(parser.value(port_option).toUInt());

    auto radio_hub = std::make_unique<RadioHub>(port);

    return a.exec();
}
