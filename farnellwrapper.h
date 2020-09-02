#ifndef FARNELLWRAPPER_H
#define FARNELLWRAPPER_H

#include "settingswindow.h"
#include <QNetworkAccessManager>
#include <QObject>

class FarnellWrapper : public QObject {
    Q_OBJECT
    public:
    FarnellWrapper(const Settings &settings, QObject *parent);
    ~FarnellWrapper();
    void query(QString sku);
    signals:
    void got_data(const QMap<QString, QString> data, const QStringList additional_text);

    private:
    const Settings &m_settings;
    QNetworkAccessManager *network_manager;
};

#endif // FARNELLWRAPPER_H
