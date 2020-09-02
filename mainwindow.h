#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"
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

private slots:
  void on_action_ffnen_triggered();

  void on_treeWidget_currentItemChanged(QTreeWidgetItem *current,
                                        QTreeWidgetItem *previous);

  void on_actionneu_triggered();

private:
  Ui::MainWindow *ui;
  std::unique_ptr<PartDataBase> m_data_base;
  void add_categories_recursive(QTreeWidgetItem *root_widget, QString root,
                                PartCategoryTreeNode categories);
  void show_parts(const QMap<int, Part> &parts);
};
#endif // MAINWINDOW_H
