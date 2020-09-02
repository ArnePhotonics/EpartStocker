#include "partcreationwindow.h"
#include "barcodescaninputwindow.h"
#include "ui_partcreationwindow.h"
#include <QtNetworkAuth>

PartCreationWindow::PartCreationWindow(const Settings &settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PartCreationWindow)
    , m_digikey_wrapper(settings) {
    ui->setupUi(this);
    connect(&m_digikey_wrapper, &DigikeyWrapper::authenticated, this, &PartCreationWindow::update_digikey);
}

PartCreationWindow::~PartCreationWindow() {
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
    m_digikey_wrapper.grant();
}

void PartCreationWindow::update_digikey() {
    m_digikey_wrapper.subscribeToLiveUpdates();
}
