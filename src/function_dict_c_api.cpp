/**
 * @file function_dict_c_api.cpp
 * @brief 函数字典C语言接口实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "function_dict_c_api.h"
#include "databasemanager.h"
#include "logger.h"
#include <QString>
#include <QVector>
#include <QMutex>

namespace {
    QMutex g_mutex;
    QString g_lastError;
}

int function_dict_init(const char* db_path) {
    QMutexLocker locker(&g_mutex);

    if (db_path == nullptr) {
        g_lastError = "数据库路径不能为空";
        Logger::instance().error(g_lastError);
        return -1;
    }

    QString dbPath = QString::fromUtf8(db_path);
    if (DatabaseManager::instance().init(dbPath)) {
        return 0;
    } else {
        g_lastError = DatabaseManager::instance().lastError();
        return -1;
    }
}

int function_dict_is_initialized() {
    QMutexLocker locker(&g_mutex);
    return DatabaseManager::instance().isInitialized() ? 1 : 0;
}

int function_dict_add_function(const char* key, const char* value) {
    QMutexLocker locker(&g_mutex);

    if (key == nullptr) {
        g_lastError = "函数名称不能为空";
        Logger::instance().error(g_lastError);
        return -1;
    }

    if (value == nullptr) {
        g_lastError = "函数介绍不能为空";
        Logger::instance().error(g_lastError);
        return -1;
    }

    QString qKey = QString::fromUtf8(key);
    QString qValue = QString::fromUtf8(value);

    if (DatabaseManager::instance().addFunction(qKey, qValue)) {
        return 0;
    } else {
        g_lastError = DatabaseManager::instance().lastError();
        return -1;
    }
}

int function_dict_delete_function(int id) {
    QMutexLocker locker(&g_mutex);

    if (DatabaseManager::instance().deleteFunction(id)) {
        return 0;
    } else {
        g_lastError = DatabaseManager::instance().lastError();
        return -1;
    }
}

int function_dict_delete_functions(const int* ids, int count) {
    QMutexLocker locker(&g_mutex);

    if (ids == nullptr || count <= 0) {
        g_lastError = "ID数组无效";
        Logger::instance().error(g_lastError);
        return -1;
    }

    QVector<int> idVector;
    idVector.reserve(count);
    for (int i = 0; i < count; ++i) {
        idVector.append(ids[i]);
    }

    if (DatabaseManager::instance().deleteFunctions(idVector)) {
        return 0;
    } else {
        g_lastError = DatabaseManager::instance().lastError();
        return -1;
    }
}

int function_dict_function_exists(const char* key) {
    QMutexLocker locker(&g_mutex);

    if (key == nullptr) {
        return 0;
    }

    QString qKey = QString::fromUtf8(key);
    return DatabaseManager::instance().functionExists(qKey) ? 1 : 0;
}

const char* function_dict_get_last_error() {
    QMutexLocker locker(&g_mutex);
    static QByteArray lastErrorUtf8;
    lastErrorUtf8 = g_lastError.toUtf8();
    return lastErrorUtf8.isEmpty() ? nullptr : lastErrorUtf8.constData();
}

void function_dict_cleanup() {
    QMutexLocker locker(&g_mutex);
    g_lastError.clear();
    Logger::instance().info("C接口资源已清理");
}
