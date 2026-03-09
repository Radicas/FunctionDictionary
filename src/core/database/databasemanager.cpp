#include "core/database/databasemanager.h"
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include "common/logger/logger.h"

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::DatabaseManager() : m_initialized(false) {}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool DatabaseManager::init(const QString& dbPath)
{
    if (m_initialized)
    {
        return true;
    }

    QFileInfo fileInfo(dbPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            m_lastError = "无法创建数据库目录: " + dir.path();
            Logger::instance().error(m_lastError);
            return false;
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open())
    {
        m_lastError = "无法打开数据库: " + m_db.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (!createTables())
    {
        m_db.close();
        return false;
    }

    if (!migrateTables())
    {
        m_db.close();
        return false;
    }

    m_initialized = true;
    Logger::instance().info("数据库初始化成功: " + dbPath);
    return true;
}

bool DatabaseManager::isInitialized() const
{
    return m_initialized;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;

    QString createProjectsSql =
        "CREATE TABLE IF NOT EXISTS projects ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "root_path TEXT NOT NULL UNIQUE, "
        "description TEXT, "
        "create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(createProjectsSql))
    {
        m_lastError = "创建projects表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    QString createFunctionsSql =
        "CREATE TABLE IF NOT EXISTS functions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "project_id INTEGER, "
        "key TEXT NOT NULL, "
        "value TEXT NOT NULL, "
        "signature TEXT, "
        "return_type TEXT, "
        "parameters TEXT, "
        "file_path TEXT, "
        "start_line INTEGER, "
        "end_line INTEGER, "
        "language TEXT, "
        "flowchart TEXT, "
        "sequence_diagram TEXT, "
        "structure_diagram TEXT, "
        "ai_model TEXT, "
        "create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "analyze_time DATETIME, "
        "UNIQUE(key, file_path), "
        "FOREIGN KEY(project_id) REFERENCES projects(id) ON DELETE CASCADE"
        ")";

    if (!query.exec(createFunctionsSql))
    {
        m_lastError = "创建functions表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    QString createProcessStateSql =
        "CREATE TABLE IF NOT EXISTS process_state ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "file_path TEXT NOT NULL, "
        "function_name TEXT NOT NULL, "
        "status TEXT NOT NULL, "
        "retry_count INTEGER DEFAULT 0, "
        "error_message TEXT, "
        "create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "UNIQUE(file_path, function_name)"
        ")";

    if (!query.exec(createProcessStateSql))
    {
        m_lastError = "创建process_state表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    QString createIndexKeySql = "CREATE INDEX IF NOT EXISTS idx_functions_key ON functions(key)";
    query.exec(createIndexKeySql);

    QString createIndexFilePathSql = "CREATE INDEX IF NOT EXISTS idx_functions_file_path ON functions(file_path)";
    query.exec(createIndexFilePathSql);

    QString createIndexLanguageSql = "CREATE INDEX IF NOT EXISTS idx_functions_language ON functions(language)";
    query.exec(createIndexLanguageSql);

    return true;
}

bool DatabaseManager::migrateTables()
{
    return checkAndAddMissingColumns();
}

bool DatabaseManager::checkAndAddMissingColumns()
{
    QSqlQuery query;

    QStringList requiredColumns = {"project_id INTEGER",    "signature TEXT",         "return_type TEXT",
                                   "parameters TEXT",       "file_path TEXT",         "start_line INTEGER",
                                   "end_line INTEGER",      "language TEXT",          "flowchart TEXT",
                                   "sequence_diagram TEXT", "structure_diagram TEXT", "ai_model TEXT",
                                   "analyze_time DATETIME"};

    for (const QString& columnDef : requiredColumns)
    {
        QString columnName = columnDef.split(" ").first();

        query.prepare("PRAGMA table_info(functions)");
        if (!query.exec())
        {
            m_lastError = "检查表结构失败: " + query.lastError().text();
            Logger::instance().error(m_lastError);
            return false;
        }

        bool columnExists = false;
        while (query.next())
        {
            if (query.value(1).toString() == columnName)
            {
                columnExists = true;
                break;
            }
        }

        if (!columnExists)
        {
            QString alterSql = QString("ALTER TABLE functions ADD COLUMN %1").arg(columnDef);
            if (!query.exec(alterSql))
            {
                m_lastError = QString("添加列 %1 失败: %2").arg(columnName).arg(query.lastError().text());
                Logger::instance().error(m_lastError);
                return false;
            }
            Logger::instance().info(QString("成功添加列: %1").arg(columnName));
        }
    }

    return true;
}

bool DatabaseManager::addFunction(const QString& key, const QString& value)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (key.trimmed().isEmpty())
    {
        m_lastError = "函数名称不能为空";
        Logger::instance().warning(m_lastError);
        return false;
    }

    if (functionExists(key))
    {
        m_lastError = "函数名称已存在: " + key;
        Logger::instance().warning(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO functions (key, value, create_time) VALUES (?, ?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(QDateTime::currentDateTime());

    if (!query.exec())
    {
        m_lastError = "添加函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    Logger::instance().info("添加函数成功: " + key);
    return true;
}

bool DatabaseManager::deleteFunction(int id)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM functions WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec())
    {
        m_lastError = "删除函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (query.numRowsAffected() == 0)
    {
        m_lastError = "函数不存在，ID: " + QString::number(id);
        Logger::instance().warning(m_lastError);
        return false;
    }

    Logger::instance().info("删除函数成功，ID: " + QString::number(id));
    return true;
}

bool DatabaseManager::deleteFunctions(const QVector<int>& ids)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (ids.isEmpty())
    {
        m_lastError = "没有选择要删除的函数";
        Logger::instance().warning(m_lastError);
        return false;
    }

    m_db.transaction();

    for (int id : ids)
    {
        QSqlQuery query;
        query.prepare("DELETE FROM functions WHERE id = ?");
        query.addBindValue(id);

        if (!query.exec())
        {
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

QVector<FunctionData> DatabaseManager::getAllFunctions()
{
    QVector<FunctionData> functions;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return functions;
    }

    QSqlQuery query("SELECT id, project_id, key, value, file_path, create_time FROM functions ORDER BY key ASC");

    while (query.next())
    {
        FunctionData data;
        data.id = query.value(0).toInt();
        data.projectId = query.value(1).toInt();
        data.key = query.value(2).toString();
        data.value = query.value(3).toString();
        data.filePath = query.value(4).toString();
        data.createTime = query.value(5).toDateTime();
        functions.append(data);
    }

    if (query.lastError().isValid())
    {
        m_lastError = "获取函数列表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return functions;
}

FunctionData DatabaseManager::getFunctionById(int id)
{
    FunctionData data;
    data.id = -1;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return data;
    }

    QSqlQuery query;
    query.prepare("SELECT id, project_id, key, value, file_path, create_time FROM functions WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next())
    {
        data.id = query.value(0).toInt();
        data.projectId = query.value(1).toInt();
        data.key = query.value(2).toString();
        data.value = query.value(3).toString();
        data.filePath = query.value(4).toString();
        data.createTime = query.value(5).toDateTime();
    }
    else
    {
        m_lastError = "获取函数失败，ID: " + QString::number(id) + " - " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return data;
}

FunctionData DatabaseManager::getFunctionByKey(const QString& key)
{
    FunctionData data;
    data.id = -1;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return data;
    }

    QSqlQuery query;
    query.prepare("SELECT id, project_id, key, value, file_path, create_time FROM functions WHERE key = ?");
    query.addBindValue(key);

    if (query.exec() && query.next())
    {
        data.id = query.value(0).toInt();
        data.projectId = query.value(1).toInt();
        data.key = query.value(2).toString();
        data.value = query.value(3).toString();
        data.filePath = query.value(4).toString();
        data.createTime = query.value(5).toDateTime();
    }
    else
    {
        m_lastError = "获取函数失败，Key: " + key + " - " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return data;
}

bool DatabaseManager::functionExists(const QString& key)
{
    if (!m_initialized)
    {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM functions WHERE key = ?");
    query.addBindValue(key);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::addFunction(const FunctionData& func)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (func.key.trimmed().isEmpty())
    {
        m_lastError = "函数名称不能为空";
        Logger::instance().warning(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT OR REPLACE INTO functions (project_id, key, value, signature, return_type, parameters, "
        "file_path, start_line, end_line, language, flowchart, sequence_diagram, "
        "structure_diagram, ai_model, create_time, analyze_time) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(func.projectId);
    query.addBindValue(func.key);
    query.addBindValue(func.value);
    query.addBindValue(func.signature);
    query.addBindValue(func.returnType);
    query.addBindValue(func.parameters);
    query.addBindValue(func.filePath);
    query.addBindValue(func.startLine);
    query.addBindValue(func.endLine);
    query.addBindValue(func.language);
    query.addBindValue(func.flowchart);
    query.addBindValue(func.sequenceDiagram);
    query.addBindValue(func.structureDiagram);
    query.addBindValue(func.aiModel);
    query.addBindValue(func.createTime);
    query.addBindValue(func.analyzeTime);

    if (!query.exec())
    {
        m_lastError = "添加函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    Logger::instance().info("添加函数成功: " + func.key);
    return true;
}

int DatabaseManager::addFunctionsBatch(const QVector<FunctionData>& functions)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return 0;
    }

    if (functions.isEmpty())
    {
        return 0;
    }

    int successCount = 0;
    m_db.transaction();

    for (const auto& func : functions)
    {
        if (addFunction(func))
        {
            successCount++;
        }
    }

    m_db.commit();
    Logger::instance().info(QString("批量添加函数完成，成功 %1/%2").arg(successCount).arg(functions.size()));
    return successCount;
}

bool DatabaseManager::upsertFunction(const FunctionData& func)
{
    return addFunction(func);
}

bool DatabaseManager::saveProcessState(const QString& filePath, const QString& functionName, const QString& status,
                                       const QString& errorMessage)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT OR REPLACE INTO process_state (file_path, function_name, status, error_message, update_time) "
        "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(filePath);
    query.addBindValue(functionName);
    query.addBindValue(status);
    query.addBindValue(errorMessage);
    query.addBindValue(QDateTime::currentDateTime());

    if (!query.exec())
    {
        m_lastError = "保存处理状态失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    return true;
}

QVector<ProcessStateRecord> DatabaseManager::getProcessState(const QString& filePath)
{
    QVector<ProcessStateRecord> records;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return records;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT file_path, function_name, status, retry_count, error_message, create_time, update_time "
        "FROM process_state WHERE file_path = ? ORDER BY create_time ASC");
    query.addBindValue(filePath);

    if (query.exec())
    {
        while (query.next())
        {
            ProcessStateRecord record;
            record.filePath = query.value(0).toString();
            record.functionName = query.value(1).toString();
            record.status = query.value(2).toString();
            record.retryCount = query.value(3).toInt();
            record.errorMessage = query.value(4).toString();
            record.createTime = query.value(5).toDateTime();
            record.updateTime = query.value(6).toDateTime();
            records.append(record);
        }
    }
    else
    {
        m_lastError = "获取处理状态失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return records;
}

bool DatabaseManager::clearProcessState(const QString& filePath)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM process_state WHERE file_path = ?");
    query.addBindValue(filePath);

    if (!query.exec())
    {
        m_lastError = "清除处理状态失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    return true;
}

QSet<QString> DatabaseManager::getProcessedFunctions(const QString& filePath)
{
    QSet<QString> functions;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return functions;
    }

    QSqlQuery query;
    query.prepare("SELECT function_name FROM process_state WHERE file_path = ? AND status = 'completed'");
    query.addBindValue(filePath);

    if (query.exec())
    {
        while (query.next())
        {
            functions.insert(query.value(0).toString());
        }
    }
    else
    {
        m_lastError = "获取已处理函数列表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return functions;
}

bool DatabaseManager::addProject(ProjectInfo& project)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    if (project.name.trimmed().isEmpty())
    {
        m_lastError = "项目名称不能为空";
        Logger::instance().warning(m_lastError);
        return false;
    }

    if (project.rootPath.trimmed().isEmpty())
    {
        m_lastError = "项目根路径不能为空";
        Logger::instance().warning(m_lastError);
        return false;
    }

    if (projectPathExists(project.rootPath))
    {
        m_lastError = "项目路径已存在: " + project.rootPath;
        Logger::instance().warning(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO projects (name, root_path, description, create_time, update_time) "
        "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(project.name);
    query.addBindValue(project.rootPath);
    query.addBindValue(project.description);
    query.addBindValue(project.createTime);
    query.addBindValue(project.updateTime);

    if (!query.exec())
    {
        m_lastError = "添加项目失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    project.id = query.lastInsertId().toInt();
    Logger::instance().info("添加项目成功: " + project.name);
    return true;
}

bool DatabaseManager::updateProject(const ProjectInfo& project)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE projects SET name = ?, root_path = ?, description = ?, update_time = ? WHERE id = ?");
    query.addBindValue(project.name);
    query.addBindValue(project.rootPath);
    query.addBindValue(project.description);
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(project.id);

    if (!query.exec())
    {
        m_lastError = "更新项目失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    Logger::instance().info("更新项目成功: " + project.name);
    return true;
}

bool DatabaseManager::deleteProject(int projectId)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    m_db.transaction();

    QSqlQuery deleteFunctionsQuery;
    deleteFunctionsQuery.prepare("DELETE FROM functions WHERE project_id = ?");
    deleteFunctionsQuery.addBindValue(projectId);
    if (!deleteFunctionsQuery.exec())
    {
        m_db.rollback();
        m_lastError = "删除项目函数失败: " + deleteFunctionsQuery.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery deleteProjectQuery;
    deleteProjectQuery.prepare("DELETE FROM projects WHERE id = ?");
    deleteProjectQuery.addBindValue(projectId);
    if (!deleteProjectQuery.exec())
    {
        m_db.rollback();
        m_lastError = "删除项目失败: " + deleteProjectQuery.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    m_db.commit();
    Logger::instance().info("删除项目成功，ID: " + QString::number(projectId));
    return true;
}

QVector<ProjectInfo> DatabaseManager::getAllProjects()
{
    QVector<ProjectInfo> projects;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return projects;
    }

    QSqlQuery query(
        "SELECT id, name, root_path, description, create_time, update_time FROM projects ORDER BY name ASC");

    while (query.next())
    {
        ProjectInfo project;
        project.id = query.value(0).toInt();
        project.name = query.value(1).toString();
        project.rootPath = query.value(2).toString();
        project.description = query.value(3).toString();
        project.createTime = query.value(4).toDateTime();
        project.updateTime = query.value(5).toDateTime();
        projects.append(project);
    }

    if (query.lastError().isValid())
    {
        m_lastError = "获取项目列表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return projects;
}

ProjectInfo DatabaseManager::getProjectById(int projectId)
{
    ProjectInfo project;
    project.id = -1;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return project;
    }

    QSqlQuery query;
    query.prepare("SELECT id, name, root_path, description, create_time, update_time FROM projects WHERE id = ?");
    query.addBindValue(projectId);

    if (query.exec() && query.next())
    {
        project.id = query.value(0).toInt();
        project.name = query.value(1).toString();
        project.rootPath = query.value(2).toString();
        project.description = query.value(3).toString();
        project.createTime = query.value(4).toDateTime();
        project.updateTime = query.value(5).toDateTime();
    }
    else
    {
        m_lastError = "获取项目失败，ID: " + QString::number(projectId) + " - " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return project;
}

bool DatabaseManager::projectPathExists(const QString& rootPath)
{
    if (!m_initialized)
    {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM projects WHERE root_path = ?");
    query.addBindValue(rootPath);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QVector<FunctionData> DatabaseManager::getFunctionsByProject(int projectId)
{
    QVector<FunctionData> functions;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return functions;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT id, project_id, key, value, file_path, create_time FROM functions WHERE project_id = ? ORDER BY key "
        "ASC");
    query.addBindValue(projectId);

    if (query.exec())
    {
        while (query.next())
        {
            FunctionData data;
            data.id = query.value(0).toInt();
            data.projectId = query.value(1).toInt();
            data.key = query.value(2).toString();
            data.value = query.value(3).toString();
            data.filePath = query.value(4).toString();
            data.createTime = query.value(5).toDateTime();
            functions.append(data);
        }
    }
    else
    {
        m_lastError = "获取项目函数列表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
    }

    return functions;
}

bool DatabaseManager::deleteFunctionsByProject(int projectId)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM functions WHERE project_id = ?");
    query.addBindValue(projectId);

    if (!query.exec())
    {
        m_lastError = "删除项目函数失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    Logger::instance().info("删除项目函数成功，项目ID: " + QString::number(projectId));
    return true;
}

bool DatabaseManager::functionExistsByKeyAndPath(const QString& key, const QString& filePath)
{
    if (!m_initialized)
    {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM functions WHERE key = ? AND file_path = ?");
    query.addBindValue(key);
    query.addBindValue(filePath);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseManager::clearAllData()
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    m_db.transaction();

    QSqlQuery query;
    if (!query.exec("DELETE FROM functions"))
    {
        m_db.rollback();
        m_lastError = "清空函数表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (!query.exec("DELETE FROM projects"))
    {
        m_db.rollback();
        m_lastError = "清空项目表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (!query.exec("DELETE FROM process_state"))
    {
        m_db.rollback();
        m_lastError = "清空处理状态表失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    m_db.commit();
    Logger::instance().info("清空所有数据成功");
    return true;
}

ProjectInfo DatabaseManager::getOrCreateTemporaryProject()
{
    ProjectInfo tempProject;
    tempProject.id = -1;

    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return tempProject;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT id, name, root_path, description, create_time, update_time FROM projects WHERE root_path = ?");
    query.addBindValue("__temporary__");

    if (query.exec() && query.next())
    {
        tempProject.id = query.value(0).toInt();
        tempProject.name = query.value(1).toString();
        tempProject.rootPath = query.value(2).toString();
        tempProject.description = query.value(3).toString();
        tempProject.createTime = query.value(4).toDateTime();
        tempProject.updateTime = query.value(5).toDateTime();
        return tempProject;
    }

    tempProject.name = "待整理";
    tempProject.rootPath = "__temporary__";
    tempProject.description = "未分配到具体项目的函数存放处";
    tempProject.createTime = QDateTime::currentDateTime();
    tempProject.updateTime = QDateTime::currentDateTime();

    if (addProject(tempProject))
    {
        Logger::instance().info("创建临时项目成功");
        return tempProject;
    }

    tempProject.id = -1;
    return tempProject;
}

bool DatabaseManager::isTemporaryProject(int projectId)
{
    if (!m_initialized)
    {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT root_path FROM projects WHERE id = ?");
    query.addBindValue(projectId);

    if (query.exec() && query.next())
    {
        return query.value(0).toString() == "__temporary__";
    }

    return false;
}

bool DatabaseManager::updateFunctionProject(int functionId, int newProjectId)
{
    if (!m_initialized)
    {
        m_lastError = "数据库未初始化";
        Logger::instance().error(m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE functions SET project_id = ? WHERE id = ?");
    query.addBindValue(newProjectId);
    query.addBindValue(functionId);

    if (!query.exec())
    {
        m_lastError = "更新函数项目失败: " + query.lastError().text();
        Logger::instance().error(m_lastError);
        return false;
    }

    if (query.numRowsAffected() == 0)
    {
        m_lastError = "函数不存在，ID: " + QString::number(functionId);
        Logger::instance().warning(m_lastError);
        return false;
    }

    Logger::instance().info(QString("更新函数项目成功，函数ID: %1, 新项目ID: %2").arg(functionId).arg(newProjectId));
    return true;
}
