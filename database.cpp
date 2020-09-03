#include "database.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
PartDataBase::PartDataBase(QString file_name)
    : m_file_name(file_name)
    , m_category_nodes("", QList<QStringList>()) {
    QFile loadFile(m_file_name);
    loadFile.open(QIODevice::ReadOnly);
    QByteArray raw_data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(raw_data));
    m_json_data = loadDoc.object();
    const auto &part_array = m_json_data["parts"].toArray();
    m_parts.clear();
    m_category_nodes.clear();

    auto cats = get_categories_by_json("");
    load_categories_recursive(m_category_nodes, "", cats);

    // qDebug() << "cat_count" << cats_;
    //  qDebug() << m_categories;
    for (const auto &part_json : part_array) {
        const auto &part_obj = part_json.toObject();
        Part part;
        part.id = part_obj["id"].toInt();
        part.sku = part_obj["sku"].toString();
        part.supplier = part_obj["supplier"].toString();
        part.mpn = part_obj["mpn"].toString();
        part.manufacturer = part_obj["manufacturer"].toString();
        part.category = part_obj["category"].toString();
        part.description = part_obj["description"].toString();
        part.datasheet_link = part_obj["datasheet_link"].toString();
        part.location = part_obj["location"].toString();
        part.qty = part_obj["qty"].toInt();
        part.image.loadFromData(QByteArray::fromBase64(part_obj["image"].toString().toLatin1()));
        insert_part_with_valid_id(part);
    }
}

int PartDataBase::insert_new_part(Part new_part) {
    new_part.id = get_new_id_and_lock_db();
    insert_part_with_valid_id(new_part);
    save_to_file();
    db_unlock();
    return new_part.id;
}

void PartDataBase::insert_part_with_valid_id(const Part &new_part) {
    if (m_parts.contains(new_part.id)) {
        throw DataBaseException("Loading parts database: contains multiple ids.");
    }
    auto &category_node = new_part.category.toLower() == "" ? m_category_nodes : get_category_node_ref(new_part.category.toLower());
    category_node.m_part_ids.append(new_part.id);
    m_parts.insert(new_part.id, new_part);
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
        part_obj["location"] = part.location;
        part_obj["qty"] = part.qty;

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        part.image.save(&buffer);
        auto const encoded = buffer.data().toBase64();
        part_obj["image"] = QLatin1String(encoded);
        parts_root_array.append(part_obj);
    }
    root_object["parts"] = parts_root_array;
    root_object["categories"] = m_category_nodes.to_json();
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
    }
    QJsonDocument saveDoc(root_object);
    saveFile.write(saveDoc.toJson());
}

int PartDataBase::get_new_id_and_lock_db() {
    db_lock();
    return m_parts.keys().back() + 1;
}

void PartDataBase::load_categories_recursive(PartCategoryTreeNode &categories_recursion, QString root, const QList<FlatCategory> &flat_categorie) {
    for (auto cat : flat_categorie) {
        cat.m_name = cat.m_name.toLower();
        qDebug() << root + "/" + cat.m_name;
        PartCategoryTreeNode category_node(cat.m_name, cat.m_description_validators);
        auto child_cats = get_categories_by_json(root + "/" + cat.m_name);
        load_categories_recursive(category_node, root + "/" + cat.m_name, child_cats);
        categories_recursion.insert_child(category_node);
    }
}

QList<FlatCategory> PartDataBase::get_categories_by_json(QString categorie_root) {
    auto categories_array = m_json_data["categories"].toArray();
    // qDebug() << categories_array;
#if 1
    auto current_cat_array = categories_array;
    bool found = categorie_root == "";
    auto root_cat_path = PartCategoryTreeNode::split_category_path(categorie_root);
    for (const auto &path_item : root_cat_path) {
        for (auto current_child : current_cat_array) {
            if (current_child.toObject()["name"].toString().toLower() == path_item.toLower()) {
                found = true;
                current_cat_array = current_child.toObject()["children"].toArray();
                break;
            }
        }
    }
    QList<FlatCategory> result;
    if (found) {
        for (auto current_child : current_cat_array) {
            FlatCategory flat_category;
            flat_category.m_name = current_child.toObject()["name"].toString();
            auto description_validators_array = current_child.toObject()["description_validator"].toArray();
            for (const auto &validors : description_validators_array) {
                QStringList sl;
                for (const auto &validor : validors.toArray()) {
                    sl.append(validor.toString());
                }
                flat_category.m_description_validators.append(sl);
            }
            result.append(flat_category);
        }
    }
    return result;
#endif
    return QList<FlatCategory>();
}

QMap<int, Part> PartDataBase::get_parts_by_categorie(QString categorie_root) {
    auto cat_node = get_category_node(categorie_root);
    QVector<int> part_ids;
    cat_node.get_partids_of_subcategories_recursive(part_ids);
    QMap<int, Part> result;
    for (auto part_id : part_ids) {
        result.insert(part_id, m_parts.value(part_id));
    }
    return result;
}

PartCategoryTreeNode PartDataBase::get_category_node(QString categorie_path) {
#if 1
    qDebug() << "queried:" << categorie_path;
    if (categorie_path == "") {
        return m_category_nodes;
    } else {
        return m_category_nodes.get_category(categorie_path);
    }
#endif
}

PartCategoryTreeNode &PartDataBase::get_category_node_ref(QString categorie_path) {
#if 1
    qDebug() << "queried:" << categorie_path;
    return m_category_nodes.get_category(categorie_path);
#endif
}

void PartDataBase::db_reload() {}

bool PartDataBase::db_is_file_modified() {
    return false;
}

void PartDataBase::db_lock() {
    if (db_is_file_modified()) {
        db_reload();
    }
}

void PartDataBase::db_unlock() {}

PartCategoryTreeNode &PartCategoryTreeNode::get_category(QString categorie_path) {
    auto cat_path = split_category_path(categorie_path);
    if (cat_path.count() == 1) {
        return get_child(cat_path[0]);
    } else {
        return get_child(cat_path[0]).get_categorie_by_path_recursion(cat_path, 1);
    }
}

PartCategoryTreeNode &PartCategoryTreeNode::get_categorie_by_path_recursion(QStringList path, int depth) {
    bool is_leaf = (path.count() == depth + 1);
    auto path_element = path[depth];
    if (is_leaf) {
        return get_child(path_element);
    } else {
        return get_child(path_element).get_categorie_by_path_recursion(path, depth + 1);
    }
}

void PartCategoryTreeNode::get_partids_of_subcategories_recursive(QVector<int> &part_ids) const {
    part_ids.append(m_part_ids);
    for (const auto &cat : m_children) {
        cat.get_partids_of_subcategories_recursive(part_ids);
    }
}

QJsonObject PartCategoryTreeNode::to_json() const {
    QJsonObject root_obj;
    QJsonArray children_array;
    QJsonArray description_validators_array;
    root_obj["name"] = m_name;
    for (const auto &child : m_children) {
        children_array.append(child.to_json());
    }
    root_obj["children"] = children_array;
    for (const auto &validators : m_description_validator) {
        QJsonArray description_validator_array;
        for (const auto &validator : validators) {
            description_validator_array.append(validator);
        }
        description_validators_array.append(description_validator_array);
    }
    root_obj["description_validator"] = description_validators_array;
    return root_obj;
}
