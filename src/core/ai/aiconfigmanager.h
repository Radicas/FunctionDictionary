/**
 * @file aiconfigmanager.h
 * @brief AI配置管理器，负责保存和加载AI相关配置，支持多配置管理
 * @author Developer
 * @date 2026-02-28
 * @version 2.0
 */

#ifndef AICONFIGMANAGER_H
#define AICONFIGMANAGER_H

#include <QString>
#include <QMap>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include "core/models/aiconfig.h"

/**
 * @brief AI配置管理器类，单例模式
 */
class AIConfigManager {
public:
    /**
     * @brief 获取AIConfigManager的单例实例
     * @return AIConfigManager的引用
     */
    static AIConfigManager& instance();
    
    AIConfigManager(const AIConfigManager&) = delete;
    AIConfigManager& operator=(const AIConfigManager&) = delete;

    /**
     * @brief 保存AI配置
     * @param configName 配置名称
     * @param config AI配置对象
     */
    void saveConfig(const QString& configName, const AIConfig& config);

    /**
     * @brief 加载指定名称的AI配置
     * @param configName 配置名称
     * @return 加载的AI配置对象
     */
    AIConfig loadConfig(const QString& configName);

    /**
     * @brief 删除指定名称的AI配置
     * @param configName 配置名称
     * @return 是否删除成功
     */
    bool deleteConfig(const QString& configName);

    /**
     * @brief 获取所有配置名称列表
     * @return 配置名称列表
     */
    QStringList getAllConfigNames();

    /**
     * @brief 检查配置是否存在
     * @param configName 配置名称
     * @return 是否存在
     */
    bool hasConfig(const QString& configName);

    /**
     * @brief 设置当前激活的配置
     * @param configName 配置名称
     * @return 是否设置成功
     */
    bool setCurrentConfig(const QString& configName);

    /**
     * @brief 获取当前激活的配置
     * @return 当前AI配置对象
     */
    AIConfig getCurrentConfig();

    /**
     * @brief 获取当前激活的配置名称
     * @return 当前配置名称
     */
    QString getCurrentConfigName();

    /**
     * @brief 检查配置是否有效
     * @param config 要检查的AI配置对象
     * @return 配置是否有效
     */
    bool isConfigValid(const AIConfig& config) const;

    /**
     * @brief 保存所有配置到文件
     */
    void saveAllConfigs();

    /**
     * @brief 从文件加载所有配置
     */
    void loadAllConfigs();

private:
    /**
     * @brief 构造函数
     */
    AIConfigManager();

    /**
     * @brief 析构函数
     */
    ~AIConfigManager();

    /**
     * @brief 确保配置目录存在
     */
    void ensureConfigDirExists();

    /**
     * @brief 获取配置文件路径
     * @return 配置文件完整路径
     */
    QString getConfigFilePath();

    mutable QMutex m_mutex;
    QMap<QString, AIConfig> m_configs;
    QString m_currentConfigName;
    QString m_configDir;
};

#endif
