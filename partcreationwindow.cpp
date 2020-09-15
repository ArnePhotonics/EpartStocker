#include "partcreationwindow.h"
#include "barcodescaninputwindow.h"
#include "mainwindow.h"
#include "mpnsuggestionwindow.h"
#include "ui_partcreationwindow.h"
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QRegularExpression>
#include <QUrl>
#include <QtGlobal>
#include <QtNetworkAuth>

PartDetailWindow::PartDetailWindow(const Settings &settings, DigikeyWrapper &digikey_wrapper, PartDataBase &part_data_base, int part_id, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PartCreationWindow)
    , m_farnell_wrapper(settings, parent)
    , m_digikey_wrapper(digikey_wrapper)
    , m_part_data_base(part_data_base)
    , m_part_id(part_id) {
    ui->setupUi(this);
    m_network_access_manager = new QNetworkAccessManager(this);
    connect(m_network_access_manager, &QNetworkAccessManager::finished, this, &PartDetailWindow::image_download_finished);
    connect(&m_digikey_wrapper, &DigikeyWrapper::got_data, this, &PartDetailWindow::lookup_received);
    connect(&m_farnell_wrapper, &FarnellWrapper::got_data, this, &PartDetailWindow::lookup_received);

    connect(&m_digikey_wrapper, &DigikeyWrapper::supplier_error, this,
            [this](const QString error_message) { QMessageBox::warning(this, QObject::tr("Lookup Error"), error_message); });
    connect(&m_farnell_wrapper, &FarnellWrapper::supplier_error, this,
            [this](const QString error_message) { QMessageBox::warning(this, QObject::tr("Lookup Error"), error_message); });

    m_part_data_base.create_tree_view_items(ui->treeWidget);

    load_ui_from_part();
    m_mpn_suggestion_window = new MPNSuggestionWindow(this);
}

PartDetailWindow::~PartDetailWindow() {
    delete m_network_access_manager;
    delete ui;
}

void PartDetailWindow::on_scanbarcodeButton_clicked() {
    BarcodeScanInputWindow scan_input;
    if (scan_input.exec()) {
        auto scan_result = scan_input.get_parsed_fields();
        for (auto s : scan_result.keys()) {
            if (s == "mpn") {
                // m_suppress_mpn_kreypress_event = true;
                ui->mPNLineEdit->setText(scan_result[s]);
                //  m_suppress_mpn_kreypress_event = false;
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

void PartDetailWindow::on_lookupButton_clicked() {
    //    m_digikey_wrapper.grant();
    QString sku = ui->sKULineEdit->text().trimmed();
    auto supplier = Supplier(sku);
    switch (supplier.type()) {
        case Supplier::Digikey:
            m_digikey_wrapper.query(sku);
            break;
        case Supplier::Farnell:
            m_farnell_wrapper.query(sku);
            break;
        case Supplier::None:
            QMessageBox::warning(this, QObject::tr("Unknown sku"), tr("The sku %1 is not recognized as a sku from Farnell or Digikey").arg(sku));
            break;
    }
}

void PartDetailWindow::image_download_finished(QNetworkReply *reply) {
    QPixmap pm;
    auto data = reply->readAll();
    pm.loadFromData(data);
    pm = pm.scaled(100, 100, Qt::KeepAspectRatio);
    ui->imageLabel->setPixmap(pm);
}

void PartDetailWindow::lookup_received(QMap<QString, QString> data, const QMap<QString, QString> additional_paramters) {
    (void)additional_paramters;
    ui->descriptionLineEdit->setText(data["description"]);
    ui->manufacturerLineEdit->setText(data["manufacturer"]);
    //  m_suppress_mpn_kreypress_event = true;
    ui->mPNLineEdit->setText(data["mpn"]);
    //m_suppress_mpn_kreypress_event = false;
    ui->supplierLineEdit->setText(data["supplier"]);
    set_ui_datasheetURL(data["datasheet_url"]);
    set_ui_supplierURL(data["url"]);
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
    m_additional_parameters = additional_paramters;
    set_ui_additional_parts();
}

void PartDetailWindow::set_ui_datasheetURL(QString URL) {
    ui->datasheetLinkLineEdit->setText(URL);
    ui->datasheetlinkLabel->setText("<a href=" + URL + ">link</a>");
}

void PartDetailWindow::set_ui_supplierURL(QString URL) {
    ui->supplierLinkLineEdit->setText(URL);
    ui->supplierlinkLinkLabel->setText("<a href=" + URL + ">link</a>");
}

void PartDetailWindow::on_buttonBox_accepted() {
    Part part;
    if (m_part_id > -1) {
        part = m_part_data_base.get_part(m_part_id);
    }
    part.datasheet_link = ui->datasheetLinkLineEdit->text();
    part.sku = ui->sKULineEdit->text();
    part.supplier = ui->supplierLineEdit->text();
    part.mpn = ui->mPNLineEdit->text();
    part.manufacturer = ui->manufacturerLineEdit->text();
    part.ERP_number = ui->eRPLineEdit->text();
    part.category = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString();
    part.description = ui->descriptionLineEdit->text();
    part.location = ui->locationLineEdit->text();
    part.url = ui->supplierLinkLineEdit->text();
    part.additional_parameters = m_additional_parameters;
    part.qty = ui->qtySpinBox->value();
    if (ui->qtyManyCheckbox->isChecked()) {
        part.qty = -1000;
    }
    if (m_part_id == -1) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        part.image = ui->imageLabel->pixmap(Qt::ReturnByValueConstant::ReturnByValue);
#else
        const QPixmap *pixmap = ui->imageLabel->pixmap();
        if (pixmap) {
            part.image = QPixmap(*pixmap);
        }
#endif
        try {
            m_part_data_base.insert_new_part(part);
        } catch (DataBaseException &e) {
            QMessageBox::warning(this, tr("Database error"),
                                 tr("Error while inserting a part into database file: %1, program must be resarted.").arg(e.what()));
        }
    } else {
        m_part_data_base.update_part(part);
    }
}

bool PartDetailWindow::is_valid_for_ok_click() {
    bool result = true;

    QPalette palette_red_text = QGuiApplication::palette();
    palette_red_text.setColor(QPalette::WindowText, Qt::red);
    palette_red_text.setColor(QPalette::Text, Qt::red);

    QPalette palette_red_bg = QGuiApplication::palette();
    palette_red_bg.setColor(QPalette::Base, Qt::red);

    QPalette palette_normal = QGuiApplication::palette();

    auto current_cat_tree_widget_item = ui->treeWidget->currentItem();
    if (current_cat_tree_widget_item == nullptr) {
        result = false;
        ui->exampleLable->setText(tr("No categorie selected.\n "));
        ui->exampleLable->setPalette(palette_red_text);
    } else {
        bool description_ok = false;
        auto current_cat_path = current_cat_tree_widget_item->data(0, Qt::UserRole).toString();
        auto cat_node =
            m_part_data_base.get_category_node_ref(current_cat_path, QObject::tr("Loading decription validator for category path %1").arg(current_cat_path));
        if (cat_node.is_allowed_to_contain_parts() == false) {
            result = false;
            ui->exampleLable->setText(tr("Can not put a part into the selected categorie.\n "));
            ui->exampleLable->setPalette(palette_red_text);
        } else {
            ui->exampleLable->setText(cat_node.get_valid_description_example() + "\n" + cat_node.get_json_comment());
            ui->exampleLable->setPalette(palette_normal);
            auto text_under_test = ui->descriptionLineEdit->text();
            for (const auto &validator : cat_node.get_validators()) {
                if (text_under_test.contains(QRegularExpression(validator))) {
                    description_ok = true;
                }
            }
            if (cat_node.get_validators().count() == 0) {
                description_ok = true;
            }
            QPalette palette = palette_normal;

            if (description_ok == false) {
                result = false;
                ui->descriptionLineEdit->setPalette(palette_red_bg);
                ui->descriptionLineEdit->setToolTip(
                    tr("must match the validator: %1\n%2").arg(cat_node.get_validators().join("\n")).arg(cat_node.get_json_comment()));
                ui->descriptionLineEdit->setToolTipDuration(std::numeric_limits<int>::max());
            } else {
                ui->descriptionLineEdit->setPalette(palette_normal);
            }
        }
    }
    QString location = ui->locationLineEdit->text();
    if (location.contains(QRegularExpression("\\d+\\/\\d+"))) {
        ui->locationLineEdit->setPalette(palette_normal);

    } else {
        ui->locationLineEdit->setPalette(palette_red_bg);
        result = false;
    }
    if (ui->mPNLineEdit->text().isEmpty()) {
        ui->mPNLineEdit->setPalette(palette_red_bg);
        result = false;
    } else {
        ui->mPNLineEdit->setPalette(palette_normal);
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(result);
    return result;
}

void PartDetailWindow::set_ui_additional_parts() {
    QString lbl_text = "<table>";
    for (auto field_name : m_additional_parameters.uniqueKeys()) {
        lbl_text += ("<tr><td>" + field_name + "</td><td>" + m_additional_parameters[field_name] + "</td></tr>");
    }
    lbl_text += "</table>";
    ui->additionalparameterLabel->setText(lbl_text);
}

void PartDetailWindow::load_ui_from_part() {
    if (m_part_id > -1) {
        auto part = m_part_data_base.get_part(m_part_id);
        ui->sKULineEdit->setText(part.sku);
        ui->supplierLineEdit->setText(part.supplier);
        m_suppress_mpn_kreypress_event = true;
        ui->mPNLineEdit->setText(part.mpn);
        m_suppress_mpn_kreypress_event = false;
        ui->manufacturerLineEdit->setText(part.manufacturer);
        ui->eRPLineEdit->setText(part.ERP_number);
        ui->descriptionLineEdit->setText(part.description);
        ui->locationLineEdit->setText(part.location);
        ui->imageLabel->setPixmap(part.image);
        set_ui_supplierURL(part.url);
        set_ui_datasheetURL(part.datasheet_link);
        if (part.qty == -1000) {
            ui->qtyManyCheckbox->setCheckState(Qt::Checked);
        } else {
            ui->qtySpinBox->setValue(part.qty);
            ui->qtyManyCheckbox->setCheckState(Qt::Unchecked);
        }
        auto select_item = tree_widget_item_by_categorie_path(ui->treeWidget, part.category);
        ui->treeWidget->setCurrentItem(select_item);
        m_additional_parameters = part.additional_parameters;
        set_ui_additional_parts();
    }
    ui->continueWithNextPartButton->setVisible(m_part_id == -1);
    ui->lookupButton->setVisible(m_part_id == -1);
    ui->scanbarcodeButton->setVisible(m_part_id == -1);
    is_valid_for_ok_click();
}

void PartDetailWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    is_valid_for_ok_click();
    (void)current;
    (void)previous;
}

void PartDetailWindow::on_descriptionLineEdit_textChanged(const QString &arg1) {
    is_valid_for_ok_click();
    (void)arg1;
}

void PartDetailWindow::on_locationLineEdit_textChanged(const QString &arg1) {
    is_valid_for_ok_click();
    (void)arg1;
}

void PartDetailWindow::on_mPNLineEdit_textChanged(const QString &arg1) {
    is_valid_for_ok_click();
    show_mpn_proposals_window(arg1);
    (void)arg1;
}

void PartDetailWindow::show_mpn_proposals_window(QString mpn) {
    if ((m_suppress_mpn_kreypress_event == false) && (m_part_id == -1)) {
        //LT1617ES5-1#TRMPBF
        //LM2664M6/NOPB
        //MAX9034AUD+T
        //LT6221CS8#TRPBF
        //74HC7541DB,112
        //XC2C64A-7VQG100C
        mpn = mpn.left(mpn.indexOf(QRegularExpression("[-/+#,]")));
        //LT1617ES5
        //LM2664M6
        //MAX9034AUD
        //LT6221CS8
        //74HC7541DB
        //XC2C64A

        QRegularExpression mpn_shorting_regex("[a-zA-Z]*\\d*[a-zA-Z]+\\d{2,10}");
        mpn = mpn_shorting_regex.match(mpn).captured(0);
        qDebug() << mpn;
        QRegularExpression regex(mpn, QRegularExpression::CaseInsensitiveOption);
        auto proposals = m_part_data_base.get_mpn_proposals(regex);
        QStringList sl;
        if (proposals.count() && (proposals.count() < 5)) {
            for (auto proposal : proposals) {
                sl.append(proposal.first);
            }
            auto pos = ui->mPNLineEdit->pos();
            pos.setY(pos.y() + ui->mPNLineEdit->height());
            m_mpn_suggestion_window->show_suggestions(mapToGlobal(pos), ui->mPNLineEdit->width(), sl);
            //     ui->mPNLineEdit->setFocus();
        }
    }
}

void PartDetailWindow::on_datasheetlinkLabel_linkActivated(const QString &link) {
    QDesktopServices::openUrl(QUrl(link));
}

void PartDetailWindow::on_supplierlinkLinkLabel_linkActivated(const QString &link) {
    QDesktopServices::openUrl(QUrl(link));
}

void PartDetailWindow::on_qtyManyCheckbox_stateChanged(int arg1) {
    (void)arg1;
    ui->qtySpinBox->setEnabled(!ui->qtyManyCheckbox->isChecked());
}

void PartDetailWindow::on_continueWithNextPartButton_clicked() {
    on_buttonBox_accepted();
    if (parent()) {
        auto mainwindow = dynamic_cast<MainWindow *>(parent());
        if (mainwindow) {
            mainwindow->open_partcreation_window_for_new_part();
        }
    }
    close();
}
