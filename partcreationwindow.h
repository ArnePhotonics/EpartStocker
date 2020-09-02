#ifndef PARTCREATIONWINDOW_H
#define PARTCREATIONWINDOW_H

#include "digikeywrapper.h"
#include "octopartinterface.h"
#include <QDialog>

namespace Ui {
    class PartCreationWindow;
}

class PartCreationWindow : public QDialog {
    Q_OBJECT

    public:
    explicit PartCreationWindow(QWidget *parent = nullptr);
    ~PartCreationWindow();

    private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void update_digikey();

    private:
    Ui::PartCreationWindow *ui;
    DigikeyWrapper digikeyWrapper;
};

#endif // PARTCREATIONWINDOW_H
