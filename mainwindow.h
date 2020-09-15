#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"
#include "digikeywrapper.h"
#include "settingswindow.h"
#include <QMainWindow>
#include <QStringList>
#include <QTreeWidgetItem>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void open_partcreation_window_for_new_part();
    private slots:
    void on_action_ffnen_triggered();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionneu_triggered();

    void on_actionEinstellungen_triggered();

    void on_treeTable_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_actioninfo_triggered();

    void on_actionNew_part_triggered();

    void on_filterLineEdit_textChanged(const QString &arg1);

    private:
    Ui::MainWindow *ui;
    std::unique_ptr<PartDataBase> m_data_base;
    Settings m_settings;
    void show_parts(const QMap<int, Part> &parts);
    DigikeyWrapper m_digikey_wrapper;
    void open_database();
    void filter_parts(QString filter);
};
#endif // MAINWINDOW_H
