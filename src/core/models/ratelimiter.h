/**
 * @file ratelimiter.h
 * @brief 速率限制器（令牌桶算法）
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

/**
 * @brief 速率限制器类（令牌桶算法）
 * 
 * 该类实现了令牌桶算法，用于控制请求发送速率，
 * 防止超过API提供商的速率限制。
 */
class RateLimiter : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param requestsPerMinute 每分钟允许的请求数
     * @param parent 父对象
     */
    explicit RateLimiter(int requestsPerMinute = 60, QObject* parent = nullptr);

    /**
     * @brief 等待直到可以发送请求
     */
    void waitForToken();

    /**
     * @brief 检查是否可以立即发送请求
     * @return 是否可以立即发送
     */
    bool canSendImmediately() const;

    /**
     * @brief 设置速率限制
     * @param requestsPerMinute 每分钟允许的请求数
     */
    void setRateLimit(int requestsPerMinute);

    /**
     * @brief 获取当前令牌数
     * @return 当前令牌数
     */
    int getCurrentTokens() const;

private:
    /**
     * @brief 补充令牌
     */
    void refillTokens();

    mutable QMutex m_mutex;           ///< 互斥锁
    QWaitCondition m_condition;       ///< 等待条件
    int m_tokens;                     ///< 当前令牌数
    int m_maxTokens;                  ///< 最大令牌数
    qint64 m_lastRefillTime;          ///< 上次补充令牌的时间
    int m_refillRate;                 ///< 补充速率（每秒令牌数）
};

#endif // RATELIMITER_H
