#include "mainwindow.h"
#include "database.h"
#include "partcreationwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrl>
#include <infowindow.h>
#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(this)
    , m_digikey_wrapper(m_settings, this) {
    ui->setupUi(this);
    //qDebug() << QCoreApplication::applicationFilePath();
    QString exe_name = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    QString install_source = m_settings.get_installation_source_path().trimmed();
    if (!install_source.endsWith(exe_name)) {
        if (install_source.contains(QRegularExpression("[\\/\\\\&]"))) { //ends with \ or  /
            install_source += QDir::separator();
        }
        install_source += exe_name;
    }
    // qDebug() << install_source;
    if (QFile::exists(install_source)) {
        if (QFileInfo(install_source).lastModified() > QFileInfo(QCoreApplication::applicationFilePath()).lastModified()) {
            QMessageBox::information(this, tr("Update needed"),
                                     tr("There is a new version under \n %1 \n please update to this one.").arg(m_settings.get_installation_source_path()));
            exit(0);
            return;
        }
    }
    open_database();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
    setWindowState(Qt::WindowMaximized);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_filterLineEdit_textChanged(const QString &arg1) {
    filter_parts(arg1);
}

void MainWindow::filter_parts(QString filter) {
    filter.replace("\\", "\\\\");
    filter.replace("$", "\\$");
    filter.replace("+", "\\+");
    filter.replace("?", "\\?");
    filter.replace(".", "\\.");
    filter.replace("(", "\\(");
    filter.replace(")", "\\)");
    filter.replace("]", "\\]");
    filter.replace("[", "\\[");
    filter.replace("]", "\\]");
    QRegularExpression filter_regex(filter, QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < ui->treeTable->topLevelItemCount(); i++) {
        auto item = ui->treeTable->topLevelItem(i);
        const auto &mpn = item->text(0);
        const auto &description = item->text(2);
        const auto &location = item->text(3);
        QStringList sl{mpn, description, location};
        bool matches = false;
        for (const auto &s : sl) {
            if (s.contains(filter_regex)) {
                matches = true;
            }
        }
        item->setHidden(!matches);
    }
}

void MainWindow::show_parts(const QMap<int, Part> &parts) {
    ui->treeTable->clear();
    for (const auto &part : parts) {
        QString link = part.datasheet_link.contains("pdf") ? "link.." : "";
        auto item = new QTreeWidgetItem(QStringList{part.mpn, part.manufacturer, part.location, part.description, link});
        item->setData(0, Qt::UserRole, part.id);
        item->setData(4, Qt::UserRole, part.datasheet_link);
        ui->treeTable->addTopLevelItem(item);
    }
    for (int i = 0; i < ui->treeTable->columnCount(); i++)
        ui->treeTable->resizeColumnToContents(i);
    filter_parts(ui->filterLineEdit->text());
}

void MainWindow::on_treeTable_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    (void)column;
    if (column == 4) {
        if (!item->data(4, Qt::UserRole).toString().isEmpty()) {
            QDesktopServices::openUrl(QUrl(item->data(4, Qt::UserRole).toString()));
        }
    } else {
        PartDetailWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get(), item->data(0, Qt::UserRole).toInt());
        if (part_creation_window.exec()) {
            on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), nullptr);
        }
    }
}

void MainWindow::open_database() {
    try {
        m_data_base = std::make_unique<PartDataBase>(m_settings.get_database_path());
        ui->treeWidget->clear();
        m_data_base.get()->create_tree_view_items(ui->treeWidget);
        auto select_item = tree_widget_item_by_categorie_path(ui->treeWidget, "");
        ui->treeWidget->setCurrentItem(select_item);
    } catch (DataBaseException &e) {
        QMessageBox::warning(this, tr("Database error"), tr("Error while opening database file: %1").arg(e.what()));
    }
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (current) {
        ui->filterLineEdit->setText("");
        auto parts = m_data_base->get_parts_by_categorie(current->data(0, Qt::UserRole).toString());
        show_parts(parts);
    }
    (void)previous;
}

void MainWindow::on_actionneu_triggered() {}

void MainWindow::open_partcreation_window_for_new_part() {
    QTimer::singleShot(1, this, &MainWindow::on_actionNew_part_triggered);
}

void MainWindow::on_actionNew_part_triggered() {
    PartDetailWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get(), -1, this);
    part_creation_window.exec();
}

void MainWindow::on_actionEinstellungen_triggered() {
    SettingsWindow settings_window(m_settings, this);
    settings_window.exec();
}

void MainWindow::on_actioninfo_triggered() {
    InfoWindow info(this);
    info.exec();
}

void MainWindow::on_action_ffnen_triggered() {
    open_database();
}
