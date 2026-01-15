#include "mainwindow.h"

#include <QApplication>

#include "base_entity.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    BaseEntity bs(1, EntityType::GNB);
    BaseEntity ue1(1, EntityType::UE);
    BaseEntity ue2(2, EntityType::UE);

    bs.run();
    ue1.run();
    ue2.run();

    w.show();
    return a.exec();
}
