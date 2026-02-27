/**
 * @file databasemanager.h
 * @brief 数据库管理模块，负责SQLite数据库的操作
 * @author Developer
 * @date 2026-02-27
 * @version 1.0
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVector>
#include <QPair>

/**
 * @brief 函数数据结构
 */
struct FunctionData {
    int id;                 ///< 函数ID
    QString key;            ///< 函数名称（唯一标识）
    QString value;          ///< 函数介绍（Markdown格式）
    QDateTime createTime;   ///< 创建时间
};

/**
 * @brief 数据库管理类，提供SQLite数据库操作功能
 */
class DatabaseManager {
public:
    /**
     * @brief 获取DatabaseManager单例实例
     * @return DatabaseManager单例引用
     */
    static DatabaseManager& instance();

    /**
     * @brief 初始化数据库
     * @param dbPath 数据库文件路径
     * @return 初始化是否成功
     */
    bool init(const QString& dbPath);

    /**
     * @brief 检查数据库是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 添加函数
     * @param key 函数名称
     * @param value 函数介绍
     * @return 添加是否成功
     */
    bool addFunction(const QString& key, const QString& value);

    /**
     * @brief 删除函数
     * @param id 函数ID
     * @return 删除是否成功
     */
    bool deleteFunction(int id);

    /**
     * @brief 删除多个函数
     * @param ids 函数ID列表
     * @return 删除是否成功
     */
    bool deleteFunctions(const QVector<int>& ids);

    /**
     * @brief 获取所有函数
     * @return 函数数据列表
     */
    QVector<FunctionData> getAllFunctions();

    /**
     * @brief 根据ID获取函数
     * @param id 函数ID
     * @return 函数数据，若不存在则返回空数据
     */
    FunctionData getFunctionById(int id);

    /**
     * @brief 根据函数名称获取函数
     * @param key 函数名称
     * @return 函数数据，若不存在则返回空数据
     */
    FunctionData getFunctionByKey(const QString& key);

    /**
     * @brief 检查函数名称是否存在
     * @param key 函数名称
     * @return 是否存在
     */
    bool functionExists(const QString& key);

    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    QString lastError() const;

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief 创建数据库表
     * @return 创建是否成功
     */
    bool createTables();

    QSqlDatabase m_db;       ///< 数据库连接
    bool m_initialized;       ///< 初始化标志
    QString m_lastError;      ///< 最后一次错误信息
};

#endif // DATABASEMANAGER_H
