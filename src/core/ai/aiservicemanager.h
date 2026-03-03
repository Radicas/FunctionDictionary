/**
 * @file aiservicemanager.h
 * @brief AI服务管理器，负责与AI API进行通信
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef AISERVICEMANAGER_H
#define AISERVICEMANAGER_H

#include "aiconfigmanager.h"
#include "core/models/extractedfunction.h"
#include "core/models/batchconfig.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QTimer>

class AIServiceManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取AIServiceManager的单例实例
     * @return AIServiceManager的引用
     */
    static AIServiceManager& instance();
    AIServiceManager(const AIServiceManager&) = delete;
    AIServiceManager& operator=(const AIServiceManager&) = delete;

    /**
     * @brief 分析代码，提取函数信息（兼容旧接口）
     * @param code 要分析的代码字符串
     */
    void analyzeCode(const QString& code);

    /**
     * @brief 分析单个函数（异步）
     * @param func 要分析的函数
     * @param requestId 请求ID
     */
    void analyzeFunction(const ExtractedFunction& func, const QString& requestId);

    /**
     * @brief 批量分析函数（队列模式）
     * @param functions 函数列表
     */
    void analyzeFunctions(const QVector<ExtractedFunction>& functions);

    /**
     * @brief 取消当前的分析请求
     */
    void cancelRequest();

    /**
     * @brief 取消所有请求
     */
    void cancelAllRequests();

    /**
     * @brief 获取请求队列状态
     * @return 队列状态
     */
    RequestQueueStatus getQueueStatus() const;

    /**
     * @brief 设置速率限制
     * @param requestsPerMinute 每分钟请求数
     */
    void setRateLimit(int requestsPerMinute);

signals:
    /**
     * @brief 分析完成信号（兼容旧接口）
     * @param functionName 函数名称
     * @param functionDescription 函数描述
     */
    void analysisComplete(const QString& functionName, const QString& functionDescription);

    /**
     * @brief 分析失败信号（兼容旧接口）
     * @param error 错误信息
     */
    void analysisFailed(const QString& error);

    /**
     * @brief 分析进度信号（兼容旧接口）
     * @param message 进度消息
     */
    void analysisProgress(const QString& message);

    /**
     * @brief 单个函数分析完成信号
     * @param response 分析响应
     */
    void functionAnalysisComplete(const AIAnalysisResponse& response);

    /**
     * @brief 队列状态变化信号
     * @param status 队列状态
     */
    void queueStatusChanged(const RequestQueueStatus& status);

private slots:
    /**
     * @brief 网络请求完成槽函数
     * @param reply 网络回复对象
     */
    void onReplyFinished(QNetworkReply* reply);

    /**
     * @brief 处理队列槽函数
     */
    void onProcessQueue();

private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AIServiceManager(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AIServiceManager();

    /**
     * @brief 构建请求URL
     * @return 请求URL字符串
     */
    QString buildRequestUrl() const;

    /**
     * @brief 构建请求JSON对象
     * @param prompt 请求提示词
     * @return 请求JSON对象
     */
    QJsonObject buildRequestJson(const QString& prompt) const;

    /**
     * @brief 解析响应JSON
     * @param jsonDoc JSON文档
     * @return 解析后的响应字符串
     */
    QString parseResponseJson(const QJsonDocument& jsonDoc) const;

    /**
     * @brief 提取函数信息
     * @param aiResponse AI响应字符串
     * @param functionName 输出参数，函数名称
     * @param functionDescription 输出参数，函数描述
     * @return 是否成功提取函数信息
     */
    bool extractFunctionInfo(const QString& aiResponse, QString& functionName, QString& functionDescription) const;

    /**
     * @brief 构建函数分析提示词
     * @param func 函数信息
     * @return 提示词
     */
    QString buildFunctionPrompt(const ExtractedFunction& func) const;

    /**
     * @brief 处理队列
     */
    void processQueue();

    /**
     * @brief 发送请求
     * @param request 请求
     */
    void sendRequest(const AIAnalysisRequest& request);

    mutable QMutex m_mutex;
    QNetworkAccessManager* m_networkManager;

    QQueue<AIAnalysisRequest> m_requestQueue;
    QMap<QString, QNetworkReply*> m_activeRequests;
    QMap<QString, AIAnalysisRequest> m_pendingRequests;

    int m_rateLimit;
    qint64 m_lastRequestTime;
    QTimer* m_processTimer;

    QDateTime m_startTime;
};

#endif
