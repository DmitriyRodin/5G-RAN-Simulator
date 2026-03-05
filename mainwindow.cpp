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
    auto gnbs = sim_controller_->getGnbs();
    auto ues = sim_controller_->getUes();

    gnb_table_->setRowCount(gnbs.size());
    gnb_table_->setColumnWidth(2, 100);
    int row = 0;

    for (auto gnb : gnbs) {
        if (!gnb) {
            continue;
        }

        setCenteredItem(gnb_table_, row, 0, QString::number(gnb->getId()),
                        Qt::black);
        setCenteredItem(
            gnb_table_, row, 1,
            QString("%1, %2").arg(gnb->position().x()).arg(gnb->position().y()),
            Qt::black);
        setCenteredItem(gnb_table_, row, 2, QString::number(gnb->getRadius()),
                        Qt::black);
        setCenteredItem(gnb_table_, row, 3,
                        QString::number(gnb->getConnectedUeCount()), Qt::black);
        ++row;
    }

    ue_table_->setRowCount(ues.size());
    row = 0;

    for (auto ue : ues) {
        if (!ue) {
            continue;
        }

        setCenteredItem(ue_table_, row, 0, QString::number(ue->getId()),
                        Qt::black);

        QString pos_str = QString("%1, %2")
                              .arg(ue->position().x(), 0, 'f', 1)
                              .arg(ue->position().y(), 0, 'f', 1);
        ue_table_->setItem(row, 1, new QTableWidgetItem(pos_str));

        auto* header_ue = ue_table_->horizontalHeader();
        header_ue->setSectionResizeMode(2, QHeaderView::Fixed);

        QColor state_color = ue->isConnected() ? Qt::green : Qt::gray;
        setCenteredItem(ue_table_, row, 2, ue->stateString(), state_color);

        QString rssi_str = QString::number(10.0, 'f', 1);
        setCenteredItem(ue_table_, row, 3, rssi_str, Qt::black);
        setCenteredItem(ue_table_, row, 4, QString::number(ue->getTargetGnb()),
                        Qt::black);
        ++row;
    }
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
