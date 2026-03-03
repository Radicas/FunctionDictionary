/**
 * @file batchconfig.h
 * @brief 批量处理配置和数据模型定义
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef BATCHCONFIG_H
#define BATCHCONFIG_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QQueue>
#include "core/models/extractedfunction.h"

/**
 * @brief 批量处理状态枚举
 */
enum class BatchProcessState {
    Idle,       ///< 空闲
    Running,    ///< 运行中
    Paused,     ///< 已暂停
    Completed,  ///< 已完成
    Cancelled   ///< 已取消
};

/**
 * @brief 批量处理配置结构
 */
struct BatchProcessConfig {
    int maxConcurrentRequests = 1;    ///< 最大并发请求数
    int requestTimeout = 60000;       ///< 请求超时时间（毫秒）
    int maxRetryCount = 3;            ///< 最大重试次数
    int retryDelay = 2000;            ///< 重试延迟（毫秒）
    int requestInterval = 1000;       ///< 请求间隔（毫秒，用于速率限制）
    bool skipExisting = true;         ///< 是否跳过已存在的函数
    bool enableCheckpoint = true;     ///< 是否启用断点续传
    
    /**
     * @brief 默认构造函数
     */
    BatchProcessConfig() = default;
};

/**
 * @brief 处理状态记录
 */
struct ProcessStateRecord {
    QString filePath;                 ///< 文件路径
    QString functionName;             ///< 函数名
    QString status;                   ///< 状态（pending, processing, completed, failed）
    int retryCount;                   ///< 重试次数
    QString errorMessage;             ///< 错误信息
    QDateTime createTime;             ///< 创建时间
    QDateTime updateTime;             ///< 更新时间
    
    /**
     * @brief 默认构造函数
     */
    ProcessStateRecord()
        : retryCount(0)
        , createTime(QDateTime::currentDateTime())
        , updateTime(QDateTime::currentDateTime())
    {}
};

/**
 * @brief 请求队列状态
 */
struct RequestQueueStatus {
    int totalRequests;                ///< 总请求数
    int pendingRequests;              ///< 待处理请求数
    int activeRequests;               ///< 活跃请求数
    int completedRequests;            ///< 已完成请求数
    int failedRequests;               ///< 失败请求数
    QDateTime startTime;              ///< 开始时间
    qint64 elapsedTime;               ///< 已用时间（毫秒）
    
    /**
     * @brief 默认构造函数
     */
    RequestQueueStatus()
        : totalRequests(0)
        , pendingRequests(0)
        , activeRequests(0)
        , completedRequests(0)
        , failedRequests(0)
        , elapsedTime(0)
    {}
};

/**
 * @brief AI分析请求结构
 */
struct AIAnalysisRequest {
    QString requestId;                  ///< 请求唯一标识
    ExtractedFunction function;         ///< 待分析的函数
    int priority;                       ///< 优先级
    QDateTime createTime;               ///< 创建时间
    int retryCount;                     ///< 已重试次数
    
    /**
     * @brief 默认构造函数
     */
    AIAnalysisRequest()
        : priority(0)
        , createTime(QDateTime::currentDateTime())
        , retryCount(0)
    {}
};

/**
 * @brief AI分析响应结构
 */
struct AIAnalysisResponse {
    QString requestId;                  ///< 请求ID
    bool success;                       ///< 是否成功
    QString functionName;               ///< 函数名称
    QString functionDescription;        ///< 函数描述（Markdown）
    QString signature;                  ///< 函数签名
    QString returnType;                 ///< 返回类型
    QString parameters;                 ///< 参数列表（JSON格式）
    QString flowchart;                  ///< 流程图
    QString sequenceDiagram;            ///< 时序图
    QString structureDiagram;           ///< 结构图
    QString errorMessage;               ///< 错误信息
    int retryCount;                     ///< 重试次数
    qint64 responseTime;                ///< 响应时间（毫秒）
    QString aiModel;                    ///< 使用的AI模型
    QDateTime analyzeTime;              ///< 分析时间
    
    /**
     * @brief 默认构造函数
     */
    AIAnalysisResponse()
        : success(false)
        , retryCount(0)
        , responseTime(0)
        , analyzeTime(QDateTime::currentDateTime())
    {}
};

/**
 * @brief 错误类型枚举
 */
enum class ProcessErrorType {
    None,                   ///< 无错误
    FileNotFound,           ///< 文件不存在
    FileReadError,          ///< 文件读取错误
    ParseError,             ///< 解析错误
    AIConfigError,          ///< AI配置错误
    AIRequestError,         ///< AI请求错误
    AIRateLimitError,       ///< AI速率限制错误
    AITimeoutError,         ///< AI超时错误
    AIResponseError,        ///< AI响应错误
    DatabaseError,          ///< 数据库错误
    DuplicateError,         ///< 重复错误
    UnknownError            ///< 未知错误
};

/**
 * @brief 错误信息结构
 */
struct ProcessError {
    ProcessErrorType type;              ///< 错误类型
    QString message;                    ///< 错误消息
    QString functionName;               ///< 相关函数名
    int retryCount;                     ///< 重试次数
    bool recoverable;                   ///< 是否可恢复
    
    /**
     * @brief 默认构造函数
     */
    ProcessError()
        : type(ProcessErrorType::None)
        , retryCount(0)
        , recoverable(true)
    {}
};

/**
 * @brief 错误处理动作枚举
 */
enum class ErrorAction {
    Retry,          ///< 重试
    Skip,           ///< 跳过
    Abort,          ///< 中止
    Pause,          ///< 暂停等待用户确认
    SaveAndContinue ///< 保存状态并继续下一个
};

#endif // BATCHCONFIG_H

