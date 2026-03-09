/**
 * @file ratelimiter.cpp
 * @brief 速率限制器实现
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#include "core/models/ratelimiter.h"
#include "common/logger/logger.h"

RateLimiter::RateLimiter(int requestsPerMinute, QObject* parent)
    : QObject(parent), m_tokens(0), m_maxTokens(0), m_lastRefillTime(0), m_refillRate(0)
{
    setRateLimit(requestsPerMinute);
}

void RateLimiter::setRateLimit(int requestsPerMinute)
{
    QMutexLocker locker(&m_mutex);

    m_maxTokens = qMax(requestsPerMinute, 1);
    m_tokens = m_maxTokens;
    m_refillRate = m_maxTokens / 60;
    if (m_refillRate < 1)
    {
        m_refillRate = 1;
    }
    m_lastRefillTime = QDateTime::currentMSecsSinceEpoch();

    Logger::instance().info(QString("速率限制器设置: 每分钟 %1 次请求").arg(requestsPerMinute));
}

void RateLimiter::refillTokens()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_lastRefillTime;

    if (elapsed > 0)
    {
        int tokensToAdd = static_cast<int>((elapsed * m_refillRate) / 1000);
        m_tokens = qMin(m_tokens + tokensToAdd, m_maxTokens);
        m_lastRefillTime = now;
    }
}

bool RateLimiter::canSendImmediately() const
{
    QMutexLocker locker(&m_mutex);
    return m_tokens > 0;
}

int RateLimiter::getCurrentTokens() const
{
    QMutexLocker locker(&m_mutex);
    const_cast<RateLimiter*>(this)->refillTokens();
    return m_tokens;
}

void RateLimiter::waitForToken()
{
    QMutexLocker locker(&m_mutex);

    while (true)
    {
        refillTokens();

        if (m_tokens > 0)
        {
            m_tokens--;
            return;
        }

        int waitTime = qMax(1000 / m_refillRate, 100);
        m_condition.wait(&m_mutex, waitTime);
    }
}
