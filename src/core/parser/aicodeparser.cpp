/**
 * @file aicodeparser.cpp
 * @brief AI代码解析器实现
 * @author Developer
 * @date 2026-03-04
 * @version 1.0
 */

#include "aicodeparser.h"
#include "core/ai/aiconfigmanager.h"
#include "common/logger/logger.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>

AICodeParser::AICodeParser(QObject* parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr)
    , m_isParsing(false) {
    
    m_networkManager = new QNetworkAccessManager(this);
    Logger::instance().info("AI代码解析器初始化完成");
}

AICodeParser::~AICodeParser() {
    cancelParsing();
}

AICodeParser& AICodeParser::instance() {
    static AICodeParser instance;
    return instance;
}

void AICodeParser::parseFile(const QString& filePath) {
    if (m_isParsing) {
        emit parseFailed("正在解析其他文件，请稍候...");
        return;
    }
    
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    if (!AIConfigManager::instance().isConfigValid(config)) {
        emit parseFailed("AI配置不完整，请先配置AI服务！");
        Logger::instance().error("AI配置不完整，无法解析文件");
        return;
    }
    
    emit parseProgress("读取文件", "正在读取文件内容...");
    Logger::instance().info("开始解析文件: " + filePath);
    
    QString code;
    if (!readFileContent(filePath, code)) {
        emit parseFailed("无法读取文件: " + filePath);
        return;
    }
    
    QString language = detectLanguage(filePath);
    parseCode(code, language, filePath);
}

void AICodeParser::parseCode(const QString& code, const QString& language, const QString& filePath) {
    if (m_isParsing) {
        emit parseFailed("正在解析其他代码，请稍候...");
        return;
    }
    
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    if (!AIConfigManager::instance().isConfigValid(config)) {
        emit parseFailed("AI配置不完整，请先配置AI服务！");
        Logger::instance().error("AI配置不完整，无法解析代码");
        return;
    }
    
    m_isParsing = true;
    m_currentFilePath = filePath;
    m_currentLanguage = language;
    
    emit parseProgress("构建请求", "正在构建AI分析请求...");
    
    QString prompt = buildParsePrompt(code, language, filePath);
    
    QUrl url(buildRequestUrl());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(config.apiKey).toUtf8());
    
    QJsonObject jsonObj = buildRequestJson(prompt);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonData = jsonDoc.toJson();
    
    m_currentReply = m_networkManager->post(request, jsonData);
    
    connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
        onReplyFinished(m_currentReply);
    });
    
    emit parseProgress("发送请求", "已发送AI分析请求，等待响应...");
    Logger::instance().info("已发送AI代码解析请求，文件: " + filePath);
}

void AICodeParser::cancelParsing() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_isParsing = false;
    Logger::instance().info("已取消AI代码解析");
}

bool AICodeParser::isParsing() const {
    return m_isParsing;
}

QString AICodeParser::detectLanguage(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    QMap<QString, QString> extensionMap;
    extensionMap["cpp"] = "cpp";
    extensionMap["h"] = "cpp";
    extensionMap["hpp"] = "cpp";
    extensionMap["cc"] = "cpp";
    extensionMap["cxx"] = "cpp";
    extensionMap["py"] = "python";
    extensionMap["java"] = "java";
    extensionMap["js"] = "javascript";
    extensionMap["ts"] = "typescript";
    extensionMap["go"] = "go";
    extensionMap["rs"] = "rust";
    extensionMap["c"] = "c";
    extensionMap["cs"] = "csharp";
    extensionMap["rb"] = "ruby";
    extensionMap["php"] = "php";
    extensionMap["swift"] = "swift";
    extensionMap["kt"] = "kotlin";
    
    return extensionMap.value(suffix, "generic");
}

void AICodeParser::onReplyFinished(QNetworkReply* reply) {
    m_isParsing = false;
    
    if (reply != m_currentReply) {
        reply->deleteLater();
        return;
    }
    
    m_currentReply = nullptr;
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = "网络请求失败: " + reply->errorString();
        emit parseFailed(error);
        Logger::instance().error(error);
        return;
    }
    
    emit parseProgress("解析响应", "正在解析AI响应...");
    
    QByteArray responseData = reply->readAll();
    
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        QString errMsg = "解析响应JSON失败: " + error.errorString();
        emit parseFailed(errMsg);
        Logger::instance().error(errMsg);
        return;
    }
    
    QString aiResponse;
    
    if (jsonDoc.isObject()) {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.contains("choices")) {
            QJsonArray choicesArray = rootObj["choices"].toArray();
            if (!choicesArray.isEmpty()) {
                QJsonObject choiceObj = choicesArray[0].toObject();
                if (choiceObj.contains("message")) {
                    QJsonObject messageObj = choiceObj["message"].toObject();
                    aiResponse = messageObj["content"].toString();
                }
            }
        }
    }
    
    if (aiResponse.isEmpty()) {
        emit parseFailed("AI响应格式错误");
        Logger::instance().error("AI响应格式错误");
        return;
    }
    
    AIParseResult result = parseAIResponse(aiResponse, m_currentFilePath, m_currentLanguage);
    
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    result.aiModel = config.modelId;
    
    if (result.success) {
        emit parseComplete(result);
        Logger::instance().info(QString("AI代码解析完成，提取到 %1 个函数").arg(result.functions.size()));
    } else {
        emit parseFailed(result.errorMessage);
        Logger::instance().error("AI代码解析失败: " + result.errorMessage);
    }
}

QString AICodeParser::buildRequestUrl() const {
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    QString url = config.baseUrl;
    if (!url.endsWith("/")) {
        url += "/";
    }
    url += "chat/completions";
    return url;
}

QJsonObject AICodeParser::buildRequestJson(const QString& prompt) const {
    AIConfig config = AIConfigManager::instance().getCurrentConfig();

    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject jsonObj;
    jsonObj["model"] = config.modelId;
    jsonObj["messages"] = messagesArray;
    jsonObj["temperature"] = 0.3;

    return jsonObj;
}

QString AICodeParser::buildParsePrompt(const QString& code, const QString& language, const QString& filePath) const {
    QString prompt = QString(
        "你是一个专业的代码分析专家。请分析以下%1代码文件，提取所有函数的完整信息。\n\n"
        "文件路径：%2\n"
        "语言：%1\n\n"
        "代码：\n"
        "```%3\n"
        "%4\n"
        "```\n\n"
        "请返回JSON数组格式，包含文件中所有函数的信息：\n"
        "[\n"
        "  {\n"
        "    \"name\": \"函数名\",\n"
        "    \"signature\": \"完整函数签名\",\n"
        "    \"return_type\": \"返回类型\",\n"
        "    \"parameters\": [\n"
        "      {\"name\": \"参数名\", \"type\": \"参数类型\", \"description\": \"参数说明\"}\n"
        "    ],\n"
        "    \"start_line\": 起始行号(数字),\n"
        "    \"end_line\": 结束行号(数字),\n"
        "    \"description\": \"函数逻辑说明，使用Markdown格式，包括：\\n"
        "      ## 功能概述\\n"
        "      函数的主要功能说明\\n\\n"
        "      ## 参数说明\\n"
        "      各参数的详细说明\\n\\n"
        "      ## 返回值说明\\n"
        "      返回值的含义和可能的值\\n\\n"
        "      ## 使用示例\\n"
        "      代码示例\\n\\n"
        "      ## 注意事项\\n"
        "      使用时需要注意的问题\",\n"
        "    \"flowchart\": \"使用mermaid flowchart语法绘制的函数内部执行流程图，清晰展示所有分支和循环\",\n"
        "    \"sequence_diagram\": \"使用mermaid sequenceDiagram语法绘制的函数调用时序图，展示函数与其他模块的交互（如无外部调用可为空字符串）\",\n"
        "    \"structure_diagram\": \"使用mermaid graph语法绘制的模块依赖关系图，展示该函数在系统中的位置和依赖关系\"\n"
        "  }\n"
        "]\n\n"
        "重要要求：\n"
        "1. 必须提取文件中的所有函数，不要遗漏任何函数\n"
        "2. 函数逻辑说明要详细准确，使用中文\n"
        "3. 流程图必须使用正确的mermaid flowchart语法，清晰展示函数内部逻辑流程\n"
        "4. 时序图展示函数与外部组件的交互过程\n"
        "5. 结构图展示函数的依赖关系和在系统中的位置\n"
        "6. 所有mermaid图表语法必须正确，能够直接渲染\n"
        "7. 返回的必须是有效的JSON数组格式\n"
        "8. 如果代码中没有函数，返回空数组 []"
    ).arg(language)
     .arg(filePath)
     .arg(language == "cpp" ? "cpp" : language)
     .arg(code);
    
    return prompt;
}

AIParseResult AICodeParser::parseAIResponse(const QString& aiResponse, const QString& filePath, const QString& language) const {
    AIParseResult result;
    result.success = false;
    result.filePath = filePath;
    result.language = language;
    
    QString jsonStr = aiResponse.trimmed();
    
    int jsonStart = jsonStr.indexOf('[');
    int jsonEnd = jsonStr.lastIndexOf(']');
    
    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart) {
        jsonStart = jsonStr.indexOf('{');
        jsonEnd = jsonStr.lastIndexOf('}');
        
        if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart) {
            QString singleObj = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);
            jsonStr = "[" + singleObj + "]";
            jsonStart = 0;
            jsonEnd = jsonStr.length() - 1;
        }
    }
    
    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart) {
        result.errorMessage = "无法从AI响应中提取JSON数据";
        return result;
    }
    
    jsonStr = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);
    
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        result.errorMessage = "解析JSON失败: " + error.errorString();
        Logger::instance().error(result.errorMessage);
        return result;
    }
    
    if (!jsonDoc.isArray()) {
        result.errorMessage = "AI响应不是有效的JSON数组";
        return result;
    }
    
    QJsonArray functionsArray = jsonDoc.array();
    
    for (const QJsonValue& value : functionsArray) {
        if (value.isObject()) {
            QJsonObject funcObj = value.toObject();
            FunctionData funcData = parseFunctionFromJson(funcObj, filePath, language);
            result.functions.append(funcData);
        }
    }
    
    result.success = true;
    return result;
}

FunctionData AICodeParser::parseFunctionFromJson(const QJsonObject& jsonObj, const QString& filePath, const QString& language) const {
    FunctionData funcData;
    
    funcData.key = jsonObj["name"].toString();
    funcData.signature = jsonObj["signature"].toString();
    funcData.returnType = jsonObj["return_type"].toString();
    funcData.filePath = filePath;
    funcData.startLine = jsonObj["start_line"].toInt();
    funcData.endLine = jsonObj["end_line"].toInt();
    funcData.language = language;
    funcData.value = jsonObj["description"].toString();
    funcData.flowchart = jsonObj["flowchart"].toString();
    funcData.sequenceDiagram = jsonObj["sequence_diagram"].toString();
    funcData.structureDiagram = jsonObj["structure_diagram"].toString();
    funcData.createTime = QDateTime::currentDateTime();
    funcData.analyzeTime = QDateTime::currentDateTime();
    
    if (jsonObj.contains("parameters") && jsonObj["parameters"].isArray()) {
        QJsonArray paramsArray = jsonObj["parameters"].toArray();
        QJsonArray paramsJsonArray;
        for (const QJsonValue& paramValue : paramsArray) {
            if (paramValue.isObject()) {
                QJsonObject paramObj = paramValue.toObject();
                QJsonObject paramJson;
                paramJson["name"] = paramObj["name"].toString();
                paramJson["type"] = paramObj["type"].toString();
                paramJson["description"] = paramObj["description"].toString();
                paramsJsonArray.append(paramJson);
            }
        }
        funcData.parameters = QJsonDocument(paramsJsonArray).toJson(QJsonDocument::Compact);
    }
    
    return funcData;
}

int AICodeParser::countLines(const QString& code) const {
    return code.count('\n') + 1;
}

bool AICodeParser::readFileContent(const QString& filePath, QString& content) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::instance().error("无法打开文件: " + filePath + ", 错误: " + file.errorString());
        return false;
    }
    
    QTextStream in(&file);
    content = in.readAll();
    file.close();
    
    return true;
}
