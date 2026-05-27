#include "mainwindow.h"

#include <QHeaderView>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(std::shared_ptr<SimulationController> controller,
                       QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sim_controller_(controller)
{
    ui->setupUi(this);
    setWindowTitle("5G RAN Simulator");
    resize(1300, 750);

    main_splitter_ = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(main_splitter_);

    table_splitter_ = new QSplitter(Qt::Vertical, main_splitter_);
    table_splitter_->setMinimumWidth(300);

    gnb_table_ = new QTableWidget(0, 4, this);
    gnb_table_->setHorizontalHeaderLabels(
        {"GNB ID", "Position", "Radius", "UE Count"});

    ue_table_ = new QTableWidget(0, 5, this);

    QFontMetrics fm(ue_table_->font());
    int id_width = 40;
    int coord_width = fm.horizontalAdvance("( -888.8, -888.8 )") + 15;

    auto* header_gnb = gnb_table_->horizontalHeader();
    gnb_table_->setColumnWidth(0, id_width);
    gnb_table_->setColumnWidth(1, coord_width);

    header_gnb->setSectionResizeMode(0, QHeaderView::Fixed);
    header_gnb->setSectionResizeMode(1, QHeaderView::Fixed);
    header_gnb->setSectionResizeMode(2, QHeaderView::Stretch);
    header_gnb->setSectionResizeMode(3, QHeaderView::Stretch);

    ue_table_->setHorizontalHeaderLabels(
        {"UE ID", "Position", "RRC State", "RSSI", "Target"});

    ue_table_->setColumnWidth(0, id_width);
    ue_table_->setColumnWidth(1, coord_width);

    auto* header_ue = ue_table_->horizontalHeader();

    header_ue->setSectionResizeMode(0, QHeaderView::Fixed);
    header_ue->setSectionResizeMode(1, QHeaderView::Fixed);
    header_ue->setSectionResizeMode(2, QHeaderView::Fixed);
    ue_table_->setColumnWidth(2, 160);
    header_ue->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header_ue->setSectionResizeMode(4, QHeaderView::Stretch);

    table_splitter_->addWidget(gnb_table_);
    table_splitter_->addWidget(ue_table_);

    map_widget_ = new MapWidget(sim_controller_, this);

    main_splitter_->addWidget(map_widget_);
    main_splitter_->addWidget(table_splitter_);

    table_splitter_->setStretchFactor(0, 1);
    table_splitter_->setStretchFactor(2, 3);

    main_splitter_->setSizes({700, 300});
    table_splitter_->setSizes({250, 550});

    main_splitter_->setCollapsible(0, false);
    main_splitter_->setCollapsible(1, false);

    auto* ui_timer = new QTimer(this);
    connect(ui_timer, &QTimer::timeout, this, &MainWindow::updateDashboard);
    ui_timer->start(200);
}

void MainWindow::updateDashboard()
{
    auto gnbs_info = sim_controller_->getGnbSnapshots();

    gnb_table_->setRowCount(gnbs_info.size());
    gnb_table_->setColumnWidth(2, 100);
    int row = 0;

    auto fillGnbRow = [this](int table_row, uint32_t id, QPointF pos,
                             double radius, uint32_t ue_count) {
        setCenteredItem(gnb_table_, table_row, 0, QString::number(id),
                        Qt::black);
        setCenteredItem(gnb_table_, table_row, 1,
                        QString("%1, %2").arg(pos.x()).arg(pos.y()), Qt::black);
        setCenteredItem(gnb_table_, table_row, 2, QString::number(radius),
                        Qt::black);
        setCenteredItem(gnb_table_, table_row, 3, QString::number(ue_count),
                        Qt::black);
    };

    for (const auto info : gnbs_info) {
        fillGnbRow(row, info.id, info.position, info.data.radius,
                   info.data.connected_ue_count);
        ++row;
    }

    auto fillUeRow = [this](int row, uint32_t id, QPointF pos,
                            bool is_connected, QString state, double rssi,
                            uint32_t target_gnb) {
        setCenteredItem(ue_table_, row, 0, QString::number(id), Qt::black);

        QString pos_str =
            QString("%1, %2").arg(pos.x(), 0, 'f', 1).arg(pos.y(), 0, 'f', 1);
        ue_table_->setItem(row, 1, new QTableWidgetItem(pos_str));

        auto* header_ue = ue_table_->horizontalHeader();
        header_ue->setSectionResizeMode(2, QHeaderView::Fixed);

        QColor state_color = is_connected ? Qt::green : Qt::gray;
        setCenteredItem(ue_table_, row, 2, state, state_color);

        QString rssi_str = QString::number(rssi, 'f', 1);
        setCenteredItem(ue_table_, row, 3, rssi_str, Qt::black);
        setCenteredItem(ue_table_, row, 4, QString::number(target_gnb),
                        Qt::black);
    };

    auto ues_info = sim_controller_->getUeSnapshots();
    ue_table_->setRowCount(ues_info.size());
    row = 0;
    const double RSSI = -70.0;

    for (auto info : ues_info) {
        fillUeRow(row, info.id, info.position, info.data.is_connected,
                  info.data.state, RSSI, info.data.target_gnb);
        ++row;
    }
}

void MainWindow::setupControllerConnections()
{
    connect(sim_controller_.get(), &SimulationController::dataUpdated, this,
            [this]() {
                if (map_widget_) map_widget_->update();
                this->updateDashboard();
            });
}

void MainWindow::setCenteredItem(QTableWidget* table, int row, int col,
                                 const QString& text, QColor color)
{
    auto* item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    if (color.isValid()) {
        item->setForeground(color);
    }

    table->setItem(row, col, item);
}

MainWindow::~MainWindow()
{
    delete ui;
}
