/**
 * @file aiconfigmanager.h
 * @brief AI配置管理器，负责保存和加载AI相关配置
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef AICONFIGMANAGER_H
#define AICONFIGMANAGER_H

#include <QString>
#include <QSettings>
#include <QMutex>
#include "../models/aiconfig.h"

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
     * @param config AI配置对象
     */
    void saveConfig(const AIConfig& config);

    /**
     * @brief 加载AI配置
     * @return 加载的AI配置对象
     */
    AIConfig loadConfig();

    /**
     * @brief 检查配置是否有效
     * @param config 要检查的AI配置对象
     * @return 配置是否有效
     */
    bool isConfigValid(const AIConfig& config) const;

    /**
     * @brief 获取当前配置
     * @return 当前AI配置对象
     */
    AIConfig getCurrentConfig() const;

private:
    /**
     * @brief 构造函数
     */
    AIConfigManager();

    /**
     * @brief 析构函数
     */
    ~AIConfigManager();

    mutable QMutex m_mutex;
    QSettings* m_settings;
    AIConfig m_currentConfig;
};

#endif
