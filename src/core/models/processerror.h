/**
 * @file processerror.h
 * @brief 错误处理数据模型
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef PROCESSERROR_H
#define PROCESSERROR_H

#include <QString>

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
 * @brief 错误处理动作
 */
enum class ErrorAction {
    Retry,          ///< 重试
    Skip,           ///< 跳过
    Abort,          ///< 中止
    Pause,          ///< 暂停等待用户确认
    SaveAndContinue ///< 保存状态并继续下一个
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
     * @brief 构造函数
     */
    ProcessError()
        : type(ProcessErrorType::None)
        , retryCount(0)
        , recoverable(true) {}

    /**
     * @brief 构造函数
     * @param t 错误类型
     * @param msg 错误消息
     * @param funcName 函数名
     */
    ProcessError(ProcessErrorType t, const QString& msg, const QString& funcName = "")
        : type(t)
        , message(msg)
        , functionName(funcName)
        , retryCount(0)
        , recoverable(true) {}
};

#endif // PROCESSERROR_H
