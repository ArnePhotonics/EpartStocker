#ifndef PARTCREATIONWINDOW_H
#define PARTCREATIONWINDOW_H

#include "digikeywrapper.h"
#include "farnellwrapper.h"
#include <QDialog>
#include <QNetworkAccessManager>

namespace Ui {
    class PartCreationWindow;
}

class PartCreationWindow : public QDialog {
    Q_OBJECT

    public:
    explicit PartCreationWindow(const Settings &settings, DigikeyWrapper &digikey_wrapper, QWidget *parent = nullptr);
    ~PartCreationWindow();

    private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    //  void update_digikey();
    void lookup_received(QMap<QString, QString> data, QStringList additional_text);

    void image_download_finished(QNetworkReply *reply);

    private:
    Ui::PartCreationWindow *ui;

    FarnellWrapper m_farnell_wrapper;
    DigikeyWrapper &m_digikey_wrapper;
    QNetworkAccessManager *m_network_access_manager;
};

#endif // PARTCREATIONWINDOW_H
