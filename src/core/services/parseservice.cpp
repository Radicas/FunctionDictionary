/**
 * @file parseservice.cpp
 * @brief 解析服务实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "core/services/parseservice.h"
#include "common/logger/logger.h"
#include <QDateTime>

ParseService::ParseService(QObject* parent)
    : IParseService(parent), m_skipExisting(true), m_isParsing(false), m_targetProjectId(-1)
{
    AICodeParser &aiParser = AICodeParser::instance();
    connect(&aiParser, &AICodeParser::parseComplete,
            this, &ParseService::onAIParseComplete);
    connect(&aiParser, &AICodeParser::parseFailed,
            this, &ParseService::onAIParseFailed);
    connect(&aiParser, &AICodeParser::parseProgress,
            this, &ParseService::onAIParseProgress);
    connect(&aiParser, &AICodeParser::parseCancelled,
            this, &ParseService::onAIParseCancelled);

    BatchCodeParser &batchParser = BatchCodeParser::instance();
    connect(&batchParser, &BatchCodeParser::batchProgress,
            this, &ParseService::onBatchProgress);
    connect(&batchParser, &BatchCodeParser::fileParsed,
            this, &ParseService::onFileParsed);
    connect(&batchParser, &BatchCodeParser::batchComplete,
            this, &ParseService::onBatchComplete);
    connect(&batchParser, &BatchCodeParser::batchFailed,
            this, &ParseService::onBatchFailed);
    connect(&batchParser, &BatchCodeParser::batchCancelled,
            this, &ParseService::onBatchCancelled);

    Logger::instance().info("解析服务初始化完成");
}

ParseService::~ParseService()
{
}

void ParseService::parseFile(const QString& filePath)
{
    if (m_isParsing) {
        emit parseFailed("正在解析其他文件，请稍候...");
        return;
    }

    m_isParsing = true;
    m_currentFilePath = filePath;
    
    Logger::instance().info("开始解析文件: " + filePath);
    AICodeParser::instance().parseFile(filePath);
}

void ParseService::parseFolder(const QString& folderPath, bool recursive)
{
    if (m_isParsing) {
        emit parseFailed("正在解析其他文件，请稍候...");
        return;
    }

    m_isParsing = true;
    
    Logger::instance().info(QString("开始批量解析文件夹: %1, 递归: %2")
                               .arg(folderPath)
                               .arg(recursive));

    BatchCodeParser::instance().setSkipExisting(m_skipExisting);
    BatchCodeParser::instance().setTargetProject(m_targetProjectId);
    
    if (m_targetProjectId > 0) {
        ProjectInfo project = DatabaseManager::instance().getProjectById(m_targetProjectId);
        if (project.id > 0) {
            BatchCodeParser::instance().setProjectRootPath(project.rootPath);
        }
    } else {
        BatchCodeParser::instance().setProjectRootPath(folderPath);
    }
    
    BatchCodeParser::instance().parseFolder(folderPath, recursive);
}

void ParseService::cancelParsing()
{
    if (!m_isParsing) {
        return;
    }

    Logger::instance().info("取消解析");
    
    if (AICodeParser::instance().isParsing()) {
        AICodeParser::instance().cancelParsing();
    }
    
    if (BatchCodeParser::instance().isParsing()) {
        BatchCodeParser::instance().cancelParsing();
    }
}

bool ParseService::isParsing() const
{
    return m_isParsing;
}

void ParseService::setSkipExisting(bool skip)
{
    m_skipExisting = skip;
    Logger::instance().info(QString("设置跳过已存在: %1").arg(skip ? "是" : "否"));
}

void ParseService::setTargetProject(int projectId)
{
    m_targetProjectId = projectId;
    Logger::instance().info(QString("设置目标项目ID: %1").arg(projectId));
}

int ParseService::targetProject() const
{
    return m_targetProjectId;
}

void ParseService::onAIParseComplete(const AIParseResult& result)
{
    ParseResult parseResult = processSingleFileResult(result);
    m_isParsing = false;
    emit parseComplete(parseResult);
}

void ParseService::onAIParseFailed(const QString& error)
{
    m_isParsing = false;
    Logger::instance().error("AI 解析失败: " + error);
    emit parseFailed(error);
}

void ParseService::onAIParseProgress(const QString& stage, const QString& message)
{
    ParseProgress progress;
    progress.stage = stage;
    progress.message = message;
    
    if (stage == "读取文件") {
        progress.current = 10;
    } else if (stage == "构建请求") {
        progress.current = 20;
    } else if (stage == "发送请求") {
        progress.current = 30;
    } else if (stage == "解析响应") {
        progress.current = 70;
    } else if (stage == "保存数据") {
        progress.current = 90;
    }
    progress.total = 100;
    
    emit parseProgress(progress);
}

void ParseService::onAIParseCancelled()
{
    m_isParsing = false;
    Logger::instance().info("AI解析已取消");
    emit parseCancelled();
}

void ParseService::onBatchProgress(const BatchParseProgress& progress)
{
    ParseProgress progressInfo;
    progressInfo.current = progress.processedFiles;
    progressInfo.total = progress.totalFiles;
    progressInfo.successCount = progress.successCount;
    progressInfo.failedCount = progress.failedCount;
    progressInfo.skippedCount = progress.skippedCount;
    progressInfo.stage = progress.currentStage;
    progressInfo.message = progress.currentMessage;
    
    emit parseProgress(progressInfo);
}

void ParseService::onFileParsed(const QString& filePath, const AIParseResult& result)
{
    Logger::instance().info(QString("文件解析完成: %1, 提取 %2 个函数")
                               .arg(filePath)
                               .arg(result.functions.size()));
}

void ParseService::onBatchComplete(const BatchParseResult& result)
{
    ParseResult parseResult = processBatchResult(result);
    m_isParsing = false;
    emit parseComplete(parseResult);
}

void ParseService::onBatchFailed(const QString& error)
{
    m_isParsing = false;
    Logger::instance().error("批量解析失败: " + error);
    emit parseFailed(error);
}

void ParseService::onBatchCancelled()
{
    m_isParsing = false;
    Logger::instance().info("批量解析已取消");
    emit parseCancelled();
}

ParseResult ParseService::processSingleFileResult(const AIParseResult& result)
{
    ParseResult parseResult;
    parseResult.filePath = result.filePath;
    parseResult.success = result.success;
    
    if (!result.success) {
        parseResult.errorMessage = result.errorMessage;
        return parseResult;
    }

    int successCount = 0;
    int failedCount = 0;
    int skippedCount = 0;

    QString relativePath = result.filePath;
    
    if (m_targetProjectId > 0) {
        ProjectInfo project = DatabaseManager::instance().getProjectById(m_targetProjectId);
        if (project.id > 0 && !project.rootPath.isEmpty()) {
            if (result.filePath.startsWith(project.rootPath)) {
                relativePath = result.filePath.mid(project.rootPath.length());
                if (relativePath.startsWith("/")) {
                    relativePath = relativePath.mid(1);
                }
            }
        }
    }

    for (const FunctionData &funcData : result.functions) {
        if (m_skipExisting && DatabaseManager::instance().functionExists(funcData.key)) {
            skippedCount++;
            Logger::instance().info(QString("[跳过] %1 - 已存在").arg(funcData.key));
        } else {
            FunctionData data = funcData;
            data.createTime = QDateTime::currentDateTime();
            data.analyzeTime = QDateTime::currentDateTime();
            data.filePath = relativePath;
            
            if (m_targetProjectId > 0) {
                data.projectId = m_targetProjectId;
            } else {
                ProjectInfo tempProject = DatabaseManager::instance().getOrCreateTemporaryProject();
                data.projectId = tempProject.id;
            }

            if (DatabaseManager::instance().addFunction(data)) {
                successCount++;
                parseResult.functions.append(data);
                Logger::instance().info(QString("[成功] %1").arg(funcData.key));
            } else {
                failedCount++;
                Logger::instance().error(QString("[失败] %1 - %2")
                                            .arg(funcData.key)
                                            .arg(DatabaseManager::instance().lastError()));
            }
        }
    }

    parseResult.successCount = successCount;
    parseResult.failedCount = failedCount;
    parseResult.skippedCount = skippedCount;
    
    Logger::instance().info(QString("解析完成！成功: %1, 失败: %2, 跳过: %3")
                               .arg(successCount)
                               .arg(failedCount)
                               .arg(skippedCount));
    
    return parseResult;
}

ParseResult ParseService::processBatchResult(const BatchParseResult& result)
{
    ParseResult parseResult;
    parseResult.success = result.success;
    parseResult.successCount = result.successCount;
    parseResult.failedCount = result.failedCount;
    parseResult.skippedCount = result.skippedCount;
    parseResult.functions = result.allFunctions;
    
    if (!result.success) {
        parseResult.errorMessage = result.errorMessage;
    }
    
    Logger::instance().info(QString("批量解析完成！总计: %1 个文件, 成功: %2, 失败: %3, 跳过: %4, 提取函数: %5 个")
                               .arg(result.totalFiles)
                               .arg(result.successCount)
                               .arg(result.failedCount)
                               .arg(result.skippedCount)
                               .arg(result.allFunctions.size()));
    
    return parseResult;
}
