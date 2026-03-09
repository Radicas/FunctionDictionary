/**
 * @file aiconfig.h
 * @brief AI配置数据模型定义
 * @author Developer
 * @date 2026-02-28
 * @version 2.0
 */

#ifndef AICONFIG_H
#define AICONFIG_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief AI配置结构体
 */
struct AIConfig {
    QString configName;        ///< 配置名称
    QString provider;          ///< API提供商（如OpenAI Compatible）
    QString baseUrl;           ///< API基础URL
    QString apiKey;            ///< API密钥
    QString defaultModel;      ///< 默认选中的模型
    QStringList modelList;     ///< 支持的模型列表

    /**
     * @brief 默认构造函数
     */
    AIConfig() : provider("其他") {}

    /**
     * @brief 从JSON对象加载配置
     * @param json JSON对象
     */
    void fromJson(const QJsonObject& json) {
        configName = json["configName"].toString();
        provider = json["provider"].toString("其他");
        baseUrl = json["baseUrl"].toString();
        apiKey = json["apiKey"].toString();
        defaultModel = json["defaultModel"].toString();
        
        modelList.clear();
        QJsonArray modelsArray = json["modelList"].toArray();
        for (const QJsonValue& value : modelsArray) {
            modelList.append(value.toString());
        }
    }

    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["configName"] = configName;
        json["provider"] = provider;
        json["baseUrl"] = baseUrl;
        json["apiKey"] = apiKey;
        json["defaultModel"] = defaultModel;
        
        QJsonArray modelsArray;
        for (const QString& model : modelList) {
            modelsArray.append(model);
        }
        json["modelList"] = modelsArray;
        
        return json;
    }
};

#endif
