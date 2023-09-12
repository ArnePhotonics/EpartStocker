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

const int COLUMN_PART_ID = 0;
const int COLUMN_ICON = 0;
const int COLUMN_MPN = 0;
const int COLUMN_MANUFACTURER = 1;
const int COLUMN_LOCATION = 2;
const int COLUMN_QTY = 3;
const int COLUMN_DESCRIPTION = 4;
const int COLUMN_DATESHEET_LINK = 5;

bool MyTreeTableWidgetItem::operator<(const QTreeWidgetItem &other) const {
    int column = treeWidget()->sortColumn();
    if (column == COLUMN_QTY) {
        return text(column).toInt() < other.text(column).toInt();
    }
    return text(column).toLower() < other.text(column).toLower();
}

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

    m_part_list_item_delegate.SetHeight(m_ICON_SIZE);
    ui->treeTable->setItemDelegate(&m_part_list_item_delegate);
    ui->treeTable->setIconSize(QSize(m_ICON_SIZE, m_ICON_SIZE));
    ui->treeTable->headerItem()->setText(COLUMN_MPN, "MPN");
    ui->treeTable->headerItem()->setText(COLUMN_MANUFACTURER, "Manufacturer");
    ui->treeTable->headerItem()->setText(COLUMN_LOCATION, "Location");
    ui->treeTable->headerItem()->setText(COLUMN_QTY, "Qty");
    ui->treeTable->headerItem()->setText(COLUMN_DESCRIPTION, "Description");
    ui->treeTable->headerItem()->setText(COLUMN_DATESHEET_LINK, "Datasheet");

    if (m_settings.get_selected_category().count()) {
        select_category(m_settings.get_selected_category());
    };

    QTimer::singleShot(200, this, &MainWindow::refresh_parts);
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
        const auto &mpn = item->text(COLUMN_MPN);
        const auto &description = item->text(COLUMN_DESCRIPTION);
        const auto &location = item->text(COLUMN_LOCATION);
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

void MainWindow::closeEvent(QCloseEvent *event) {
    (void)event;
}

void MainWindow::refresh_parts() {
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    on_treeWidget_currentItemChanged(current, nullptr);
}

void MainWindow::show_parts(const QMap<int, Part> &parts) {
    ui->treeTable->clear();

    for (const auto &part : parts) {
        QString link = part.datasheet_link.count() > 5 ? "link.." : "";

        auto content = QStringList{"", "", "", "", "", ""};
        content[COLUMN_MPN] = part.mpn;
        content[COLUMN_MANUFACTURER] = part.manufacturer;
        content[COLUMN_LOCATION] = part.location;
        content[COLUMN_QTY] = QString::number(part.qty);
        content[COLUMN_DESCRIPTION] = part.description;
        content[COLUMN_DATESHEET_LINK] = link;
        auto item = new MyTreeTableWidgetItem(content);

        item->setData(COLUMN_PART_ID, Qt::UserRole, part.id);
        item->setData(COLUMN_DATESHEET_LINK, Qt::UserRole, part.datasheet_link);
        item->setData(COLUMN_QTY, Qt::DisplayRole, part.qty);
        if (part.image.isNull()) {
            auto pixmap = QPixmap(m_ICON_SIZE, m_ICON_SIZE);
            pixmap.fill();
            item->setIcon(COLUMN_ICON, QIcon(pixmap));
        } else {
            item->setIcon(COLUMN_ICON, QIcon(part.image));
        }
        ui->treeTable->addTopLevelItem(item);
    }
    auto max_column_widths_percent = QList<float>({25, 15, 5, 5, 45, 5});
    for (int i = 0; i < ui->treeTable->columnCount(); i++)
        ui->treeTable->resizeColumnToContents(i);
    // qDebug() << ui->treeTable->width();
    for (int i = 0; i < ui->treeTable->columnCount(); i++) {
        int max_size = round((max_column_widths_percent[i] / 100.0) * ui->treeTable->width());
        if (ui->treeTable->columnWidth(i) > max_size) {
            ui->treeTable->setColumnWidth(i, max_size);
        }
    }
    filter_parts(ui->filterLineEdit->text());
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

void MainWindow::select_category(QString selected_category) {
    auto select_item = tree_widget_item_by_categorie_path(ui->treeWidget, selected_category);
    ui->treeWidget->setCurrentItem(select_item);
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (current) {
        ui->filterLineEdit->setText("");
        auto currently_selected_category = current->data(0, Qt::UserRole).toString();

        m_settings.set_selected_category(currently_selected_category);
        auto parts = m_data_base->get_parts_by_categorie(currently_selected_category);
        show_parts(parts);
    }
    (void)previous;
}

void MainWindow::on_actionneu_triggered() {}

void MainWindow::open_partcreation_window_for_new_part() {
    QTimer::singleShot(1, this, &MainWindow::on_actionNew_part_triggered);
}

void MainWindow::open_partcreation_window_for_update_part(int part_id) {
    QTimer::singleShot(1, this, [part_id, this]() { //
        this->open_partcreation_window_for_update_part_slot(part_id);
    });
}

void MainWindow::on_treeTable_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    (void)column;
    if (column == COLUMN_DATESHEET_LINK) {
        if (!item->data(COLUMN_DATESHEET_LINK, Qt::UserRole).toString().isEmpty()) {
            QDesktopServices::openUrl(QUrl(item->data(COLUMN_DATESHEET_LINK, Qt::UserRole).toString()));
        }
    } else {
        open_partcreation_window_for_update_part_slot(item->data(COLUMN_PART_ID, Qt::UserRole).toInt());
    }
}

void MainWindow::open_partcreation_window_for_update_part_slot(int part_id) {
    PartDetailWindow part_creation_window(m_settings, m_digikey_wrapper, *m_data_base.get(), part_id);
    if (part_creation_window.exec()) {
        on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), nullptr);
    }
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
