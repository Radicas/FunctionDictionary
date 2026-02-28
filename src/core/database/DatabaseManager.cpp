#include "databasemanager.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::DatabaseManager() : m_initialized(false) {
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::init(const QString& dbPath) {
    if (m_initialized) {
        return true;
    }

    QFileInfo fileInfo(dbPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = "无法创建数据库目录: " + dir.path();
            Logger::instance().error(m_lastError);
            return false;
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = "无法打开数据库: " + m_db.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (!createTables()) {
        m_db.close();
        return false;
    }

    m_initialized = true;
    Logger::instance().info("数据库初始化成功: " + dbPath);
    return true;
}

bool DatabaseManager::isInitialized() const {
    return m_initialized;
}

bool DatabaseManager::createTables() {
    QSqlQuery query;
    QString createTableSql = "CREATE TABLE IF NOT EXISTS functions ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "key TEXT NOT NULL UNIQUE, "
                              "value TEXT NOT NULL, "
                              "create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
                              ")";

    if (!query.exec(createTableSql)) {
        m_lastError = "创建表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    return true;
}

bool DatabaseManager::addFunction(const QString& key, const QString& value) {
    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (key.trimmed().isEmpty()) {
        m_lastError = "函数名称不能为空";
        Logger::instance().warning(m_lastError);
        return false;
    }

    if (functionExists(key)) {
        m_lastError = "函数名称已存在: " + key;
        Logger::instance().warning(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO functions (key, value, create_time) VALUES (?, ?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(QDateTime::currentDateTime());

    if (!query.exec()) {
        m_lastError = "添加函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    Logger::instance().info("添加函数成功: " + key);
    return true;
}

bool DatabaseManager::deleteFunction(int id) {
    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM functions WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        m_lastError = "删除函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (query.numRowsAffected() == 0) {
        m_lastError = "函数不存在，ID: " + QString::number(id);
        Logger::instance().warning(m_lastError);
        return false;
    }

    Logger::instance().info("删除函数成功，ID: " + QString::number(id));
    return true;
}

bool DatabaseManager::deleteFunctions(const QVector<int>& ids) {
    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (ids.isEmpty()) {
        m_lastError = "没有选择要删除的函数";
        Logger::instance().warning(m_lastError);
        return false;
    }

    m_db.transaction();

    for (int id : ids) {
        QSqlQuery query;
        query.prepare("DELETE FROM functions WHERE id = ?");
        query.addBindValue(id);

        if (!query.exec()) {
            m_db.rollback();
            m_lastError = "删除函数失败，ID: " + QString::number(id) + " - " + query.lastError().text();
            Logger::instance().error(m_lastError);
            return false;
        }
    }

    m_db.commit();
    Logger::instance().info("批量删除函数成功，共删除 " + QString::number(ids.size()) + " 个函数");
    return true;
}

QVector<FunctionData> DatabaseManager::getAllFunctions() {
    QVector<FunctionData> functions;

    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return functions;
    }

    QSqlQuery query("SELECT id, key, value, create_time FROM functions ORDER BY key ASC");

    while (query.next()) {
        FunctionData data;
        data.id = query.value(0).toInt();
        data.key = query.value(1).toString();
        data.value = query.value(2).toString();
        data.createTime = query.value(3).toDateTime();
        functions.append(data);
    }

    if (query.lastError().isValid()) {
        m_lastError = "获取函数列表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return functions;
}

FunctionData DatabaseManager::getFunctionById(int id) {
    FunctionData data;
    data.id = -1;

    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return data;
    }

    QSqlQuery query;
    query.prepare("SELECT id, key, value, create_time FROM functions WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        data.id = query.value(0).toInt();
        data.key = query.value(1).toString();
        data.value = query.value(2).toString();
        data.createTime = query.value(3).toDateTime();
    } else {
        m_lastError = "获取函数失败，ID: " + QString::number(id) + " - " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return data;
}

FunctionData DatabaseManager::getFunctionByKey(const QString& key) {
    FunctionData data;
    data.id = -1;

    if (!m_initialized) {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return data;
    }

    QSqlQuery query;
    query.prepare("SELECT id, key, value, create_time FROM functions WHERE key = ?");
    query.addBindValue(key);

    if (query.exec() && query.next()) {
        data.id = query.value(0).toInt();
        data.key = query.value(1).toString();
        data.value = query.value(2).toString();
        data.createTime = query.value(3).toDateTime();
    } else {
        m_lastError = "获取函数失败，Key: " + key + " - " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return data;
}

bool DatabaseManager::functionExists(const QString& key) {
    if (!m_initialized) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM functions WHERE key = ?");
    query.addBindValue(key);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QString DatabaseManager::lastError() const {
    return m_lastError;
}
