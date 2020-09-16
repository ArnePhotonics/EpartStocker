#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"
#include "digikeywrapper.h"
#include "settingswindow.h"
#include <QCloseEvent>
#include <QItemDelegate>
#include <QMainWindow>
#include <QStringList>
#include <QTreeWidgetItem>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class ItemDelegate : public QItemDelegate {
    private:
    int m_height;

    public:
    ItemDelegate(QObject *poParent = Q_NULLPTR, int height = -1)
        : QItemDelegate(poParent)
        , m_height(height) {}

    void SetHeight(int height) {
        m_height = height;
    }

    // Use this for setting tree item height.
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QSize oSize = QItemDelegate::sizeHint(option, index);

        if (m_height != -1) {
            // Set tree item height.
            oSize.setHeight(m_height);
        }

        return oSize;
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void open_partcreation_window_for_new_part();
    void open_partcreation_window_for_update_part(int part_id);
    private slots:
    void on_action_ffnen_triggered();
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_actionneu_triggered();
    void on_actionEinstellungen_triggered();
    void on_treeTable_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_actioninfo_triggered();
    void on_actionNew_part_triggered();
    void on_filterLineEdit_textChanged(const QString &arg1);
    void open_partcreation_window_for_update_part_slot(int part_id);

    private:
    Ui::MainWindow *ui;
    std::unique_ptr<PartDataBase> m_data_base;
    Settings m_settings;
    void show_parts(const QMap<int, Part> &parts);
    DigikeyWrapper m_digikey_wrapper;
    void open_database();
    void filter_parts(QString filter);
    ItemDelegate m_part_list_item_delegate;
    const int m_ICON_SIZE = 50;
    void closeEvent(QCloseEvent *event) override;
    QString m_currently_selected_category;
    void select_category(QString selected_category);
};
#endif // MAINWINDOW_H
