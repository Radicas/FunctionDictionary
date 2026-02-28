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
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>

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
     * @brief 分析代码，提取函数信息
     * @param code 要分析的代码字符串
     */
    void analyzeCode(const QString& code);

    /**
     * @brief 取消当前的分析请求
     */
    void cancelRequest();

signals:
    /**
     * @brief 分析完成信号
     * @param functionName 函数名称
     * @param functionDescription 函数描述
     */
    void analysisComplete(const QString& functionName, const QString& functionDescription);

    /**
     * @brief 分析失败信号
     * @param error 错误信息
     */
    void analysisFailed(const QString& error);

    /**
     * @brief 分析进度信号
     * @param message 进度消息
     */
    void analysisProgress(const QString& message);

private slots:
    /**
     * @brief 网络请求完成槽函数
     * @param reply 网络回复对象
     */
    void onReplyFinished(QNetworkReply* reply);

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

    mutable QMutex m_mutex;
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
};

#endif
