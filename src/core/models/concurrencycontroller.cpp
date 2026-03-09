/**
 * @file concurrencycontroller.cpp
 * @brief 并发控制器实现
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#include "core/models/concurrencycontroller.h"
#include "common/logger/logger.h"

ConcurrencyController::ConcurrencyController(int maxConcurrent, QObject* parent)
    : QObject(parent), m_semaphore(maxConcurrent), m_maxConcurrent(maxConcurrent)
{
    Logger::instance().info(QString("并发控制器初始化: 最大并发数 %1").arg(maxConcurrent));
}

bool ConcurrencyController::tryAcquire()
{
    return m_semaphore.tryAcquire();
}

void ConcurrencyController::release()
{
    m_semaphore.release();
}

bool ConcurrencyController::waitForAcquire(int timeout)
{
    if (timeout < 0)
    {
        m_semaphore.acquire();
        return true;
    }
    return m_semaphore.tryAcquire(1, timeout);
}

void ConcurrencyController::setMaxConcurrent(int max)
{
    QMutexLocker locker(&m_mutex);

    int current = available();
    int diff = max - m_maxConcurrent;

    m_maxConcurrent = qMax(max, 1);

    if (diff > 0)
    {
        m_semaphore.release(diff);
    }

    Logger::instance().info(QString("并发控制器更新: 最大并发数 %1").arg(m_maxConcurrent));
}

int ConcurrencyController::available() const
{
    return m_semaphore.available();
}
