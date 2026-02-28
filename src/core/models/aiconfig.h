/**
 * @file aiconfig.h
 * @brief AI配置数据模型定义
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef AICONFIG_H
#define AICONFIG_H

#include <QString>

/**
 * @brief AI配置结构体
 */
struct AIConfig {
    QString provider;      ///< API提供商（如OpenAI Compatible）
    QString baseUrl;       ///< API基础URL
    QString apiKey;        ///< API密钥
    QString modelId;       ///< 模型ID
};

#endif
