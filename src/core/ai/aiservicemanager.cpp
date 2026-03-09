/**
 * @file aiservicemanager.cpp
 * @brief AI服务管理器实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "core/ai/aiservicemanager.h"
#include <QElapsedTimer>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QUuid>
#include "common/logger/logger.h"

AIServiceManager::AIServiceManager(QObject* parent)
    : QObject(parent),
      m_networkManager(nullptr),
      m_rateLimit(60),
      m_lastRequestTime(0),
      m_processTimer(new QTimer(this)),
      m_timeoutTimer(new QTimer(this)),
      m_timeoutMs(120000)
{

    m_networkManager = new QNetworkAccessManager(this);

    connect(m_processTimer, &QTimer::timeout, this, &AIServiceManager::onProcessQueue);
    connect(m_timeoutTimer, &QTimer::timeout, this, &AIServiceManager::onRequestTimeout);

    Logger::instance().info("AI服务管理器初始化完成");
}

AIServiceManager::~AIServiceManager()
{
    cancelAllRequests();

    if (m_processTimer->isActive())
    {
        m_processTimer->stop();
    }
    if (m_timeoutTimer->isActive())
    {
        m_timeoutTimer->stop();
    }
}

AIServiceManager& AIServiceManager::instance()
{
    static AIServiceManager instance;
    return instance;
}

void AIServiceManager::analyzeCode(const QString& code)
{
    QMutexLocker locker(&m_mutex);

    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    if (!AIConfigManager::instance().isConfigValid(config))
    {
        emit analysisFailed("AI配置不完整，请先配置AI服务！");
        Logger::instance().error("AI配置不完整，无法分析代码");
        return;
    }

    emit analysisProgress("正在向AI发送请求...");

    ExtractedFunction func;
    func.name = "code_analysis";
    func.body = code;
    func.language = "generic";
    func.signature = "code_analysis";

    AIAnalysisRequest request;
    request.requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    request.function = func;
    request.createTime = QDateTime::currentDateTime();

    m_pendingRequests[request.requestId] = request;
    m_requestQueue.enqueue(request);

    if (m_requestQueue.size() == 1)
    {
        processQueue();
    }
}

void AIServiceManager::analyzeFunction(const ExtractedFunction& func, const QString& requestId)
{
    QMutexLocker locker(&m_mutex);

    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    if (!AIConfigManager::instance().isConfigValid(config))
    {
        AIAnalysisResponse response;
        response.requestId = requestId;
        response.success = false;
        response.errorMessage = "AI配置不完整，请先配置AI服务！";
        emit functionAnalysisComplete(response);
        Logger::instance().error("AI配置不完整，无法分析代码");
        return;
    }

    AIAnalysisRequest request;
    request.requestId = requestId;
    request.function = func;
    request.createTime = QDateTime::currentDateTime();

    m_pendingRequests[requestId] = request;
    m_requestQueue.enqueue(request);

    if (m_requestQueue.size() == 1)
    {
        processQueue();
    }
}

void AIServiceManager::analyzeFunctions(const QVector<ExtractedFunction>& functions)
{
    QMutexLocker locker(&m_mutex);

    for (const auto& func : functions)
    {
        AIAnalysisRequest request;
        request.requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        request.function = func;
        request.createTime = QDateTime::currentDateTime();

        m_pendingRequests[request.requestId] = request;
        m_requestQueue.enqueue(request);
    }

    m_startTime = QDateTime::currentDateTime();

    if (m_requestQueue.size() == functions.size())
    {
        processQueue();
    }
}

void AIServiceManager::cancelRequest()
{
    QMutexLocker locker(&m_mutex);
    cancelAllRequests();
}

void AIServiceManager::cancelAllRequests()
{
    QMutexLocker locker(&m_mutex);

    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it)
    {
        if (it.value())
        {
            it.value()->abort();
            it.value()->deleteLater();
        }
    }
    m_activeRequests.clear();

    m_requestQueue.clear();
    m_pendingRequests.clear();

    if (m_processTimer->isActive())
    {
        m_processTimer->stop();
    }

    Logger::instance().info("已取消所有AI分析请求");
}

RequestQueueStatus AIServiceManager::getQueueStatus() const
{
    QMutexLocker locker(&m_mutex);

    RequestQueueStatus status;
    status.totalRequests = m_requestQueue.size() + m_activeRequests.size();
    status.pendingRequests = m_requestQueue.size();
    status.activeRequests = m_activeRequests.size();
    status.completedRequests = 0;
    status.failedRequests = 0;
    status.startTime = m_startTime;
    status.elapsedTime = m_startTime.msecsTo(QDateTime::currentDateTime());

    return status;
}

void AIServiceManager::setRateLimit(int requestsPerMinute)
{
    QMutexLocker locker(&m_mutex);
    m_rateLimit = requestsPerMinute;
    Logger::instance().info(QString("速率限制已设置为每分钟 %1 次").arg(requestsPerMinute));
}

void AIServiceManager::setTimeout(int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    m_timeoutMs = timeoutMs;
    Logger::instance().info(QString("请求超时时间已设置为 %1 毫秒").arg(timeoutMs));
}

void AIServiceManager::onRequestTimeout()
{
    QMutexLocker locker(&m_mutex);

    Logger::instance().error(QString("请求超时 (%1 ms)").arg(m_timeoutMs));

    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it)
    {
        QString requestId = it.key();
        QNetworkReply* reply = it.value();

        if (reply)
        {
            reply->abort();
            reply->deleteLater();
        }

        AIAnalysisResponse response;
        response.requestId = requestId;
        response.success = false;
        response.errorMessage = QString("请求超时 (%1 秒)").arg(m_timeoutMs / 1000);

        if (m_pendingRequests.contains(requestId))
        {
            response.retryCount = m_pendingRequests.value(requestId).retryCount;
            response.functionName = m_pendingRequests.value(requestId).function.name;
            m_pendingRequests.remove(requestId);
        }

        emit functionAnalysisComplete(response);
    }

    m_activeRequests.clear();
    m_requestStartTimes.clear();

    emit queueStatusChanged(getQueueStatus());

    processQueue();
}

void AIServiceManager::onReplyFinished(QNetworkReply* reply)
{
    QMutexLocker locker(&m_mutex);

    m_timeoutTimer->stop();

    QString requestId;
    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it)
    {
        if (it.value() == reply)
        {
            requestId = it.key();
            break;
        }
    }

    m_activeRequests.remove(requestId);
    m_requestStartTimes.remove(requestId);
    reply->deleteLater();

    AIAnalysisResponse response;
    response.requestId = requestId;

    if (m_requestStartTimes.contains(requestId))
    {
        response.responseTime = QDateTime::currentMSecsSinceEpoch() - m_requestStartTimes.value(requestId);
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        response.success = false;
        response.errorMessage = "网络请求失败: " + reply->errorString();
        response.retryCount = m_pendingRequests.value(requestId).retryCount;
        Logger::instance().error(response.errorMessage);
    }
    else
    {
        QByteArray responseData = reply->readAll();

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &error);

        if (error.error != QJsonParseError::NoError)
        {
            response.success = false;
            response.errorMessage = "解析响应JSON失败: " + error.errorString();
            Logger::instance().error(response.errorMessage);
        }
        else
        {
            QString aiResponse = parseResponseJson(jsonDoc);

            if (aiResponse.isEmpty())
            {
                response.success = false;
                response.errorMessage = "AI响应格式错误";
                Logger::instance().error(response.errorMessage);
            }
            else
            {
                QString functionName;
                QString functionDescription;

                if (extractFunctionInfo(aiResponse, functionName, functionDescription))
                {
                    response.success = true;
                    response.functionName = functionName;
                    response.functionDescription = functionDescription;

                    QString jsonStr = aiResponse.trimmed();
                    int jsonStart = jsonStr.indexOf('{');
                    int jsonEnd = jsonStr.lastIndexOf('}');

                    if (jsonStart != -1 && jsonEnd != -1)
                    {
                        jsonStr = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);
                        QJsonDocument funcJson = QJsonDocument::fromJson(jsonStr.toUtf8());
                        if (funcJson.isObject())
                        {
                            QJsonObject obj = funcJson.object();
                            if (obj.contains("signature"))
                                response.signature = obj["signature"].toString();
                            if (obj.contains("return_type"))
                                response.returnType = obj["return_type"].toString();
                            if (obj.contains("flowchart"))
                                response.flowchart = obj["flowchart"].toString();
                            if (obj.contains("sequence_diagram"))
                                response.sequenceDiagram = obj["sequence_diagram"].toString();
                            if (obj.contains("structure_diagram"))
                                response.structureDiagram = obj["structure_diagram"].toString();

                            if (obj.contains("parameters") && obj["parameters"].isArray())
                            {
                                QJsonDocument paramsDoc(obj["parameters"].toArray());
                                response.parameters = paramsDoc.toJson(QJsonDocument::Compact);
                            }
                        }
                    }

                    Logger::instance().info("AI分析完成，函数名称: " + functionName);
                }
                else
                {
                    response.success = false;
                    response.errorMessage = "无法从AI响应中提取函数信息";
                    Logger::instance().error(response.errorMessage);
                }
            }
        }
    }

    if (m_pendingRequests.contains(requestId))
    {
        response.retryCount = m_pendingRequests.value(requestId).retryCount;
        response.functionName = m_pendingRequests.value(requestId).function.name;
        m_pendingRequests.remove(requestId);
    }

    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    response.aiModel = config.defaultModel;
    response.analyzeTime = QDateTime::currentDateTime();

    emit functionAnalysisComplete(response);

    emit analysisComplete(response.functionName, response.functionDescription);

    processQueue();
}

void AIServiceManager::onProcessQueue()
{
    processQueue();
}

QString AIServiceManager::buildRequestUrl() const
{
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    QString url = config.baseUrl;
    if (!url.endsWith("/"))
    {
        url += "/";
    }
    url += "chat/completions";
    return url;
}

QJsonObject AIServiceManager::buildRequestJson(const QString& prompt) const
{
    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject jsonObj;
    jsonObj["model"] = config.defaultModel;
    jsonObj["messages"] = messagesArray;
    jsonObj["temperature"] = 0.7;

    return jsonObj;
}

QString AIServiceManager::parseResponseJson(const QJsonDocument& jsonDoc) const
{
    if (!jsonDoc.isObject())
    {
        return QString();
    }

    QJsonObject rootObj = jsonDoc.object();
    if (!rootObj.contains("choices"))
    {
        return QString();
    }

    QJsonArray choicesArray = rootObj["choices"].toArray();
    if (choicesArray.isEmpty())
    {
        return QString();
    }

    QJsonObject choiceObj = choicesArray[0].toObject();
    if (!choiceObj.contains("message"))
    {
        return QString();
    }

    QJsonObject messageObj = choiceObj["message"].toObject();
    if (!messageObj.contains("content"))
    {
        return QString();
    }

    return messageObj["content"].toString();
}

bool AIServiceManager::extractFunctionInfo(const QString& aiResponse, QString& functionName,
                                           QString& functionDescription) const
{
    QString jsonStr = aiResponse.trimmed();

    int jsonStart = jsonStr.indexOf('{');
    int jsonEnd = jsonStr.lastIndexOf('}');

    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart)
    {
        Logger::instance().error("AI响应中未找到有效的JSON对象");
        return false;
    }

    jsonStr = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError)
    {
        Logger::instance().error("解析AI响应JSON失败: " + error.errorString());
        Logger::instance().error("JSON位置 " + QString::number(error.offset) +
                                 " 附近内容: " + jsonStr.mid(qMax(0, error.offset - 50), 100));
        return false;
    }

    if (!jsonDoc.isObject())
    {
        Logger::instance().error("AI响应不是有效的JSON对象");
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("function_name"))
    {
        functionName = jsonObj["function_name"].toString().trimmed();
    }

    if (functionName.isEmpty())
    {
        Logger::instance().warning("AI响应中缺少function_name字段或为空");
        functionName = "unknown_function";
    }

    if (jsonObj.contains("function_description"))
    {
        functionDescription = jsonObj["function_description"].toString().trimmed();
    }

    if (functionDescription.isEmpty())
    {
        Logger::instance().warning("AI响应中缺少function_description字段或为空");
        functionDescription = "暂无描述";
    }

    return true;
}

QString AIServiceManager::buildFunctionPrompt(const ExtractedFunction& func) const
{
    QString prompt = QString(
                         "请分析以下%1函数，提取函数详细信息。\n\n"
                         "函数名称: %2\n"
                         "函数签名: %3\n\n"
                         "函数代码:\n"
                         "```%4\n"
                         "%5\n"
                         "```\n\n"
                         "请以JSON格式返回，格式如下：\n"
                         "{\n"
                         "  \"function_name\": \"函数名\",\n"
                         "  \"function_description\": \"函数的详细功能说明，包括：\\n"
                         "    1. 功能概述\\n"
                         "    2. 参数说明\\n"
                         "    3. 返回值说明\\n"
                         "    4. 使用示例\\n"
                         "    5. 注意事项\",\n"
                         "  \"signature\": \"函数签名\",\n"
                         "  \"return_type\": \"返回值类型\",\n"
                         "  \"parameters\": [\n"
                         "    {\"name\": \"参数名\", \"type\": \"参数类型\", \"description\": \"参数说明\"}\n"
                         "  ],\n"
                         "  \"flowchart\": \"使用mermaid flowchart语法绘制的函数运行流程图\",\n"
                         "  \"sequence_diagram\": \"使用mermaid sequenceDiagram语法绘制的函数调用时序图\",\n"
                         "  \"structure_diagram\": \"使用mermaid graph语法绘制的函数结构关系图\"\n"
                         "}\n\n"
                         "要求：\n"
                         "1. 描述应基于代码实际逻辑，准确反映函数功能\n"
                         "2. 所有mermaid图表语法必须正确\n"
                         "3. 返回的JSON必须是有效格式\n"
                         "4. 所有描述使用中文")
                         .arg(func.language)
                         .arg(func.name)
                         .arg(func.signature)
                         .arg(func.language)
                         .arg(func.body);

    return prompt;
}

void AIServiceManager::processQueue()
{
    QMutexLocker locker(&m_mutex);

    if (m_requestQueue.isEmpty())
    {
        if (m_processTimer->isActive())
        {
            m_processTimer->stop();
        }
        emit queueStatusChanged(getQueueStatus());
        return;
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 minInterval = 60000 / m_rateLimit;

    if (currentTime - m_lastRequestTime < minInterval)
    {
        qint64 waitTime = minInterval - (currentTime - m_lastRequestTime);
        if (!m_processTimer->isActive())
        {
            m_processTimer->start(waitTime);
        }
        return;
    }

    sendRequest(m_requestQueue.dequeue());
}

void AIServiceManager::sendRequest(const AIAnalysisRequest& request)
{
    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    QString prompt = buildFunctionPrompt(request.function);

    QUrl url(buildRequestUrl());
    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setRawHeader("Authorization", QString("Bearer %1").arg(config.apiKey).toUtf8());

    QJsonObject jsonObj = buildRequestJson(prompt);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkReply* reply = m_networkManager->post(networkRequest, jsonData);

    m_activeRequests[request.requestId] = reply;
    m_requestStartTimes[request.requestId] = QDateTime::currentMSecsSinceEpoch();
    m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReplyFinished(reply); });

    m_timeoutTimer->start(m_timeoutMs);

    Logger::instance().info(QString("已发送AI分析请求，函数: %1").arg(request.function.name));

    emit queueStatusChanged(getQueueStatus());
}
