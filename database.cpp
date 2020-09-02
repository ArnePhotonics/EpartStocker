#include "database.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

PartDataBase::PartDataBase(QString file_name)
    : m_file_name(file_name)
    , m_category_nodes("") {
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
        part.lot = part_obj["lot"].toString();
        part.qty = part_obj[" pa"].toInt();
        if (m_parts.contains(part.id)) {
            throw DataBaseException("Loading parts database: contains multiple ids.");
        }

        auto &category_node = get_category_node_ref(part.category.toLower());
        category_node.m_part_ids.append(part.id);
        m_parts.insert(part.id, part);
    }
}

void PartDataBase::load_categories_recursive(PartCategoryTreeNode &categories_recursion, QString root, const QStringList &categorie_names) {
    for (auto cat : categorie_names) {
        cat = cat.toLower();
        qDebug() << root + "/" + cat;
        PartCategoryTreeNode category_node(cat);
        auto child_cats = get_categories_by_json(root + "/" + cat);
        load_categories_recursive(category_node, root + "/" + cat, child_cats);
        categories_recursion.insert_child(category_node);
    }
}

QStringList PartDataBase::get_categories_by_json(QString categorie_root) {
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
    QStringList result;
    if (found) {
        for (auto current_child : current_cat_array) {
            result.append(current_child.toObject()["name"].toString());
        }
    }
    return result;
#endif
    return QStringList();
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
