#include "mainwindow.h"
#include "database.h"
#include "partcreationwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidgetItem>
#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(this)
    , m_digikey_wrapper(m_settings, this) {
    ui->setupUi(this);
    on_action_ffnen_triggered();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_action_ffnen_triggered() {
    m_data_base = std::make_unique<PartDataBase>(m_settings.get_database_path());
    ui->treeWidget->clear();
    m_data_base.get()->create_tree_view_items(ui->treeWidget);
    auto select_item = tree_widget_item_by_categorie_path(ui->treeWidget, "");
    ui->treeWidget->setCurrentItem(select_item);
}

void MainWindow::show_parts(const QMap<int, Part> &parts) {
    ui->treeTable->clear();
    for (const auto &part : parts) {
        auto item = new QTreeWidgetItem(QStringList({QString::number(part.id), part.mpn, part.manufacturer, part.location, part.description}));
        item->setData(0, Qt::UserRole, part.id);
        ui->treeTable->addTopLevelItem(item);
    }
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (current) {
        auto parts = m_data_base->get_parts_by_categorie(current->data(0, Qt::UserRole).toString());
        show_parts(parts);
    }
    (void)previous;
}

void MainWindow::on_actionneu_triggered() {
    PartDetailWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get(), -1);
    part_creation_window.exec();
}

void MainWindow::on_actionEinstellungen_triggered() {
    SettingsWindow settings_window(m_settings, this);
    settings_window.exec();
}

void MainWindow::on_actionSpeichern_triggered() {
    m_data_base.get()->save_to_file();
}

void MainWindow::on_treeTable_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    (void)column;
    PartDetailWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get(), item->data(0, Qt::UserRole).toInt());
    if (part_creation_window.exec()) {
        on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), nullptr);
    }
}
