/**
 * @file AIConfigManager.cpp
 * @brief AI配置管理器实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "aiconfigmanager.h"
#include "logger.h"
#include <QCoreApplication>

AIConfigManager::AIConfigManager()
    : m_settings(nullptr) {
    QString configPath = QCoreApplication::applicationDirPath() + "/ai_config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat);
    m_currentConfig = loadConfig();
    Logger::instance().info("AI配置管理器初始化完成");
}

AIConfigManager::~AIConfigManager() {
    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
}

AIConfigManager& AIConfigManager::instance() {
    static AIConfigManager instance;
    return instance;
}

void AIConfigManager::saveConfig(const AIConfig& config) {
    QMutexLocker locker(&m_mutex);

    m_settings->setValue("AI/provider", config.provider);
    m_settings->setValue("AI/baseUrl", config.baseUrl);
    m_settings->setValue("AI/apiKey", config.apiKey);
    m_settings->setValue("AI/modelId", config.modelId);
    m_settings->sync();

    m_currentConfig = config;
    Logger::instance().info("AI配置已保存");
}

AIConfig AIConfigManager::loadConfig() {
    QMutexLocker locker(&m_mutex);

    AIConfig config;
    config.provider = m_settings->value("AI/provider", "OpenAI Compatible").toString();
    config.baseUrl = m_settings->value("AI/baseUrl", "").toString();
    config.apiKey = m_settings->value("AI/apiKey", "").toString();
    config.modelId = m_settings->value("AI/modelId", "").toString();

    return config;
}

bool AIConfigManager::isConfigValid(const AIConfig& config) const {
    return !config.baseUrl.isEmpty() && 
           !config.apiKey.isEmpty() && 
           !config.modelId.isEmpty();
}

AIConfig AIConfigManager::getCurrentConfig() const {
    QMutexLocker locker(&m_mutex);
    return m_currentConfig;
}
