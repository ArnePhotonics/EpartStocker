#include "database.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

static QTreeWidgetItem *tree_widget_item_by_categorie_path_recursive(QTreeWidgetItem *root_widget_item, QString categorie_path) {
    for (int i = 0; i < root_widget_item->childCount(); i++) {
        auto child = root_widget_item->child(i);
        auto child_cat = child->data(0, Qt::UserRole).toString();
        if (child_cat == categorie_path) {
            return child;
        }
        auto result = tree_widget_item_by_categorie_path_recursive(root_widget_item->child(i), categorie_path);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

QTreeWidgetItem *tree_widget_item_by_categorie_path(QTreeWidget *tree_widget, QString categorie_path) {
    for (int i = 0; i < tree_widget->topLevelItemCount(); i++) {
        auto child = tree_widget->topLevelItem(i);
        auto child_cat = child->data(0, Qt::UserRole).toString();
        if (child_cat == categorie_path) {
            return child;
        }
        auto result = tree_widget_item_by_categorie_path_recursive(child, categorie_path);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

PartDataBase::PartDataBase(QString file_name)
    : m_file_name(file_name)
    , m_category_nodes("", QStringList(), "", "", false)
    , m_lockfile(file_name + ".lock") {
    db_reload_part_database();
}

void PartDataBase::db_reload_part_database() {
    QFile loadFile(m_file_name);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        throw DataBaseException(QObject::tr("Cannot open database file %1").arg(m_file_name));
    }
    QByteArray raw_data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(raw_data));
    auto json_data = loadDoc.object();
    if (json_data.isEmpty()) {
        throw DataBaseException(QObject::tr("Cannot parse database file %1. you should check its syntax with an json validor.").arg(m_file_name));
    }
    const auto &part_array = json_data["parts"].toArray();
    m_parts.clear();
    m_category_nodes.clear();
    m_mpn_to_id_map.clear();

    auto cats = get_categories_by_json("", json_data);
    load_categories_recursive(m_category_nodes, "", cats, json_data);

    m_next_id = json_data["next_part_id"].toInt();
    for (const auto &part_json : part_array) {
        const auto &part_obj = part_json.toObject();
        Part part;
        part.id = part_obj["id"].toInt();
        part.sku = part_obj["sku"].toString();
        part.supplier = part_obj["supplier"].toString();
        part.mpn = part_obj["mpn"].toString();
        part.manufacturer = part_obj["manufacturer"].toString();
        part.category = part_obj["category"].toString();
        part.ERP_number = part_obj["erp"].toString();
        part.description = part_obj["description"].toString();
        part.datasheet_link = part_obj["datasheet_link"].toString();
        part.location = part_obj["location"].toString();
        part.qty = part_obj["qty"].toInt();
        part.url = part_obj["url"].toString();

        part.reserved_for = part_obj["reserved_for"].toString();
        part.comment = part_obj["comment"].toString();
        part.provisioning_for = part_obj["provisioning_for"].toString();

        part.image.loadFromData(QByteArray::fromBase64(part_obj["image"].toString().toLatin1()), "JPG");
        auto param_obj = part_obj["parameters"].toObject();
        for (auto field_name : param_obj.keys()) {
            part.additional_parameters[field_name] = param_obj[field_name].toString();
        }
        insert_part_with_valid_id(part);
    }
    if (m_parts.keys().back() + 1 > m_next_id) {
        m_next_id = m_parts.keys().back() + 1;
    }
    db_is_file_modified();
}

int PartDataBase::insert_new_part(Part new_part) {
    new_part.id = get_new_id_and_lock_db();
    insert_part_with_valid_id(new_part);
    save_to_file();
    db_unlock();
    return new_part.id;
}

int PartDataBase::update_part(Part new_part) {
    update_part_with_valid_id(new_part);
    save_to_file();
    db_unlock();
    return new_part.id;
}

void PartDataBase::update_part_with_valid_id(const Part &part) {
    if (!m_parts.contains(part.id)) {
        throw DataBaseException(QObject::tr("Updating parts database: part which is to be updated does not exist"));
    }
    auto old_part = get_part(part.id);
    auto &old_category_node =
        old_part.category.toLower() == "" ?
            m_category_nodes :
            get_category_node_ref(old_part.category.toLower(), QObject::tr("loading category for old partid %1 before part update").arg(part.id));
    old_category_node.remove_part_id(part.id);

    m_parts[part.id] = part;
    m_mpn_to_id_map[part.mpn] = part.id;
    auto &category_node = part.category.toLower() == "" ?
                              m_category_nodes :
                              get_category_node_ref(part.category.toLower(), QObject::tr("loading category new old partid %1 after part update").arg(part.id));
    category_node.append_part_id(part.id);
}

void PartDataBase::insert_part_with_valid_id(const Part &new_part) {
    if (m_parts.contains(new_part.id)) {
        throw DataBaseException(QObject::tr("Loading parts database: contains multiple ids."));
    }
    auto &category_node = new_part.category.toLower() == "" ?
                              m_category_nodes :
                              get_category_node_ref(new_part.category.toLower(), QObject::tr("loading category partid %1 before part insert").arg(new_part.id));
    category_node.append_part_id(new_part.id);
    m_parts.insert(new_part.id, new_part);
    m_mpn_to_id_map[new_part.mpn] = new_part.id;
}

void PartDataBase::save_to_file() {
    QJsonObject root_object;
    QJsonArray parts_root_array;
    for (const auto &part : m_parts) {
        QJsonObject part_obj;

        part_obj["id"] = part.id;
        part_obj["sku"] = part.sku;
        part_obj["supplier"] = part.supplier;
        part_obj["mpn"] = part.mpn;
        part_obj["manufacturer"] = part.manufacturer;
        part_obj["category"] = part.category;
        part_obj["description"] = part.description;
        part_obj["datasheet_link"] = part.datasheet_link;
        part_obj["erp"] = part.ERP_number;
        part_obj["location"] = part.location;
        part_obj["qty"] = part.qty;
        part_obj["url"] = part.url;

        part_obj["reserved_for"] = part.reserved_for;
        part_obj["comment"] = part.comment;
        part_obj["provisioning_for"] = part.provisioning_for;

        QJsonObject param_obj;
        for (auto field_name : part.additional_parameters.keys()) {
            param_obj[field_name] = part.additional_parameters[field_name];
        }

        part_obj["parameters"] = param_obj;
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        part.image.save(&buffer, "JPG");
        auto const encoded = buffer.data().toBase64();
        part_obj["image"] = QLatin1String(encoded);
        parts_root_array.append(part_obj);
    }
    root_object["parts"] = parts_root_array;
    root_object["categories"] = m_category_nodes.to_json();
    root_object["next_part_id"] = m_next_id;
    QJsonObject storage_root_obj;
    QJsonObject storage_item_obj;
    storage_item_obj["min"] = 1;
    storage_item_obj["max"] = 30;
    storage_root_obj["1"] = storage_item_obj;
    storage_root_obj["2"] = storage_item_obj;
    root_object["storage_locations"] = storage_root_obj;

    QFile saveFile(m_file_name);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        throw DataBaseException(QObject::tr("Couldn't open save file."));
    }
    QJsonDocument saveDoc(root_object);
    saveFile.write(saveDoc.toJson());
    db_is_file_modified();
}

int PartDataBase::get_new_id_and_lock_db() {
    db_lock();
    m_next_id++;
    return m_next_id;
}

void PartDataBase::load_categories_recursive(PartCategoryTreeNode &categories_recursion, QString root, const QList<FlatCategory> &flat_categories,
                                             const QJsonObject &json_object) {
    for (auto cat : flat_categories) {
        cat.m_name = cat.m_name.toLower();
        //qDebug() << root + "/" + cat.m_name;
        PartCategoryTreeNode category_node(cat.m_name, cat.m_description_validator, cat.m_valid_descriptor_example, cat.m_json_comment,
                                           cat.m_allowed_to_contain_parts);
        auto child_cats = get_categories_by_json(root + "/" + cat.m_name, json_object);
        load_categories_recursive(category_node, root + "/" + cat.m_name, child_cats, json_object);
        categories_recursion.insert_child(category_node);
    }
}

QList<FlatCategory> PartDataBase::get_categories_by_json(QString categorie_root, const QJsonObject &json_object) {
    auto categories_array = json_object["categories"].toArray();
    // qDebug() << categories_array;
#if 1
    auto current_cat_array = categories_array;
    bool found = categorie_root == "";
    auto root_cat_path = PartCategoryTreeNode::split_category_path(categorie_root);
    for (const auto &path_item : root_cat_path) {
        for (auto current_child : current_cat_array) {
            if (current_child.toObject()["name"].toString().toLower() == "") {
                qDebug() << "cat name empty";
            }
            if (current_child.toObject()["name"].toString().toLower() == path_item.toLower()) {
                found = true;
                const auto &children_val = current_child.toObject()["children"];
                if (children_val.isArray()) {
                    current_cat_array = children_val.toArray();
                } else {
                    throw DataBaseException(QObject::tr("Error while loading category path %1. Seems the children field is not an array as it should be.")
                                                .arg(root_cat_path.join("/")));
                }
                break;
            }
        }
    }
    QList<FlatCategory> result;
    if (found) {
        for (auto current_child : current_cat_array) {
            if (current_child.isObject()) {
            } else {
                throw DataBaseException(QObject::tr("Error while loading category path %1. Seems the children field is not an array as it should be.")
                                            .arg(root_cat_path.join("/")));
            }
            FlatCategory flat_category;
            flat_category.m_name = current_child.toObject()["name"].toString();
            if (current_child.toObject()["parts_allowed"].isBool()) {
                flat_category.m_allowed_to_contain_parts = current_child.toObject()["parts_allowed"].toBool();
            } else {
                flat_category.m_allowed_to_contain_parts = true;
            }
            flat_category.m_valid_descriptor_example = current_child.toObject()["description_example"].toString();
            flat_category.m_json_comment = current_child.toObject()["_comment"].toString();

            for (const auto validator : current_child.toObject()["description_validator"].toArray()) {
                flat_category.m_description_validator.append(validator.toString());
            }

            result.append(flat_category);
        }
    }
    return result;
#endif
    return QList<FlatCategory>();
}

QMap<int, Part> PartDataBase::get_parts_by_categorie(QString categorie_root) {
    auto cat_node = get_category_node(categorie_root, QObject::tr("get_parts_by_categorie: %1").arg(categorie_root));
    QVector<int> part_ids;
    cat_node.get_partids_of_subcategories(part_ids);
    QMap<int, Part> result;
    for (auto part_id : part_ids) {
        result.insert(part_id, m_parts.value(part_id));
    }
    return result;
}

PartCategoryTreeNode PartDataBase::get_category_node(QString categorie_path, const QString &additional_info) {
#if 1
    //qDebug() << "queried:" << categorie_path;
    if (categorie_path == "") {
        return m_category_nodes;
    } else {
        return m_category_nodes.get_category(categorie_path, additional_info);
    }
#endif
}

PartCategoryTreeNode &PartDataBase::get_category_node_ref(QString categorie_path, const QString &additional_info) {
#if 1
    //qDebug() << "queried:" << categorie_path;
    return m_category_nodes.get_category(categorie_path, additional_info);
#endif
}

QList<QPair<QString, int>> PartDataBase::get_mpn_proposals(const QRegularExpression &base_mpn) const {
    QList<QPair<QString, int>> result;
    const QStringList &sl = m_mpn_to_id_map.keys();
    const QStringList &filtered_sl = sl.filter(base_mpn);
    for (const auto &mpn : filtered_sl) {
        QPair<QString, int> val;
        val.first = mpn;
        val.second = m_mpn_to_id_map[mpn];
        result.append(val);
    }
    return result;
}

Part PartDataBase::get_part(int part_id) {
    return m_parts[part_id];
}

void PartDataBase::create_tree_view_items(QTreeWidget *treewidget) const {
    treewidget->clear();
    m_category_nodes.create_tree_view_items(treewidget);
    treewidget->expandAll();
    treewidget->sortItems(0, Qt::AscendingOrder);
}

bool PartDataBase::db_is_file_modified() {
    auto current_filedate = QFileInfo(m_file_name).lastModified();
    if (current_filedate == m_file_date) {
        return false;
    } else {
        m_file_date = current_filedate;
        return true;
    }
}

void PartDataBase::db_lock() {
    bool result = m_lockfile.tryLock(5 * 1000);
    if (!result) {
        qint64 pid;
        QString hostname;
        QString appname;
        QString message;
        if (m_lockfile.getLockInfo(&pid, &hostname, &appname)) {
            message = QObject::tr("Used by host: %1, app: %2, pid: %3").arg(hostname).arg(appname).arg(pid);
        }
        throw DataBaseException(QObject::tr("Can't obtain database write permission. Locked by lockfile. %1").arg(message));
    }
    if (db_is_file_modified()) {
        db_reload_part_database();
    }
}

void PartDataBase::db_unlock() {
    m_lockfile.unlock();
}

PartCategoryTreeNode &PartCategoryTreeNode::get_category(QString categorie_path, const QString &additional_info) {
    auto cat_path = split_category_path(categorie_path);
    if (cat_path.count() == 1) {
        return get_child(cat_path[0], additional_info);
    } else {
        return get_child(cat_path[0], additional_info).get_categorie_by_path_recursion(cat_path, 1, additional_info);
    }
}

PartCategoryTreeNode &PartCategoryTreeNode::get_categorie_by_path_recursion(QStringList path, int depth, const QString &additional_info) {
    bool is_leaf = (path.count() == depth + 1);
    auto path_element = path[depth];
    if (is_leaf) {
        return get_child(path_element, additional_info);
    } else {
        return get_child(path_element, additional_info).get_categorie_by_path_recursion(path, depth + 1, additional_info);
    }
}

void PartCategoryTreeNode::get_partids_of_subcategories_recursive(QVector<int> &part_ids) const {
    part_ids.append(m_part_ids);
    for (const auto &cat : m_children) {
        cat.get_partids_of_subcategories_recursive(part_ids);
    }
}

void PartCategoryTreeNode::get_partids_of_subcategories(QVector<int> &part_ids) const {
    get_partids_of_subcategories_recursive(part_ids);
}

QJsonObject PartCategoryTreeNode::to_json_recursive() const {
    QJsonObject root_obj;
    QJsonArray children_array;
    for (const auto &child : m_children) {
        children_array.append(child.to_json_recursive());
    }
    QJsonArray validator_array;
    for (const auto &validator : m_description_validators) {
        validator_array.append(validator);
    }
    root_obj["name"] = m_name;
    root_obj["children"] = children_array;
    root_obj["description_validator"] = validator_array;
    root_obj["description_example"] = m_valid_description_example;
    root_obj["parts_allowed"] = m_allowed_to_contain_parts;
    root_obj["_comment"] = m_json_comment;

    return root_obj;
}

QJsonArray PartCategoryTreeNode::to_json() const {
    QJsonArray children_array;
    for (const auto &child : m_children) {
        children_array.append(child.to_json_recursive());
    }
    return children_array;
}

static QTreeWidgetItem *create_treewiedget_item(QString path, QString name, bool allowed_to_contain_parts) {
    QTreeWidgetItem *cat_widget = new QTreeWidgetItem(QStringList(name));
    cat_widget->setData(0, Qt::UserRole, QString(path));
    if (allowed_to_contain_parts == false) {
        cat_widget->setForeground(0, QBrush{Qt::gray});
    }
    return cat_widget;
}

void PartCategoryTreeNode::create_tree_view_items(QTreeWidget *treeview) const {
    for (auto child : m_children) {
        QTreeWidgetItem *cat_widget = create_treewiedget_item(child.m_name, child.m_name, child.m_allowed_to_contain_parts);
        child.create_tree_view_items_recursive(cat_widget, child.m_name);
        treeview->addTopLevelItem(cat_widget);
    }
}

void PartCategoryTreeNode::create_tree_view_items_recursive(QTreeWidgetItem *treeview_item, QString root_string) {
    for (auto child : m_children) {
        auto cat_widget = create_treewiedget_item(root_string + "/" + child.m_name, child.m_name, child.m_allowed_to_contain_parts);
        child.create_tree_view_items_recursive(cat_widget, root_string + "/" + child.m_name);
        treeview_item->addChild(cat_widget);
    }
}
