#include "mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    QLoggingCategory::setFilterRules("qt.networkauth.*=true");
    w.show();
    return a.exec();
}
