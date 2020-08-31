#ifndef DATABASE_H
#define DATABASE_H
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <exception>

class DataBaseException : public std::runtime_error {
public:
  DataBaseException(const QString &str)
      : std::runtime_error(str.toStdString()) {}

  int get_error_number() const { return error_number; }

private:
  int error_number;
};

class Part {
public:
  int id;
  QString sku;
  QString supplier;
  QString mpn;
  QString manufacturer;
  QString category;
  QString description;
  QString datasheet_link;
  QString lot;

  int qty;
};

class PartCategoryTreeNode {
public:
  PartCategoryTreeNode(){};
  PartCategoryTreeNode(QString name) : m_name(name) {}
  void clear() { m_children.clear(); }
  PartCategoryTreeNode &get_child(const QString &node_name) {
    return m_children[node_name];
  }
  void insert_child(PartCategoryTreeNode &child) {
    m_children.insert(child.m_name, child);
  }
  QStringList get_children_names() { return m_children.keys(); }
  QVector<int> m_part_ids;
  PartCategoryTreeNode &get_category(QString categorie_path);
  static QStringList split_category_path(QString path) {
    auto result = path.split("/", Qt::SkipEmptyParts);
    return result;
  }
  void get_partids_of_subcategories_recursive(QVector<int> &part_ids) const;

private:
  QMap<QString, PartCategoryTreeNode> m_children;
  PartCategoryTreeNode &get_categorie_by_path_recursion(QStringList path,
                                                        int depth);

  QString m_name;
};

class PartDataBase {
public:
  PartDataBase(QString file_name);

  QMap<int, Part> get_parts_by_categorie(QString categorie_root);

  PartCategoryTreeNode get_category_node(QString categorie_path);
  PartCategoryTreeNode &get_category_node_ref(QString categorie_path);

private:
  void
  get_partids_of_subcategories_recursive(QList<int> &part_ids,
                                         PartCategoryTreeNode category_node);
  QString m_file_name;
  QJsonObject m_json_data;
  QMap<int, Part> m_parts;
  PartCategoryTreeNode m_category_nodes;
  void load_categories_recursive(PartCategoryTreeNode &categories_recursion,
                                 QString root,
                                 const QStringList &categorie_names);

  QStringList get_categories_by_json(QString categorie_root);
};

#endif // DATABASE_H
