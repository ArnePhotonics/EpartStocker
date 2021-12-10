#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(Settings &settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsWindow)
    , m_settings(settings) {
    ui->setupUi(this);
    ui->databasePathLineEdit->setText(m_settings.get_database_path());
    ui->farnellAPIKeyLineEdit->setText(m_settings.get_farnell_apikey());
    ui->digikeyClientIDLineEdit->setText(m_settings.get_digikey_clientID());
    ui->digikeySecretLineEdit->setText(m_settings.get_digikey_secret());
    ui->digikey_URLComboBox->clear();
    ui->digikey_URLComboBox->addItem(Settings::get_digikey_url_by_index(0));
    ui->digikey_URLComboBox->addItem(Settings::get_digikey_url_by_index(1));
    ui->digikey_URLComboBox->setCurrentIndex(m_settings.get_digikey_url());
    ui->mouserAPIKeyLineEdit->setText(m_settings.get_mouser_apikey());
    ui->installationSourceLineEdit->setText(m_settings.get_installation_source_path());
}

SettingsWindow::~SettingsWindow() {
    delete ui;
}

void SettingsWindow::on_buttonBox_accepted() {
    m_settings.set_database_path(ui->databasePathLineEdit->text());
    m_settings.set_farnell_apikey(ui->farnellAPIKeyLineEdit->text());
    m_settings.set_digikey_clientID(ui->digikeyClientIDLineEdit->text());
    m_settings.set_digikey_secret(ui->digikeySecretLineEdit->text());
    m_settings.set_digikey_url(ui->digikey_URLComboBox->currentIndex());
    m_settings.set_installation_source_path(ui->installationSourceLineEdit->text());
    m_settings.set_mouser_apikey(ui->mouserAPIKeyLineEdit->text());
}
