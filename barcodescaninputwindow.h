#ifndef BARCODESCANINPUTWINDOW_H
#define BARCODESCANINPUTWINDOW_H

#include <QDialog>
#include <QMap>
#include <QRegularExpression>

namespace Ui {
    class BarcodeScanInputWindow;
}

class Supplier {
    public:
    enum Type { Digikey, Farnell, Mouser, None };
    Supplier(Type t)
        : m_t(t) {}
    Supplier(QString name_or_sku) {
        if (name_or_sku == "digikey") {
            m_t = Type::Digikey;
        } else if (name_or_sku == "farnell") {
            m_t = Type::Farnell;
        } else if (name_or_sku == "mouser") {
            m_t = Type::Mouser;
        } else if (name_or_sku == "none") {
            m_t = Type::None;
        } else if (QRegularExpression("\\b\\d{6,7}\\b").match(name_or_sku).hasMatch()) {
            m_t = Type::Farnell;
        } else if (QRegularExpression(".*-ND").match(name_or_sku).hasMatch()) {
            m_t = Type::Digikey;
        } else if (QRegularExpression("^\\d\\d\\d-.+$").match(name_or_sku).hasMatch()) {
            m_t = Type::Mouser;
        } else {
            m_t = Type::None;
        }
    }

    Supplier()
        : m_t(Type::None) {}
    Type type() const {
        return m_t;
    }
    QString toStr() const {
        switch (m_t) {
            case Type::Digikey:
                return "digikey";
            case Type::Farnell:
                return "farnell";
            case Type::Mouser:
                return "mouser";
            case Type::None:
                return "none";
        }
        return "";
    }

    private:
    Type m_t;
};

class BarcodeScanInputWindow : public QDialog {
    Q_OBJECT

    public:
    explicit BarcodeScanInputWindow(QWidget *parent = nullptr);
    ~BarcodeScanInputWindow();

    QMap<QString, QString> get_parsed_fields();
    private slots:
    void on_lineEdit_returnPressed();

    void on_btnOK_clicked();

    void on_BarcodeScanInputWindow_accepted();

    private:
    Ui::BarcodeScanInputWindow *ui;
};

#endif // BARCODESCANINPUTWINDOW_H
