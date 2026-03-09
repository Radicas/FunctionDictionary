/**
 * @file batchprocessmanager.cpp
 * @brief 批量处理管理器实现
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#include "core/batch/batchprocessmanager.h"
#include <QFileInfo>
#include <QSettings>
#include <QUuid>
#include "common/logger/logger.h"
#include "core/ai/aiservicemanager.h"
#include "core/database/databasemanager.h"

BatchProcessManager& BatchProcessManager::instance()
{
    static BatchProcessManager instance;
    return instance;
}

BatchProcessManager::BatchProcessManager(QObject* parent)
    : QObject(parent),
      m_state(BatchProcessState::Idle),
      m_successCount(0),
      m_failedCount(0),
      m_skippedCount(0),
      m_totalCount(0),
      m_currentIndex(0),
      m_processTimer(new QTimer(this))
{

    connect(m_processTimer, &QTimer::timeout, this, &BatchProcessManager::onProcessNext);

    Logger::instance().info("BatchProcessManager 初始化完成");
}

BatchProcessManager::~BatchProcessManager()
{
    if (m_processTimer->isActive())
    {
        m_processTimer->stop();
    }
}

void BatchProcessManager::setState(BatchProcessState newState)
{
    if (m_state != newState)
    {
        m_state = newState;
        emit stateChanged(newState);

        QString stateStr;
        switch (newState)
        {
            case BatchProcessState::Idle:
                stateStr = "空闲";
                break;
            case BatchProcessState::Running:
                stateStr = "运行中";
                break;
            case BatchProcessState::Paused:
                stateStr = "已暂停";
                break;
            case BatchProcessState::Completed:
                stateStr = "已完成";
                break;
            case BatchProcessState::Cancelled:
                stateStr = "已取消";
                break;
        }
        Logger::instance().info("批量处理状态变化: " + stateStr);
    }
}

void BatchProcessManager::startBatchProcessing(const QVector<ExtractedFunction>& functions)
{
    QMutexLocker locker(&m_mutex);

    if (m_state == BatchProcessState::Running)
    {
        Logger::instance().warning("批量处理已在运行中");
        return;
    }

    m_processQueue.clear();
    m_processedFunctions.clear();
    m_failedFunctions.clear();
    m_activeRequests.clear();

    m_successCount = 0;
    m_failedCount = 0;
    m_skippedCount = 0;
    m_totalCount = functions.size();
    m_currentIndex = 0;

    for (const auto& func : functions)
    {
        m_processQueue.enqueue(func);
    }

    if (m_config.enableCheckpoint)
    {
        loadProcessState();
    }

    Logger::instance().info(QString("开始批量处理，共 %1 个函数").arg(m_totalCount));

    setState(BatchProcessState::Running);

    if (m_processQueue.isEmpty())
    {
        completeBatch();
    }
    else
    {
        processNext();
    }
}

void BatchProcessManager::pauseProcessing()
{
    QMutexLocker locker(&m_mutex);

    if (m_state != BatchProcessState::Running)
    {
        return;
    }

    if (m_processTimer->isActive())
    {
        m_processTimer->stop();
    }

    setState(BatchProcessState::Paused);

    if (m_config.enableCheckpoint)
    {
        saveProcessState();
    }

    Logger::instance().info("批量处理已暂停");
}

void BatchProcessManager::resumeProcessing()
{
    QMutexLocker locker(&m_mutex);

    if (m_state != BatchProcessState::Paused)
    {
        return;
    }

    Logger::instance().info("恢复批量处理");

    setState(BatchProcessState::Running);
    processNext();
}

void BatchProcessManager::cancelProcessing()
{
    QMutexLocker locker(&m_mutex);

    if (m_state != BatchProcessState::Running && m_state != BatchProcessState::Paused)
    {
        return;
    }

    if (m_processTimer->isActive())
    {
        m_processTimer->stop();
    }

    setState(BatchProcessState::Cancelled);

    clearProcessState();

    Logger::instance().info("批量处理已取消");

    emit batchCompleted(m_successCount, m_failedCount, m_skippedCount);
}

BatchProcessState BatchProcessManager::getState() const
{
    QMutexLocker locker(&m_mutex);
    return m_state;
}

void BatchProcessManager::setConfig(const BatchProcessConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    Logger::instance().info("批量处理配置已更新");
}

BatchProcessConfig BatchProcessManager::getConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

int BatchProcessManager::getSuccessCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_successCount;
}

int BatchProcessManager::getFailedCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_failedCount;
}

int BatchProcessManager::getSkippedCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_skippedCount;
}

void BatchProcessManager::onProcessNext()
{
    processNext();
}

void BatchProcessManager::onAIAnalysisComplete(const AIAnalysisResponse& response)
{
    QMutexLocker locker(&m_mutex);

    if (!m_activeRequests.contains(response.requestId))
    {
        return;
    }

    ExtractedFunction func = m_activeRequests.take(response.requestId);

    if (response.success)
    {
        m_successCount++;
        m_processedFunctions.insert(func.name);
        Logger::instance().info(QString("函数分析成功: %1").arg(func.name));

        emit functionProcessed(func, true, "分析成功");
    }
    else
    {
        int retryCount = m_failedFunctions.value(func.name, 0) + 1;

        if (retryCount <= m_config.maxRetryCount)
        {
            m_failedFunctions[func.name] = retryCount;
            m_processQueue.enqueue(func);
            Logger::instance().warning(
                QString("函数分析失败，将重试 (%1/%2): %3").arg(retryCount).arg(m_config.maxRetryCount).arg(func.name));
        }
        else
        {
            m_failedCount++;
            m_failedFunctions.remove(func.name);
            Logger::instance().error(QString("函数分析失败，已达最大重试次数: %1").arg(func.name));

            emit functionProcessed(func, false, response.errorMessage);
        }
    }

    m_currentIndex++;
    emit batchProgress(m_currentIndex, m_totalCount, func.name);

    if (m_state == BatchProcessState::Running)
    {
        m_processTimer->start(m_config.requestInterval);
    }
}

void BatchProcessManager::processNext()
{
    QMutexLocker locker(&m_mutex);

    if (m_state != BatchProcessState::Running)
    {
        return;
    }

    if (m_processQueue.isEmpty())
    {
        if (m_activeRequests.isEmpty())
        {
            completeBatch();
        }
        return;
    }

    ExtractedFunction func = m_processQueue.dequeue();

    if (m_config.skipExisting && functionExists(func))
    {
        m_skippedCount++;
        m_processedFunctions.insert(func.name);
        m_currentIndex++;

        Logger::instance().info(QString("跳过已存在的函数: %1").arg(func.name));
        emit functionProcessed(func, true, "已存在，跳过");
        emit batchProgress(m_currentIndex, m_totalCount, func.name);

        m_processTimer->start(0);
        return;
    }

    QString requestId = generateRequestId();
    m_activeRequests[requestId] = func;

    Logger::instance().info(
        QString("开始分析函数 (%1/%2): %3").arg(m_currentIndex + 1).arg(m_totalCount).arg(func.name));

    emit batchProgress(m_currentIndex, m_totalCount, func.name);

    AIServiceManager& aiService = AIServiceManager::instance();
    aiService.analyzeFunction(func, requestId);
}

void BatchProcessManager::completeBatch()
{
    setState(BatchProcessState::Completed);

    Logger::instance().info(
        QString("批量处理完成: 成功 %1, 失败 %2, 跳过 %3").arg(m_successCount).arg(m_failedCount).arg(m_skippedCount));

    clearProcessState();

    emit batchCompleted(m_successCount, m_failedCount, m_skippedCount);
}

void BatchProcessManager::saveProcessState()
{
    QSettings settings("FunctionDictionary", "BatchProcess");

    settings.setValue("processedFunctions", QStringList(m_processedFunctions.begin(), m_processedFunctions.end()));

    QVariantMap failedMap;
    for (auto it = m_failedFunctions.begin(); it != m_failedFunctions.end(); ++it)
    {
        failedMap[it.key()] = it.value();
    }
    settings.setValue("failedFunctions", failedMap);

    settings.setValue("successCount", m_successCount);
    settings.setValue("failedCount", m_failedCount);
    settings.setValue("skippedCount", m_skippedCount);
    settings.setValue("totalCount", m_totalCount);
    settings.setValue("currentIndex", m_currentIndex);

    settings.sync();

    Logger::instance().info("处理状态已保存");
}

void BatchProcessManager::loadProcessState()
{
    QSettings settings("FunctionDictionary", "BatchProcess");

    if (!settings.contains("processedFunctions"))
    {
        return;
    }

    QStringList processed = settings.value("processedFunctions").toStringList();
    m_processedFunctions = QSet<QString>(processed.begin(), processed.end());

    QVariantMap failedMap = settings.value("failedFunctions").toMap();
    for (auto it = failedMap.begin(); it != failedMap.end(); ++it)
    {
        m_failedFunctions[it.key()] = it.value().toInt();
    }

    m_successCount = settings.value("successCount", 0).toInt();
    m_failedCount = settings.value("failedCount", 0).toInt();
    m_skippedCount = settings.value("skippedCount", 0).toInt();
    m_totalCount = settings.value("totalCount", 0).toInt();
    m_currentIndex = settings.value("currentIndex", 0).toInt();

    Logger::instance().info(
        QString("已加载处理状态: 已处理 %1, 待重试 %2").arg(m_processedFunctions.size()).arg(m_failedFunctions.size()));
}

void BatchProcessManager::clearProcessState()
{
    QSettings settings("FunctionDictionary", "BatchProcess");
    settings.clear();
    settings.sync();
}

bool BatchProcessManager::functionExists(const ExtractedFunction& func) const
{
    if (m_processedFunctions.contains(func.name))
    {
        return true;
    }

    return DatabaseManager::instance().functionExists(func.name);
}

QString BatchProcessManager::generateRequestId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
