#ifndef DIGIKEYWRAPPER_H
#define DIGIKEYWRAPPER_H

#include "settingswindow.h"
#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>
#include <QtCore>
#include <QtNetwork>

class DigikeyWrapper : public QObject {
    Q_OBJECT
    public:
    DigikeyWrapper(const Settings &settings, QObject *parent);
    ~DigikeyWrapper();
    QNetworkReply *requestHotThreads();

    bool isPermanent() const;
    void setPermanent(bool value);

    void query(QString sku);
    public slots:
    void grant();

    void just_authenticated();
    signals:
    void authenticated();
    void got_data(const QMap<QString, QString> data, const QStringList additional_text);

    private:
    QOAuth2AuthorizationCodeFlow oauth2;
    QNetworkAccessManager *network_manager;
    const Settings &m_settings;
    bool permanent = false;
    QString m_sku_to_query_after_auth;
    bool is_authenticated = false;
};

#endif // DIGIKEYWRAPPER_H
