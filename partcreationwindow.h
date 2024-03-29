#ifndef PARTCREATIONWINDOW_H
#define PARTCREATIONWINDOW_H

#include "database.h"
#include "digikeywrapper.h"
#include "farnellwrapper.h"
#include "mouserwrapper.h"
#include "mpnsuggestionwindow.h"
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
    void lookup_received(QMap<QString, QString> data, const QMap<QString, QString> additional_paramters);
    void image_download_finished(QNetworkReply *reply);
    void on_buttonBox_accepted();
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_descriptionLineEdit_textChanged(const QString &arg1);
    void on_datasheetlinkLabel_linkActivated(const QString &link);
    void on_supplierlinkLinkLabel_linkActivated(const QString &link);
    void on_locationLineEdit_textChanged(const QString &arg1);
    void on_mPNLineEdit_textChanged(const QString &arg1);
    void on_qtyManyCheckbox_stateChanged(int arg1);
    void on_continueWithNextPartButton_clicked();
    void show_mpn_proposals_window(QString mpn);

    void on_sKULineEdit_textEdited(const QString &arg1);

    void on_reserved_lineEdit_textChanged(const QString &arg1);

    void on_provisioning_lineEdit_textChanged(const QString &arg1);

    private:
    bool is_valid_for_ok_click();
    void load_ui_from_part();
    Ui::PartCreationWindow *ui;

    void set_ui_datasheetURL(QString URL);
    void set_ui_supplierURL(QString URL);
    void set_ui_additional_parts();
    FarnellWrapper m_farnell_wrapper;
    MouserWrapper m_mouser_wrapper;
    DigikeyWrapper &m_digikey_wrapper;
    QNetworkAccessManager *m_network_access_manager;
    PartDataBase &m_part_data_base;
    int m_part_id;
    QMap<QString, QString> m_additional_parameters;
    bool m_suppress_mpn_kreypress_event = false;
    MPNSuggestionWindow *m_mpn_suggestion_window = nullptr;
};

#endif // PARTCREATIONWINDOW_H
