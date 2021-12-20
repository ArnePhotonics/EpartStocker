#include "barcodescaninputwindow.h"
#include "ui_barcodescaninputwindow.h"

BarcodeScanInputWindow::BarcodeScanInputWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BarcodeScanInputWindow) {
    ui->setupUi(this);
}

BarcodeScanInputWindow::~BarcodeScanInputWindow() {
    delete ui;
}

QMap<QString, QString> BarcodeScanInputWindow::get_parsed_fields() {
    QMap<QString, QString> result;
    //result["supplier"] = m_supplier.toStr();
    //if ((m_supplier.type() == Supplier::Digikey) || (m_supplier.type() == Supplier::Mouser))
    if (ui->listWidget->count() > 1) {
        //2DBarcode
        result["supplier"] = Supplier(Supplier::Mouser).toStr();
        for (int i = 0; i < ui->listWidget->count(); i++) {
            auto s = ui->listWidget->item(i)->text();
            if (s.startsWith("P")) {
                result["sku"] = s.mid(1);
                result["supplier"] = Supplier(Supplier::Digikey).toStr();
            } else if (s.startsWith("1P")) {
                result["mpn"] = s.mid(2);
            } else if (s.startsWith("3P")) {
                result["sku"] = s.mid(2);
                result["supplier"] = Supplier(Supplier::Farnell).toStr();
            } else if (s.startsWith("Q")) {
                result["qty"] = s.mid(1);
            }
        }
    } else {
        for (int i = 0; i < ui->listWidget->count(); i++) {
            auto s = ui->listWidget->item(i)->text();
            QRegularExpression regex("\\b\\d{7}\\b");
            if (regex.match(s).hasMatch()) {
                result["supplier"] = Supplier(Supplier::Farnell).toStr();
                result["sku"] = s;
            }
        }
    }
    return result;
}

void BarcodeScanInputWindow::on_lineEdit_returnPressed() {
    if (ui->lineEdit->text() != "") {
        on_btnOK_clicked();
    }
}

void BarcodeScanInputWindow::on_btnOK_clicked() {
    if (ui->lineEdit->text().contains('')) {
        QStringList items = ui->lineEdit->text().split('');
        //  m_supplier = Supplier::Mouser;
        ui->listWidget->addItems(items);

        for (int i = 0; i < ui->listWidget->count(); i++) {
            auto s = ui->listWidget->item(i)->text();
            if (s.startsWith("P")) { //seenms mouser doesnt print the SKU on their label
                                     // m_supplier = Supplier::Digikey;
            }
            break;
        }

    } else if (QRegularExpression("\\b\\d{7}\\b").match(ui->lineEdit->text()).hasMatch()) {
        //m_supplier = Supplier::Farnell;
        ui->listWidget->addItem(ui->lineEdit->text());
    }
    ui->lineEdit->setText("");
}
//[)>06PLTC6102CMS8#PBF-ND1PLTC6102CMS8#PBFKBE-0020221K5272417510K6034077511K14LQ1511ZPICK12Z164000813Z54115620Z00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

void BarcodeScanInputWindow::on_BarcodeScanInputWindow_accepted() {}
