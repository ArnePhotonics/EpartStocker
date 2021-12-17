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

void MouserWrapper::query(QString partnumber, SKUorMPN sku_or_mpn) {
    QUrlQuery url_query;
    url_query.addQueryItem("apiKey", m_settings.get_mouser_apikey());

    QJsonObject obj_request;
    if (sku_or_mpn == bySKU) {
        obj_request["mouserPartNumber"] = partnumber;
    } else if (sku_or_mpn == byMPN) {
        obj_request["mouserPartNumber"] = partnumber;
        //obj_request["keyword"] = partnumber;
    } else {
        assert(0);
    }
    obj_request["partSearchOptions"] = "value2";
    QJsonObject obj;
    obj["SearchByPartRequest"] = obj_request;
    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QString endpoint;
    if (sku_or_mpn == bySKU) {
        endpoint = "partnumber";
    } else if (sku_or_mpn == byMPN) {
        endpoint = "partnumber";
        //endpoint = "keyword";
    } else {
        assert(0);
    }

    QUrl url("https://api.mouser.com/api/v1.0/search/" + endpoint);
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

        data["image_url"] = product_obj["ImagePath"].toString();
        data["datasheet_url"] = product_obj["DataSheetUrl"].toString();

        data["manufacturer"] = product_obj["Manufacturer"].toString();
        data["url"] = product_obj["ProductDetailUrl"].toString();
        data["mpn"] = product_obj["ManufacturerPartNumber"].toString();
        data["category"] = product_obj["Category"].toString();
        data["supplier"] = Supplier(Supplier::Mouser).toStr();
        data["sku"] = product_obj["MouserPartNumber"].toString();
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
