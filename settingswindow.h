#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QObject>
#include <QSettings>
namespace Ui {
    class SettingsWindow;
}

class Settings {
    public:
    Settings(QObject *parent)
        : m_settings(QSettings::IniFormat, QSettings::UserScope, "CPG", "elektronik_lager", parent) {}

    void set_digikey_secret(QString val) {
        m_settings.setValue("digikey_secret", val);
    }

    QString get_digikey_secret() const {
        return m_settings.value("digikey_secret", "").toString();
    }

    void set_digikey_clientID(QString val) {
        m_settings.setValue("digikey_clientID", val);
    }

    QString get_digikey_clientID() const {
        return m_settings.value("digikey_clientID", "").toString();
    }

    void set_digikey_url(int val) {
        m_settings.setValue("digikey_url", val);
    }

    int get_digikey_url() const {
        return m_settings.value("digikey_url", "").toInt();
    }

    QString get_digikey_url_string() const {
        return get_digikey_url_by_index(get_digikey_url());
    }

    static QString get_digikey_url_by_index(int val) {
        return QStringList{"https://sandbox-api.digikey.com/", "https://api.digikey.com"}[val];
    }

    void set_farnell_apikey(QString val) {
        m_settings.setValue("farnell_apikey", val);
    }
    QString get_farnell_apikey() const {
        return m_settings.value("farnell_apikey", "").toString();
    }

    void set_mouser_apikey(QString val) {
        m_settings.setValue("mouser_apikey", val);
    }
    QString get_mouser_apikey() const {
        return m_settings.value("mouser_apikey", "").toString();
    }

    QString get_farnell_store() const {
        return "de.farnell.com";
    }

    void set_database_path(QString val) {
        m_settings.setValue("database_path", val);
    }
    QString get_database_path() const {
        return m_settings.value("database_path", "").toString();
    }

    void set_installation_source_path(QString val) {
        m_settings.setValue("installation_source", val);
    }

    QString get_installation_source_path() const {
        return m_settings.value("installation_source", "").toString();
    }

    void set_selected_category(QString val) {
        m_settings.setValue("selected_category", val);
    }

    QString get_selected_category() const {
        return m_settings.value("selected_category", "").toString();
    }

    private:
    QSettings m_settings;
};

class SettingsWindow : public QDialog {
    Q_OBJECT

    public:
    explicit SettingsWindow(Settings &settings, QWidget *parent = nullptr);
    ~SettingsWindow();

    private slots:
    void on_buttonBox_accepted();

    private:
    Ui::SettingsWindow *ui;
    Settings &m_settings;
};

#endif // SETTINGSWINDOW_H
