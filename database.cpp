#include "database.h"
#include <QFile>
#include <QJsonDocument>

PartDataBase::PartDataBase(QString file_name) : m_file_name(file_name) {
  QFile loadFile(m_file_name);
  loadFile.open(QIODevice::ReadOnly);
  QByteArray raw_data = loadFile.readAll();
  QJsonDocument loadDoc(QJsonDocument::fromJson(raw_data));
  m_json_data = loadDoc.object();
}
