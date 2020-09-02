#include "partcreationwindow.h"
#include "barcodescaninputwindow.h"
#include "ui_partcreationwindow.h"
#include <QtNetworkAuth>

PartCreationWindow::PartCreationWindow(const Settings &settings, DigikeyWrapper &digikey_wrapper, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PartCreationWindow)
    , m_farnell_wrapper(settings, parent)
    , m_digikey_wrapper(digikey_wrapper) {
    ui->setupUi(this);
    m_network_access_manager = new QNetworkAccessManager(this);
    connect(m_network_access_manager, &QNetworkAccessManager::finished, this, &PartCreationWindow::image_download_finished);
    connect(&m_digikey_wrapper, &DigikeyWrapper::got_data, this, &PartCreationWindow::lookup_received);
    connect(&m_farnell_wrapper, &FarnellWrapper::got_data, this, &PartCreationWindow::lookup_received);
}

PartCreationWindow::~PartCreationWindow() {
    delete m_network_access_manager;
    delete ui;
}

void PartCreationWindow::on_pushButton_clicked() {
    BarcodeScanInputWindow scan_input;
    if (scan_input.exec()) {
        auto scan_result = scan_input.get_parsed_fields();
        for (auto s : scan_result.keys()) {
            if (s == "mpn") {
                ui->mPNLineEdit->setText(scan_result[s]);
            } else if (s == "sku") {
                ui->sKULineEdit->setText(scan_result[s]);
            } else if (s == "supplier") {
                ui->supplierLineEdit->setText(scan_result[s]);
            } else if (s == "qty") {
                ui->qtySpinBox->setValue(scan_result[s].toInt());
                ui->qtyManyCheckbox->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void PartCreationWindow::on_pushButton_2_clicked() {
    //    m_digikey_wrapper.grant();
    auto supplier = Supplier(ui->sKULineEdit->text());
    switch (supplier.type()) {
        case Supplier::Digikey:
            m_digikey_wrapper.query(ui->sKULineEdit->text());
            break;
        case Supplier::Farnell:
            m_farnell_wrapper.query(ui->sKULineEdit->text());
            break;
        case Supplier::None:
            break;
    }
}

void PartCreationWindow::image_download_finished(QNetworkReply *reply) {
    QPixmap pm;
    auto data = reply->readAll();
    pm.loadFromData(data);
    pm = pm.scaled(100, 100, Qt::KeepAspectRatio);
    ui->imageLabel->setPixmap(pm);
}

void PartCreationWindow::lookup_received(QMap<QString, QString> data, QStringList additional_text) {
    (void)additional_text;
    ui->descriptionLineEdit->setText(data["description"]);
    ui->manufacturerLineEdit->setText(data["manufacturer"]);
    ui->mPNLineEdit->setText(data["mpn"]);
    ui->supplierLineEdit->setText(data["supplier"]);
    ui->datasheetLinkLineEdit->setText(data["datasheet_url"]);
    if (data["image_url"] != "") {
        QString url_str = data["image_url"];
        QUrl url(url_str);
        if (data["supplier"] == Supplier(Supplier::Digikey).toStr()) {
            url.setScheme("https");
        }
        qDebug() << url_str;
        QNetworkRequest request(url);
        m_network_access_manager->get(request);
    }
}
