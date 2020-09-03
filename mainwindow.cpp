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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_action_ffnen_triggered() {
    m_data_base = std::make_unique<PartDataBase>(m_settings.get_database_path());
    ui->treeWidget->clear();
    auto cats = m_data_base->get_category_node("");
    add_categories_recursive(nullptr, "", cats);
    ui->treeWidget->expandAll();
    ui->treeWidget->sortItems(0, Qt::DescendingOrder);
}

void MainWindow::add_categories_recursive(QTreeWidgetItem *root_widget, QString root, PartCategoryTreeNode categorie_node) {
    for (auto &cat : categorie_node.get_children_names()) {
        auto child_cats = m_data_base->get_category_node(root + "/" + cat);
        QTreeWidgetItem *cat_widget = new QTreeWidgetItem(QStringList(cat));
        cat_widget->setData(0, Qt::UserRole, QString(root + "/" + cat));
        add_categories_recursive(cat_widget, root + "/" + cat, child_cats);
        if (root_widget) {
            root_widget->addChild(cat_widget);
        } else {
            ui->treeWidget->addTopLevelItem(cat_widget);
        }
    }
}

void MainWindow::show_parts(const QMap<int, Part> &parts) {
    ui->treeTable->clear();
    for (const auto &part : parts) {
        auto item = new QTreeWidgetItem(QStringList({QString::number(part.id), part.mpn, part.manufacturer, part.location, part.description}));
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
    PartCreationWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get());
    part_creation_window.exec();
}

void MainWindow::on_actionEinstellungen_triggered() {
    SettingsWindow settings_window(m_settings, this);
    settings_window.exec();
}
