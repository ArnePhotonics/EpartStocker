#include "mainwindow.h"
#include "database.h"
#include "ui_mainwindow.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_action_ffnen_triggered() {
  m_data_base = std::make_unique<PartDataBase>(
      QString("C:/Users/ak/entwicklung/qt/elektronik_lager/elektronik_lager/"
              "database.json"));
}
