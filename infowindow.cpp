#include "infowindow.h"
#include "ui_infowindow.h"
#include "vc.h"

InfoWindow::InfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoWindow) {
    ui->setupUi(this);
    ui->githashLabel->setText("Git hash:" + QString::number(GITHASH, 16));
    ui->git_dateLabel->setText("Git date:" + QString(GITDATE));
    ui->gitbranchLabel->setText("git branch:" + QString(GITBRANCH));
}

InfoWindow::~InfoWindow() {
    delete ui;
}
