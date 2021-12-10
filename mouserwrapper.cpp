#include "mouserwrapper.h"
#include "barcodescaninputwindow.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

MouserWrapper::MouserWrapper(const Settings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings) {
    network_manager = new QNetworkAccessManager(this);
}

MouserWrapper::~MouserWrapper() {
    delete network_manager;
}

void MouserWrapper::query(QString sku) {
    QUrlQuery url_query;
    url_query.addQueryItem("apiKey", m_settings.get_mouser_apikey());

    QJsonObject obj_request;
    obj_request["mouserPartNumber"] = sku;
    obj_request["partSearchOptions"] = "value2";
    QJsonObject obj;
    obj["SearchByPartRequest"] = obj_request;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    QUrl url("https://api.mouser.com/api/v1.0/search/partnumber");
    url.setQuery(url_query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = network_manager->post(request, data);

    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit supplier_error("Mouser error: " + reply->errorString());
            qCritical() << "Mouser error: " << reply->errorString();
            return;
        }

        const auto json = reply->readAll();
        QMap<QString, QString> data;
        qDebug() << json;
        QFile f("mouser.json");
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&f);
        out << json;

#if 1
        const auto document = QJsonDocument::fromJson(json);
        Q_ASSERT(document.isObject());
        const auto product_array = document.object()["SearchResults"].toObject()["Parts"].toArray();
        Q_ASSERT(product_array.count());
        const auto product_obj = product_array[0];
        data["description"] = product_obj["Description"].toString();

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

        data["image_url"] = product_obj["ImagePath"].toString();
        data["datasheet_url"] = product_obj["DataSheetUrl"].toString();

        data["manufacturer"] = product_obj["Manufacturer"].toString();
        data["url"] = product_obj["ProductDetailUrl"].toString();
        data["mpn"] = product_obj["ManufacturerPartNumber"].toString();
        data["category"] = product_obj["Category"].toString();
        data["supplier"] = Supplier(Supplier::Mouser).toStr();
        qDebug() << data;

        QMap<QString, QString> additional_parameters;
        additional_parameters["category"] = data["category"];
        auto parameters = product_obj["ProductAttributes"].toArray();
        for (const auto &param : parameters) {
            //"attributeLabel":"Anschlussabstand",
            //"attributeUnit":"mm",
            //"attributeValue":"3.5"

            auto param_obj = param.toObject();
            auto param_name = param_obj["AttributeName"].toString();
            auto param_value = param_obj["AttributeValue"].toString();
            if ((param_value == "") || (param_value == "-")) {
                continue;
            }
            additional_parameters[param_name] = param_value;
        }

        emit got_data(data, additional_parameters);

#endif
    });
}
