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

/**
 * @brief AI配置结构体
 */
struct AIConfig {
    QString provider;      ///< API提供商（如OpenAI Compatible）
    QString baseUrl;       ///< API基础URL
    QString apiKey;        ///< API密钥
    QString modelId;       ///< 模型ID
};

/**
 * @brief AI配置管理器类
 */
class AIConfigManager {
public:
    /**
     * @brief 获取单例实例
     * @return AIConfigManager单例引用
     */
    static AIConfigManager& instance();

    /**
     * @brief 禁止拷贝构造
     */
    AIConfigManager(const AIConfigManager&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    AIConfigManager& operator=(const AIConfigManager&) = delete;

    /**
     * @brief 保存AI配置
     * @param config 要保存的AI配置
     */
    void saveConfig(const AIConfig& config);

    /**
     * @brief 加载AI配置
     * @return 加载的AI配置
     */
    AIConfig loadConfig();

    /**
     * @brief 检查配置是否有效
     * @param config 要检查的配置
     * @return 配置是否有效
     */
    bool isConfigValid(const AIConfig& config) const;

    /**
     * @brief 获取当前配置
     * @return 当前AI配置
     */
    AIConfig getCurrentConfig() const;

private:
    /**
     * @brief 私有构造函数
     */
    AIConfigManager();

    /**
     * @brief 私有析构函数
     */
    ~AIConfigManager();

    mutable QMutex m_mutex;   ///< 互斥锁，用于线程安全
    QSettings* m_settings;    ///< 设置对象
    AIConfig m_currentConfig; ///< 当前配置
};

#endif // AICONFIGMANAGER_H
