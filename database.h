#ifndef DATABASE_H
#define DATABASE_H
#include <QJsonObject>
#include <QString>

class PartDataBase {
public:
  PartDataBase(QString file_name);

private:
  QString m_file_name;
  QJsonObject m_json_data;
};

#endif // DATABASE_H
