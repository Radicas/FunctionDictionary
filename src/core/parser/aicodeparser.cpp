/**
 * @file aicodeparser.cpp
 * @brief AI代码解析器实现
 * @author Developer
 * @date 2026-03-04
 * @version 1.0
 */

#include "core/parser/aicodeparser.h"
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
    , m_timeoutTimer(nullptr)
    , m_isParsing(false)
    , m_timeoutMs(120000) {
    
    m_networkManager = new QNetworkAccessManager(this);
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    
    connect(m_timeoutTimer, &QTimer::timeout, this, &AICodeParser::onTimeout);
    
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
    Logger::instance().info(QString("检测到语言类型: %1, 文件大小: %2 字节").arg(language).arg(code.size()));
    
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
    Logger::instance().info(QString("构建Prompt完成, 长度: %1 字符").arg(prompt.size()));
    
    QString urlStr = buildRequestUrl();
    Logger::instance().info("请求URL: " + urlStr);
    Logger::instance().info("使用模型: " + config.modelId);
    
    QUrl requestUrl(urlStr);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(config.apiKey).toUtf8());
    request.setRawHeader("Accept", "application/json");
    
    QJsonObject jsonObj = buildRequestJson(prompt);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonData = jsonDoc.toJson();
    
    Logger::instance().info(QString("请求JSON大小: %1 字节").arg(jsonData.size()));
    
    m_currentReply = m_networkManager->post(request, jsonData);
    
    connect(m_currentReply, &QNetworkReply::finished, this, &AICodeParser::onReplyFinished);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &AICodeParser::onNetworkError);
    
    m_timeoutTimer->start(m_timeoutMs);
    
    emit parseProgress("发送请求", "已发送AI分析请求，等待响应...");
    Logger::instance().info("已发送AI代码解析请求，文件: " + filePath);
}

void AICodeParser::cancelParsing() {
    m_timeoutTimer->stop();
    
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_isParsing = false;
    Logger::instance().info("已取消AI代码解析");
    emit parseCancelled();
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

void AICodeParser::setTimeout(int timeoutMs) {
    m_timeoutMs = timeoutMs;
}

void AICodeParser::onReplyFinished() {
    m_timeoutTimer->stop();
    m_isParsing = false;
    
    if (!m_currentReply) {
        Logger::instance().error("onReplyFinished: m_currentReply 为空");
        return;
    }
    
    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;
    
    Logger::instance().info("收到AI响应");
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = "网络请求失败: " + reply->errorString() + " (错误码: " + QString::number(reply->error()) + ")";
        Logger::instance().error(error);
        emit parseFailed(error);
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    reply->deleteLater();
    
    Logger::instance().info(QString("响应数据大小: %1 字节").arg(responseData.size()));
    
    if (responseData.isEmpty()) {
        emit parseFailed("AI响应为空");
        Logger::instance().error("AI响应为空");
        return;
    }
    
    emit parseProgress("解析响应", "正在解析AI响应...");
    
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errMsg = "解析响应JSON失败: " + parseError.errorString() + " (位置: " + QString::number(parseError.offset) + ")";
        Logger::instance().error(errMsg);
        Logger::instance().error("响应内容前500字符: " + QString(responseData.left(500)));
        emit parseFailed(errMsg);
        return;
    }
    
    QString aiResponse;
    
    if (jsonDoc.isObject()) {
        QJsonObject rootObj = jsonDoc.object();
        
        if (rootObj.contains("error")) {
            QString errorMsg = rootObj["error"].toObject()["message"].toString();
            if (errorMsg.isEmpty()) {
                errorMsg = rootObj["error"].toString();
            }
            emit parseFailed("API错误: " + errorMsg);
            Logger::instance().error("API错误: " + errorMsg);
            return;
        }
        
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
        emit parseFailed("AI响应格式错误或内容为空");
        Logger::instance().error("AI响应格式错误或内容为空");
        Logger::instance().error("响应JSON: " + QString(jsonDoc.toJson(QJsonDocument::Compact).left(1000)));
        return;
    }
    
    Logger::instance().info(QString("AI响应内容长度: %1 字符").arg(aiResponse.size()));
    
    AIParseResult result = parseAIResponse(aiResponse, m_currentFilePath, m_currentLanguage);

    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    result.aiModel = config.modelId;

    if (result.success) {
        emit parseProgress("保存数据", "正在保存解析结果...");
        emit parseComplete(result);
        Logger::instance().info(QString("AI代码解析完成，提取到 %1 个函数").arg(result.functions.size()));
    } else {
        emit parseFailed(result.errorMessage);
        Logger::instance().error("AI代码解析失败: " + result.errorMessage);
    }
}

void AICodeParser::onNetworkError(QNetworkReply::NetworkError error) {
    Logger::instance().error(QString("网络错误: %1 (错误码: %2)").arg(m_currentReply ? m_currentReply->errorString() : "未知").arg(error));
}

void AICodeParser::onTimeout() {
    Logger::instance().error(QString("请求超时 (%1 ms)").arg(m_timeoutMs));
    
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_isParsing = false;
    
    emit parseFailed(QString("请求超时 (%1 秒)，请检查网络连接或增加超时时间").arg(m_timeoutMs / 1000));
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
    jsonObj["max_tokens"] = 16000;

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
        Logger::instance().error(result.errorMessage);
        Logger::instance().error("AI响应前500字符: " + aiResponse.left(500));
        return result;
    }
    
    jsonStr = jsonStr.mid(jsonStart, jsonEnd - jsonStart + 1);
    
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        result.errorMessage = "解析JSON失败: " + error.errorString() + " (位置: " + QString::number(error.offset) + ")";
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
    in.setEncoding(QStringConverter::Utf8);
    content = in.readAll();
    file.close();
    
    return true;
}
