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
    DigikeyWrapper(const Settings &settings, QObject *parent = nullptr);

    QNetworkReply *requestHotThreads();

    bool isPermanent() const;
    void setPermanent(bool value);

    public slots:
    void grant();
    void subscribeToLiveUpdates();

    signals:
    void authenticated();
    void got_digikey_data(const QMap<QString, QString> &data);

    private:
    QOAuth2AuthorizationCodeFlow oauth2;
    QNetworkAccessManager *network_manager;
    const Settings &m_settings;
    bool permanent = false;
};

#endif // DIGIKEYWRAPPER_H
