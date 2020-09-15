#include "mpnsuggestionwindow.h"
#include "ui_mpnsuggestionwindow.h"

MPNSuggestionWindow::MPNSuggestionWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MPNSuggestionWindow) {
    ui->setupUi(this);
}

MPNSuggestionWindow::~MPNSuggestionWindow() {
    delete ui;
}

void MPNSuggestionWindow::show_suggestions(QPoint position, int width_, QStringList mpns) {
    QString t = "Already existing similar parts:\n<table>";
    for (auto s : mpns) {
        t = t + QString("<tr><td>" + s + "</td></tr>");
    }
    t = t + "</table>";
    ui->suggestionLabel->setText(t);
    setWindowFlag(Qt::Tool);
    setFixedWidth(width_);
    show();
    move(position);
    //Qt::Tool
    //setWindowFlag(Qt::WindowStaysOnTopHint);
}
