/**
 * @file aiconfigmanager.cpp
 * @brief AI配置管理器实现，支持多配置管理和JSON存储
 * @author Developer
 * @date 2026-02-28
 * @version 2.0
 */

#include "core/ai/aiconfigmanager.h"
#include "common/logger/logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AIConfigManager::AIConfigManager() {
    m_configDir = QCoreApplication::applicationDirPath() + "/config/ai_config";
    ensureConfigDirExists();
    loadAllConfigs();
    Logger::instance().info("AI配置管理器初始化完成，配置目录: " + m_configDir);
}

AIConfigManager::~AIConfigManager() {
    saveAllConfigs();
}

AIConfigManager& AIConfigManager::instance() {
    static AIConfigManager instance;
    return instance;
}

void AIConfigManager::ensureConfigDirExists() {
    QDir dir(m_configDir);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            Logger::instance().info("创建AI配置目录成功: " + m_configDir);
        } else {
            Logger::instance().error("创建AI配置目录失败: " + m_configDir);
        }
    }
}

QString AIConfigManager::getConfigFilePath() {
    return m_configDir + "/ai_configs.json";
}

void AIConfigManager::saveConfig(const QString& configName, const AIConfig& config) {
    QMutexLocker locker(&m_mutex);
    
    m_configs[configName] = config;
    m_configs[configName].configName = configName;
    
    saveAllConfigs();
    Logger::instance().info("保存AI配置成功: " + configName);
}

AIConfig AIConfigManager::loadConfig(const QString& configName) {
    QMutexLocker locker(&m_mutex);
    
    if (m_configs.contains(configName)) {
        return m_configs[configName];
    }
    
    Logger::instance().warning("未找到AI配置: " + configName);
    return AIConfig();
}

bool AIConfigManager::deleteConfig(const QString& configName) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_configs.contains(configName)) {
        Logger::instance().warning("删除配置失败，配置不存在: " + configName);
        return false;
    }
    
    m_configs.remove(configName);
    
    if (m_currentConfigName == configName) {
        if (!m_configs.isEmpty()) {
            m_currentConfigName = m_configs.keys().first();
        } else {
            m_currentConfigName.clear();
        }
    }
    
    saveAllConfigs();
    Logger::instance().info("删除AI配置成功: " + configName);
    return true;
}

QStringList AIConfigManager::getAllConfigNames() {
    QMutexLocker locker(&m_mutex);
    return m_configs.keys();
}

bool AIConfigManager::hasConfig(const QString& configName) {
    QMutexLocker locker(&m_mutex);
    return m_configs.contains(configName);
}

bool AIConfigManager::setCurrentConfig(const QString& configName) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_configs.contains(configName)) {
        Logger::instance().warning("设置当前配置失败，配置不存在: " + configName);
        return false;
    }
    
    m_currentConfigName = configName;
    saveAllConfigs();
    Logger::instance().info("切换当前AI配置为: " + configName);
    return true;
}

AIConfig AIConfigManager::getCurrentConfig() {
    QMutexLocker locker(&m_mutex);
    
    if (m_currentConfigName.isEmpty() || !m_configs.contains(m_currentConfigName)) {
        return AIConfig();
    }
    
    return m_configs[m_currentConfigName];
}

QString AIConfigManager::getCurrentConfigName() {
    QMutexLocker locker(&m_mutex);
    return m_currentConfigName;
}

bool AIConfigManager::isConfigValid(const AIConfig& config) const {
    return !config.baseUrl.isEmpty() && 
           !config.apiKey.isEmpty() && 
           !config.defaultModel.isEmpty();
}

void AIConfigManager::saveAllConfigs() {
    QString filePath = getConfigFilePath();
    
    QJsonObject rootObj;
    rootObj["currentConfig"] = m_currentConfigName;
    
    QJsonObject configsObj;
    for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
        configsObj[it.key()] = it.value().toJson();
    }
    rootObj["configs"] = configsObj;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance().error("无法打开配置文件进行写入: " + filePath);
        return;
    }
    
    QJsonDocument doc(rootObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    Logger::instance().info("保存所有AI配置到文件成功: " + filePath);
}

void AIConfigManager::loadAllConfigs() {
    QString filePath = getConfigFilePath();
    
    QFile file(filePath);
    if (!file.exists()) {
        Logger::instance().info("配置文件不存在，将创建新文件: " + filePath);
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance().error("无法打开配置文件进行读取: " + filePath);
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        Logger::instance().error("解析配置文件失败: " + parseError.errorString());
        return;
    }
    
    QJsonObject rootObj = doc.object();
    m_currentConfigName = rootObj["currentConfig"].toString();
    
    m_configs.clear();
    QJsonObject configsObj = rootObj["configs"].toObject();
    for (auto it = configsObj.begin(); it != configsObj.end(); ++it) {
        AIConfig config;
        config.fromJson(it.value().toObject());
        m_configs[it.key()] = config;
    }
    
    Logger::instance().info("从文件加载AI配置成功，共加载 " + QString::number(m_configs.size()) + " 个配置");
}
