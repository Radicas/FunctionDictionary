/**
 * @file AIServiceManager.cpp
 * @brief AI服务管理器实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "aiservicemanager.h"
#include "logger.h"
#include <QNetworkRequest>
#include <QJsonParseError>

AIServiceManager::AIServiceManager(QObject* parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr) {
    m_networkManager = new QNetworkAccessManager(this);
    Logger::instance().info("AI服务管理器初始化完成");
}

AIServiceManager::~AIServiceManager() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

AIServiceManager& AIServiceManager::instance() {
    static AIServiceManager instance;
    return instance;
}

void AIServiceManager::analyzeCode(const QString& code) {
    QMutexLocker locker(&m_mutex);

    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    if (!AIConfigManager::instance().isConfigValid(config)) {
        emit analysisFailed("AI配置不完整，请先配置AI服务！");
        Logger::instance().error("AI配置不完整，无法分析代码");
        return;
    }

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }

    emit analysisProgress("正在向AI发送请求...");

    QString prompt = "请分析以下代码，提取函数详细信息。\n"
                     "请以JSON格式返回，格式如下：\n"
                     "{\n"
                     "  \"function_name\": \"函数名\",\n"
                     "  \"function_description\": \"函数的基本功能说明\",\n"
                     "  \"parameters\": [\n"
                     "    {\"name\": \"参数名\", \"type\": \"参数类型\", \"description\": \"参数说明\"}\n"
                     "  ],\n"
                     "  \"return_type\": \"返回值类型\",\n"
                     "  \"return_description\": \"返回值说明\",\n"
                     "  \"flowchart\": \"使用mermaid flowchart语法绘制的函数运行流程图，清晰展示函数内部执行步骤和分支逻辑\",\n"
                     "  \"sequence_diagram\": \"使用mermaid sequenceDiagram语法绘制的函数调用时序图，明确函数与其他模块或组件的交互过程和调用顺序\",\n"
                     "  \"structure_diagram\": \"使用mermaid graph语法绘制的函数结构关系图，直观呈现该函数在整体系统架构中的位置以及与其他函数或类的依赖关系\"\n"
                     "}\n\n"
                     "要求：\n"
                     "1. 所有mermaid图表语法必须正确、逻辑清晰，符合mermaid规范\n"
                     "2. 图表内容应基于代码实际逻辑，能够直接用于文档生成或可视化展示\n"
                     "3. 流程图应包含所有关键分支和循环结构\n"
                     "4. 时序图应展示函数与外部组件的交互\n"
                     "5. 结构图应清晰展示依赖关系\n\n"
                     "代码：\n" + code;

    QUrl url(buildRequestUrl());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(config.apiKey).toUtf8());

    QJsonObject jsonObj = buildRequestJson(prompt);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonData = jsonDoc.toJson();

    m_currentReply = m_networkManager->post(request, jsonData);

    connect(m_currentReply, &QNetworkReply::finished, this, [this, reply = m_currentReply]() {
        onReplyFinished(reply);
    });

    Logger::instance().info("已发送AI分析请求，Base URL: " + config.baseUrl + ", Model: " + config.modelId);
}

void AIServiceManager::cancelRequest() {
    QMutexLocker locker(&m_mutex);
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        Logger::instance().info("用户取消了AI分析请求");
    }
}

QString AIServiceManager::buildRequestUrl() const {
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    QString url = config.baseUrl;
    if (!url.endsWith("/")) {
        url += "/";
    }
    url += "chat/completions";
    return url;
}

QJsonObject AIServiceManager::buildRequestJson(const QString& prompt) const {
    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject jsonObj;
    jsonObj["model"] = config.modelId;
    jsonObj["messages"] = messagesArray;
    jsonObj["temperature"] = 0.7;

    return jsonObj;
}

QString AIServiceManager::parseResponseJson(const QJsonDocument& jsonDoc) const {
    if (!jsonDoc.isObject()) {
        return QString();
    }

    QJsonObject rootObj = jsonDoc.object();
    if (!rootObj.contains("choices")) {
        return QString();
    }

    QJsonArray choicesArray = rootObj["choices"].toArray();
    if (choicesArray.isEmpty()) {
        return QString();
    }

    QJsonObject choiceObj = choicesArray[0].toObject();
    if (!choiceObj.contains("message")) {
        return QString();
    }

    QJsonObject messageObj = choiceObj["message"].toObject();
    if (!messageObj.contains("content")) {
        return QString();
    }

    return messageObj["content"].toString();
}

bool AIServiceManager::extractFunctionInfo(const QString& aiResponse, QString& functionName, QString& functionDescription) const {
    QString jsonStr = aiResponse.trimmed();

    int jsonStart = jsonStr.indexOf('{');
    int jsonEnd = jsonStr.lastIndexOf('}');

    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart) {
        return false;
    }

    jsonStr = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        Logger::instance().error("解析AI响应JSON失败: " + error.errorString());
        return false;
    }

    if (!jsonDoc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("function_name")) {
        functionName = jsonObj["function_name"].toString();
    }

    if (jsonObj.contains("function_description")) {
        functionDescription = jsonObj["function_description"].toString();
    }

    return !functionName.isEmpty() && !functionDescription.isEmpty();
}

void AIServiceManager::onReplyFinished(QNetworkReply* reply) {
    QMutexLocker locker(&m_mutex);

    if (reply != m_currentReply) {
        reply->deleteLater();
        return;
    }

    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = "网络请求失败: " + reply->errorString();
        emit analysisFailed(errorMsg);
        Logger::instance().error(errorMsg);
        reply->deleteLater();
        return;
    }

    emit analysisProgress("正在解析AI响应...");

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &error);

    if (error.error != QJsonParseError::NoError) {
        QString errorMsg = "解析响应JSON失败: " + error.errorString();
        emit analysisFailed(errorMsg);
        Logger::instance().error(errorMsg);
        return;
    }

    QString aiResponse = parseResponseJson(jsonDoc);

    if (aiResponse.isEmpty()) {
        QString errorMsg = "AI响应格式错误";
        emit analysisFailed(errorMsg);
        Logger::instance().error(errorMsg);
        return;
    }

    QString functionName;
    QString functionDescription;

    if (extractFunctionInfo(aiResponse, functionName, functionDescription)) {
        Logger::instance().info("AI分析完成，函数名称: " + functionName);
        emit analysisComplete(functionName, functionDescription);
    } else {
        QString errorMsg = "无法从AI响应中提取函数信息";
        emit analysisFailed(errorMsg);
        Logger::instance().error(errorMsg);
    }
}
