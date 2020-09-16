#include "mpnsuggestionwindow.h"
#include "mainwindow.h"
#include "ui_mpnsuggestionwindow.h"
#include <QDebug>
#include <partcreationwindow.h>

MPNSuggestionWindow::MPNSuggestionWindow(QWidget *parent, QWidget *mainwindow_parent)
    : QDialog(parent)
    , ui(new Ui::MPNSuggestionWindow)
    , m_mainwindow_parent(mainwindow_parent) {
    ui->setupUi(this);
}

MPNSuggestionWindow::~MPNSuggestionWindow() {
    delete ui;
}

void MPNSuggestionWindow::show_suggestions(QPoint position, int width_, QList<SuggestionPartInfo> parts) {
    QString t = "<div>Already existing similar parts:</div><table>";
    for (const auto &part : parts) {
        t = t + QString("<tr><td><a href=" + QString::number(part.part_id) + ">" + part.mpn + "</a></td><td>" + part.location + "</td></tr>");
    }
    t = t + "</table>";
    ui->suggestionLabel->setText(t);
    setWindowFlag(Qt::Tool);
    setFixedWidth(width_);
    show();
    move(position);
    //Qt::Tool
}

void MPNSuggestionWindow::on_suggestionLabel_linkActivated(const QString &link) {
    qDebug() << link;
    MainWindow *main_window = dynamic_cast<MainWindow *>(m_mainwindow_parent);
    if (main_window) {
        PartDetailWindow *detail_win = dynamic_cast<PartDetailWindow *>(parent());
        if (detail_win) {
            detail_win->close();
        }

        main_window->open_partcreation_window_for_update_part(link.toInt());
        close();
    }
}
