#include "farnellwrapper.h"
#include "barcodescaninputwindow.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

FarnellWrapper::FarnellWrapper(const Settings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings) {
    network_manager = new QNetworkAccessManager(this);
}

FarnellWrapper::~FarnellWrapper() {
    delete network_manager;
}

void FarnellWrapper::query(QString sku) {
    QUrlQuery url_query;
    QUrl url("http://api.element14.com/catalog/products");
    url_query.addQueryItem("storeInfo.id", m_settings.get_farnell_store());
    url_query.addQueryItem("term", ("id:" + sku).toUtf8());
    url_query.addQueryItem("callInfo.omitXmlSchema", "false");
    url_query.addQueryItem("callInfo.responseDataFormat", "json");
    url_query.addQueryItem("callInfo.apiKey", m_settings.get_farnell_apikey());
    url_query.addQueryItem("resultsSettings.offset", "0");
    url_query.addQueryItem("resultsSettings.numberOfResults", "0");
    url_query.addQueryItem("resultsSettings.refinements", "0");
    url_query.addQueryItem("resultsSettings.responseGroup", "large");
    url.setQuery(url_query);
    QNetworkRequest request(url);

    QNetworkReply *reply = network_manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << "Farnell error:" << reply->errorString();
            return;
        }

        const auto json = reply->readAll();
        QMap<QString, QString> data;
        // qDebug() << json;
        QFile f("farnell.json");
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&f);
        out << json;

#if 1
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto product_array = document.object()["premierFarnellPartNumberReturn"].toObject()["products"].toArray();
        Q_ASSERT(product_array.count());
        const auto product_obj = product_array[0];
        data["description"] = product_obj["displayName"].toString();

        //https://
        //The domain used for the API request  i.e. uk.farnell.com
        //Append the static text ‘/productimages/standard’
        //Then based on the vrntPath value append the locale following these rules -
        //  If vrntPath is ‘nio/’ add ‘en_US’ to path
        //Else if vrntPath is ‘farnell/’ take one of the following approaches –
        //i.     You could use our fall back language default across all domains and set the locale as ‘en_GB’,
        //but note if the customer is on any Store that doesn’t use English as a first language then the image
        //names themselves may have local language elements in them and hence you should use a locale code appropriate
        //to the primary transactional language
        //Then append the baseName  i.e the filename
        //
        //So by example you could end up with these : https://uk.farnell.com/productimages/standard/en_GB/GE20TSSOP-40.jpg
        //
        //Or https://fr.farnell.com/productimages/standard/fr_FR/GE20TSSOP-40.jpg
        QString lang = "en_US";
        if (product_obj["image"].toObject()["vrntPath"].toString().startsWith("‘nio")) {
        } else {
            lang = "en_GB";
        }
        data["image_url"] =
            "https://" + m_settings.get_farnell_store() + "/productimages/standard/" + lang + product_obj["image"].toObject()["baseName"].toString();
        const auto data_sheet_url = product_obj["datasheets"].toArray();
        if (data_sheet_url.count()) {
            data["datasheet_url"] = data_sheet_url[0].toObject()["url"].toString();
        }
        data["manufacturer"] = product_obj["brandName"].toString();
        data["url"] = "https://" + m_settings.get_farnell_store() + "/" + sku;
        data["mpn"] = product_obj["translatedManufacturerPartNumber"].toString();
        data["supplier"] = Supplier(Supplier::Farnell).toStr();
        qDebug() << data;

        QMap<QString, QString> additional_parameters;
        auto parameters = product_obj["attributes"].toArray();
        for (const auto &param : parameters) {
            //"attributeLabel":"Anschlussabstand",
            //"attributeUnit":"mm",
            //"attributeValue":"3.5"

            auto param_obj = param.toObject();
            auto param_name = param_obj["attributeLabel"].toString();
            auto param_value = param_obj["attributeValue"].toString();
            auto param_unit = param_obj["attributeUnit"].toString();
            if ((param_value == "") || (param_value == "-")) {
                continue;
            }
            additional_parameters[param_name] = param_value + param_unit;
        }
        emit got_data(data, additional_parameters);

#endif
    });
}
