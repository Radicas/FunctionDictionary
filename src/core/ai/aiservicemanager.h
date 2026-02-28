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
    static AIServiceManager& instance();
    AIServiceManager(const AIServiceManager&) = delete;
    AIServiceManager& operator=(const AIServiceManager&) = delete;

    void analyzeCode(const QString& code);
    void cancelRequest();

signals:
    void analysisComplete(const QString& functionName, const QString& functionDescription);
    void analysisFailed(const QString& error);
    void analysisProgress(const QString& message);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    explicit AIServiceManager(QObject* parent = nullptr);
    ~AIServiceManager();

    QString buildRequestUrl() const;
    QJsonObject buildRequestJson(const QString& prompt) const;
    QString parseResponseJson(const QJsonDocument& jsonDoc) const;
    bool extractFunctionInfo(const QString& aiResponse, QString& functionName, QString& functionDescription) const;

    mutable QMutex m_mutex;
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
};

#endif
