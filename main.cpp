#include "mainwindow.h"

#include <QApplication>

#include "common/base_entity.hpp"
#include "gnb/gnb_logic.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    GnbLogic bs(1);
    bs.run();

    w.show();
    return a.exec();
}
