/**
 * @file batchprocessmanager.h
 * @brief 批量处理管理器，负责协调函数的批量AI分析
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef BATCHPROCESSMANAGER_H
#define BATCHPROCESSMANAGER_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include "core/models/batchconfig.h"
#include "core/models/extractedfunction.h"

/**
 * @brief 批量处理管理器类
 * 
 * 该类负责协调多个函数的批量AI分析，包括任务队列管理、
 * 进度追踪、错误处理、断点续传等功能。
 */
class BatchProcessManager : public QObject
{
    Q_OBJECT

   public:
    /**
     * @brief 获取BatchProcessManager的单例实例
     * @return BatchProcessManager的引用
     */
    static BatchProcessManager& instance();

    BatchProcessManager(const BatchProcessManager&) = delete;
    BatchProcessManager& operator=(const BatchProcessManager&) = delete;

    /**
     * @brief 开始批量处理
     * @param functions 待处理的函数列表
     */
    void startBatchProcessing(const QVector<ExtractedFunction>& functions);

    /**
     * @brief 暂停处理
     */
    void pauseProcessing();

    /**
     * @brief 恢复处理
     */
    void resumeProcessing();

    /**
     * @brief 取消处理
     */
    void cancelProcessing();

    /**
     * @brief 获取当前处理状态
     * @return 当前处理状态
     */
    BatchProcessState getState() const;

    /**
     * @brief 设置并发配置
     * @param config 批量处理配置
     */
    void setConfig(const BatchProcessConfig& config);

    /**
     * @brief 获取当前配置
     * @return 当前配置
     */
    BatchProcessConfig getConfig() const;

    /**
     * @brief 获取成功数量
     * @return 成功处理的函数数量
     */
    int getSuccessCount() const;

    /**
     * @brief 获取失败数量
     * @return 失败处理的函数数量
     */
    int getFailedCount() const;

    /**
     * @brief 获取跳过数量
     * @return 跳过的函数数量
     */
    int getSkippedCount() const;

   signals:
    /**
     * @brief 整体进度信号
     * @param current 当前处理的函数索引
     * @param total 总函数数量
     * @param functionName 当前处理的函数名
     */
    void batchProgress(int current, int total, const QString& functionName);

    /**
     * @brief 单个函数处理完成信号
     * @param func 函数信息
     * @param success 是否成功
     * @param message 结果消息
     */
    void functionProcessed(const ExtractedFunction& func, bool success, const QString& message);

    /**
     * @brief 批量处理完成信号
     * @param successCount 成功数量
     * @param failedCount 失败数量
     * @param skippedCount 跳过数量
     */
    void batchCompleted(int successCount, int failedCount, int skippedCount);

    /**
     * @brief 状态变化信号
     * @param newState 新状态
     */
    void stateChanged(BatchProcessState newState);

   private slots:
    /**
     * @brief 处理下一个函数（内部槽函数）
     */
    void onProcessNext();

    /**
     * @brief AI分析完成槽函数
     * @param response 分析响应
     */
    void onAIAnalysisComplete(const AIAnalysisResponse& response);

   private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BatchProcessManager(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BatchProcessManager();

    /**
     * @brief 设置新状态
     * @param newState 新状态
     */
    void setState(BatchProcessState newState);

    /**
     * @brief 处理下一个函数
     */
    void processNext();

    /**
     * @brief 保存处理状态（断点续传）
     */
    void saveProcessState();

    /**
     * @brief 加载处理状态
     */
    void loadProcessState();

    /**
     * @brief 清除处理状态
     */
    void clearProcessState();

    /**
     * @brief 完成批量处理
     */
    void completeBatch();

    /**
     * @brief 检查函数是否已存在
     * @param func 函数信息
     * @return 是否已存在
     */
    bool functionExists(const ExtractedFunction& func) const;

    /**
     * @brief 生成唯一请求ID
     * @return 请求ID
     */
    QString generateRequestId() const;

    // 处理队列
    QQueue<ExtractedFunction> m_processQueue;

    // 已完成列表（用于断点续传）
    QSet<QString> m_processedFunctions;

    // 失败列表（用于重试）
    QMap<QString, int> m_failedFunctions;

    // 活跃请求
    QMap<QString, ExtractedFunction> m_activeRequests;

    // 状态管理
    BatchProcessState m_state;
    BatchProcessConfig m_config;

    // 统计信息
    int m_successCount;
    int m_failedCount;
    int m_skippedCount;
    int m_totalCount;
    int m_currentIndex;

    // 定时器
    QTimer* m_processTimer;

    // 互斥锁
    mutable QMutex m_mutex;
};

#endif  // BATCHPROCESSMANAGER_H
