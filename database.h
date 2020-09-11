#ifndef DATABASE_H
#define DATABASE_H
#include <QJsonObject>
#include <QLockFile>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <QTreeWidget>
#include <exception>

QTreeWidgetItem *tree_widget_item_by_categorie_path(QTreeWidget *tree_widget, QString categorie_path);

class DataBaseException : public std::runtime_error {
    public:
    DataBaseException(const QString &str)
        : std::runtime_error(str.toStdString()) {}

    int get_error_number() const {
        return error_number;
    }

    private:
    int error_number;
};

class PartCategoryTreeNode {
    public:
    PartCategoryTreeNode(){};
    PartCategoryTreeNode(QString name, QStringList description_validors, QString valid_description_example, QString json_comment, bool allowed_to_contain_parts)
        : m_name(name)
        , m_description_validators(description_validors)
        , m_valid_description_example(valid_description_example)
        , m_json_comment(json_comment)
        , m_allowed_to_contain_parts(allowed_to_contain_parts) {}
    void clear() {
        m_children.clear();
    }

    void insert_child(PartCategoryTreeNode &child) {
        m_children.insert(child.m_name, child);
    }

    QStringList get_children_names() {
        return m_children.keys();
    }

    void remove_part_id(int part_id) {
        m_part_ids.removeAll(part_id);
    }

    void append_part_id(int part_id) {
        m_part_ids.append(part_id);
    }

    bool is_allowed_to_contain_parts() const {
        return m_allowed_to_contain_parts;
    }
    PartCategoryTreeNode &get_category(QString categorie_path, const QString &additional_info);
    static QStringList split_category_path(QString path) {
        auto result = path.split("/", Qt::SkipEmptyParts);
        return result;
    }

    QJsonArray to_json() const;
    void create_tree_view_items(QTreeWidget *treeview) const;
    void get_partids_of_subcategories(QVector<int> &part_ids) const;

    const QStringList &get_validators() const {
        return m_description_validators;
    }

    const QString &get_json_comment() const {
        return m_json_comment;
    }

    const QString &get_valid_description_example() const {
        return m_valid_description_example;
    }

    private:
    PartCategoryTreeNode &get_child(const QString &node_name, const QString &additional_info) {
        if (m_children.contains(node_name)) {
            return m_children[node_name];
        } else {
            throw DataBaseException(
                QObject::tr(
                    "Dont find categorie element \"%1\". This often happens if a part is assigned to a non existing categorie(%1).\nAdditional info: %2")
                    .arg(node_name)
                    .arg(additional_info));
        }
    }
    QVector<int> m_part_ids;
    void get_partids_of_subcategories_recursive(QVector<int> &part_ids) const;
    void create_tree_view_items_recursive(QTreeWidgetItem *treeview_item, QString root_string);
    QMap<QString, PartCategoryTreeNode> m_children;
    PartCategoryTreeNode &get_categorie_by_path_recursion(QStringList path, int depth, const QString &additional_info);

    QString m_name;
    QStringList m_description_validators;
    QString m_valid_description_example;
    QString m_json_comment;
    bool m_allowed_to_contain_parts;
    QJsonObject to_json_recursive() const;
};

class Part {
    public:
    int id;
    QString sku;
    QString url;
    QString supplier;
    QString mpn;
    QString manufacturer;
    QString ERP_number;
    QString category;
    QString description;
    QString datasheet_link;
    QString location;
    QPixmap image;
    QMap<QString, QString> additional_parameters;
    int qty;
};

class FlatCategory {
    public:
    QString m_name;
    QStringList m_description_validator;
    QString m_valid_descriptor_example;
    QString m_json_comment;
    bool m_allowed_to_contain_parts = true;
};

class PartDataBase {
    public:
    PartDataBase(QString file_name);

    int insert_new_part(Part new_part);

    void save_to_file();
    QMap<int, Part> get_parts_by_categorie(QString categorie_root);

    PartCategoryTreeNode get_category_node(QString categorie_path, const QString &additional_info);
    PartCategoryTreeNode &get_category_node_ref(QString categorie_path, const QString &additional_info);

    Part get_part(int part_id);
    void create_tree_view_items(QTreeWidget *treewidget) const;

    int update_part(Part new_part);

    private:
    void db_reload();
    bool db_is_file_modified();
    void db_lock();
    void db_unlock();
    void update_part_with_valid_id(const Part &part);
    void insert_part_with_valid_id(const Part &new_part);
    void get_partids_of_subcategories_recursive(QList<int> &part_ids, PartCategoryTreeNode category_node);
    int get_new_id_and_lock_db();
    QString m_file_name;
    QJsonObject m_json_data;
    QMap<int, Part> m_parts;
    PartCategoryTreeNode m_category_nodes;
    void load_categories_recursive(PartCategoryTreeNode &categories_recursion, QString root, const QList<FlatCategory> &flat_categories);

    QList<FlatCategory> get_categories_by_json(QString categorie_root);

    QString m_filename;
    QLockFile m_lockfile;
    int m_next_id = 1000;
};

#endif // DATABASE_H
