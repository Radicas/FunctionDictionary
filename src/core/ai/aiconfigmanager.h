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
    static AIConfigManager& instance();
    AIConfigManager(const AIConfigManager&) = delete;
    AIConfigManager& operator=(const AIConfigManager&) = delete;

    void saveConfig(const AIConfig& config);
    AIConfig loadConfig();
    bool isConfigValid(const AIConfig& config) const;
    AIConfig getCurrentConfig() const;

private:
    AIConfigManager();
    ~AIConfigManager();

    mutable QMutex m_mutex;
    QSettings* m_settings;
    AIConfig m_currentConfig;
};

#endif
