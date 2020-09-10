#ifndef PARTCREATIONWINDOW_H
#define PARTCREATIONWINDOW_H

#include "database.h"
#include "digikeywrapper.h"
#include "farnellwrapper.h"
#include <QDialog>
#include <QNetworkAccessManager>

namespace Ui {
    class PartCreationWindow;
}

class PartDetailWindow : public QDialog {
    Q_OBJECT

    public:
    explicit PartDetailWindow(const Settings &settings, DigikeyWrapper &digikey_wrapper, PartDataBase &part_data_base, int part_id, QWidget *parent = nullptr);
    ~PartDetailWindow();

    private slots:
    void on_scanbarcodeButton_clicked();
    void on_lookupButton_clicked();
    void lookup_received(QMap<QString, QString> data, QStringList additional_text);
    void image_download_finished(QNetworkReply *reply);
    void on_buttonBox_accepted();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_descriptionLineEdit_textChanged(const QString &arg1);

    void on_datasheetlinkLabel_linkActivated(const QString &link);

    void on_supplierlinkLinkLabel_linkActivated(const QString &link);

    private:
    bool is_valid_for_ok_click();
    void load_ui_from_part();
    Ui::PartCreationWindow *ui;

    void set_ui_datasheetURL(QString URL);
    void set_ui_supplierURL(QString URL);
    FarnellWrapper m_farnell_wrapper;
    DigikeyWrapper &m_digikey_wrapper;
    QNetworkAccessManager *m_network_access_manager;
    PartDataBase &m_part_data_base;
    int m_part_id;
};

#endif // PARTCREATIONWINDOW_H
