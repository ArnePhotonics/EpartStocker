#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"
#include <QMainWindow>
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

private:
  Ui::MainWindow *ui;
  std::unique_ptr<PartDataBase> m_data_base;
};
#endif // MAINWINDOW_H
