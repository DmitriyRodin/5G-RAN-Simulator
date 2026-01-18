#include "mainwindow.h"

#include <QApplication>

#include "common/base_entity.hpp"
#include "gnb/gnb_logic.hpp"
#include "ue/src/ue_logic.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    GnbLogic bs(1);
    bs.run();
    UeLogic ue1(1);

    w.show();
    return a.exec();
}
