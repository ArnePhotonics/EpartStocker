#ifndef DATABASE_H
#define DATABASE_H
#include <QJsonObject>
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
    PartCategoryTreeNode(QString name, QList<QStringList> description_validors)
        : m_name(name)
        , m_description_validator(description_validors) {}
    void clear() {
        m_children.clear();
    }
    PartCategoryTreeNode &get_child(const QString &node_name) {
        return m_children[node_name];
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

    PartCategoryTreeNode &get_category(QString categorie_path);
    static QStringList split_category_path(QString path) {
        auto result = path.split("/", Qt::SkipEmptyParts);
        return result;
    }

    QJsonArray to_json() const;
    void create_tree_view_items(QTreeWidget *treeview) const;
    void get_partids_of_subcategories(QVector<int> &part_ids) const;

    private:
    QVector<int> m_part_ids;
    void get_partids_of_subcategories_recursive(QVector<int> &part_ids) const;
    void create_tree_view_items_recursive(QTreeWidgetItem *treeview_item, QString root_string);
    QMap<QString, PartCategoryTreeNode> m_children;
    PartCategoryTreeNode &get_categorie_by_path_recursion(QStringList path, int depth);

    QString m_name;
    QList<QStringList> m_description_validator;
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

    int qty;
};

class FlatCategory {
    public:
    QString m_name;
    QList<QStringList> m_description_validators;
};

class PartDataBase {
    public:
    PartDataBase(QString file_name);

    int insert_new_part(Part new_part);

    void save_to_file();
    QMap<int, Part> get_parts_by_categorie(QString categorie_root);

    PartCategoryTreeNode get_category_node(QString categorie_path);
    PartCategoryTreeNode &get_category_node_ref(QString categorie_path);

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
    void load_categories_recursive(PartCategoryTreeNode &categories_recursion, QString root, const QList<FlatCategory> &flat_categorie);

    QList<FlatCategory> get_categories_by_json(QString categorie_root);

    QString m_filename;
};

#endif // DATABASE_H
