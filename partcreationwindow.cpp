#include "partcreationwindow.h"
#include "barcodescaninputwindow.h"
#include "ui_partcreationwindow.h"
#include <QPixmap>
#include <QtGlobal>
#include <QtNetworkAuth>

PartCreationWindow::PartCreationWindow(const Settings &settings, DigikeyWrapper &digikey_wrapper, PartDataBase &part_data_base, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PartCreationWindow)
    , m_farnell_wrapper(settings, parent)
    , m_digikey_wrapper(digikey_wrapper)
    , m_part_data_base(part_data_base) {
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

void PartCreationWindow::on_buttonBox_accepted() {
    Part new_part;
    new_part.id = -1;
    new_part.datasheet_link = ui->datasheetLinkLineEdit->text();
    new_part.sku = ui->sKULineEdit->text();
    new_part.supplier = ui->supplierLineEdit->text();
    new_part.mpn = ui->mPNLineEdit->text();
    new_part.manufacturer = ui->manufacturerLineEdit->text();
    // new_part.category = ui->datasheetLinkLineEdit->text();
    new_part.description = ui->descriptionLineEdit->text();
    new_part.location = ui->locationLineEdit->text();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    new_part.image = ui->imageLabel->pixmap(Qt::ReturnByValueConstant::ReturnByValue);
#else
    const QPixmap *pixmap = ui->imageLabel->pixmap();
    if (pixmap) {
        new_part.image = QPixmap(*pixmap);
    }
#endif

    new_part.qty = ui->qtySpinBox->value();
    if (ui->qtyManyCheckbox->isChecked()) {
        new_part.qty = -1000;
    }
    m_part_data_base.insert_new_part(new_part);
}
