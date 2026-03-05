#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QMap>
#include <QSplitter>
#include <QTableWidget>

#include "controller/simulation_controller.hpp"
#include "map_widget.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<SimulationController> controller,
                        QWidget* parent = nullptr);
    ~MainWindow();

private slots:

    void updateDashboard();

private:
    void setupLayout();
    void setupTables();

    static void setCenteredItem(QTableWidget* table, int row, int col,
                                const QString& text, QColor color);

    Ui::MainWindow* ui;

    std::shared_ptr<SimulationController> sim_controller_;

    MapWidget* map_widget_{nullptr};
    QTableWidget* gnb_table_{nullptr};
    QTableWidget* ue_table_{nullptr};

    QSplitter* main_splitter_{nullptr};
    QSplitter* table_splitter_{nullptr};
};

#endif  // MAINWINDOW_H
