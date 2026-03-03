/**
 * @file concurrencycontroller.h
 * @brief 并发控制器
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef CONCURRENCYCONTROLLER_H
#define CONCURRENCYCONTROLLER_H

#include <QObject>
#include <QSemaphore>
#include <QMutex>

/**
 * @brief 并发控制器类
 * 
 * 该类用于控制并发请求数量，防止同时发送过多请求。
 */
class ConcurrencyController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param maxConcurrent 最大并发数
     * @param parent 父对象
     */
    explicit ConcurrencyController(int maxConcurrent = 1, QObject* parent = nullptr);

    /**
     * @brief 尝试获取处理许可
     * @return 是否获得许可
     */
    bool tryAcquire();

    /**
     * @brief 释放处理许可
     */
    void release();

    /**
     * @brief 等待获取许可
     * @param timeout 超时时间（毫秒），-1表示无限等待
     * @return 是否成功获取
     */
    bool waitForAcquire(int timeout = -1);

    /**
     * @brief 设置最大并发数
     * @param max 最大并发数
     */
    void setMaxConcurrent(int max);

    /**
     * @brief 获取当前可用许可数
     * @return 可用许可数
     */
    int available() const;

private:
    QSemaphore m_semaphore;  ///< 信号量
    mutable QMutex m_mutex;   ///< 互斥锁
    int m_maxConcurrent;      ///< 最大并发数
};

#endif // CONCURRENCYCONTROLLER_H
