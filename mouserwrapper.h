#ifndef MOUSERWRAPPER_H
#define MOUSERWRAPPER_H

#include "settingswindow.h"
#include <QNetworkAccessManager>
#include <QObject>

class MouserWrapper : public QObject {
    Q_OBJECT
    public:
    enum SKUorMPN { bySKU, byMPN };
    MouserWrapper(const Settings &settings, QObject *parent);
    ~MouserWrapper();
    void query(QString partnumber, SKUorMPN sku_or_mpn);
    signals:
    void got_data(const QMap<QString, QString> data, const QMap<QString, QString> additional_paramters);
    void supplier_error(const QString error_message);

    private:
    const Settings &m_settings;
    QNetworkAccessManager *network_manager;
};

#endif // MOUSERWRAPPER_H
