#ifndef DIGIKEYWRAPPER_H
#define DIGIKEYWRAPPER_H

#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>
#include <QtCore>
#include <QtNetwork>

class DigikeyWrapper : public QObject {
    Q_OBJECT
    public:
    DigikeyWrapper(QObject *parent = nullptr);
    DigikeyWrapper(const QString &clientIdentifier, QObject *parent = nullptr);

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
    bool permanent = false;
};

#endif // DIGIKEYWRAPPER_H
