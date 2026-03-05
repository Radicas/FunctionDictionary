/**
 * @file errorhandler.h
 * @brief 错误处理器，负责处理各类错误
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>
#include <QMutex>
#include "core/models/processerror.h"
#include "common/logger/logger.h"

/**
 * @brief 错误处理器类
 * 
 * 该类负责处理各类错误，包括确定错误处理策略、
 * 计算重试延迟时间等功能。
 */
class ErrorHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取ErrorHandler的单例实例
     * @return ErrorHandler的引用
     */
    static ErrorHandler& instance();

    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;

    /**
     * @brief 处理错误
     * @param error 错误信息
     * @return 处理后的动作
     */
    ErrorAction handleError(const ProcessError& error);

    /**
     * @brief 获取重试延迟时间（指数退避）
     * @param retryCount 当前重试次数
     * @return 延迟时间（毫秒）
     */
    int getRetryDelay(int retryCount) const;

    /**
     * @brief 将错误类型转换为字符串
     * @param type 错误类型
     * @return 错误类型字符串
     */
    static QString errorTypeToString(ProcessErrorType type);

signals:
    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const ProcessError& error);

private:
    /**
     * @brief 构造函数
     */
    explicit ErrorHandler(QObject* parent = nullptr);

    /**
     * @brief 初始化错误处理策略映射
     */
    void initErrorActions();

    QMap<ProcessErrorType, ErrorAction> m_errorActions;  ///< 错误处理策略映射
    mutable QMutex m_mutex;  ///< 互斥锁
};

#endif // ERRORHANDLER_H
