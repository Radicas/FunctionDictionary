/**
 * @file batchcodeparser.cpp
 * @brief 批量代码解析器实现
 * @author Developer
 * @date 2026-03-04
 * @version 2.0
 */

#include "core/parser/batchcodeparser.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include "common/logger/logger.h"

BatchCodeParser::BatchCodeParser(IDatabaseManager* dbManager, QObject* parent)
    : QObject(parent),
      m_dbManager(dbManager),
      m_isParsing(false),
      m_skipExisting(true),
      m_cancelled(false),
      m_targetProjectId(-1)
{

    m_allowedExtensions = {"cpp", "h",  "hpp", "cc", "cxx", "py",  "java",  "js", "ts",
                           "go",  "rs", "c",   "cs", "rb",  "php", "swift", "kt"};

    m_excludeDirectories = {"node_modules", ".git",        ".svn",  "build",   "dist", "bin",
                            "obj",          "__pycache__", ".idea", ".vscode", "venv", "env"};

    AICodeParser& aiParser = AICodeParser::instance();
    connect(&aiParser, &AICodeParser::parseComplete, this, &BatchCodeParser::onFileParseComplete);
    connect(&aiParser, &AICodeParser::parseFailed, this, &BatchCodeParser::onFileParseFailed);
    connect(&aiParser, &AICodeParser::parseProgress, this, &BatchCodeParser::onFileParseProgress);

    Logger::instance().info("批量代码解析器初始化完成");
}

BatchCodeParser::~BatchCodeParser()
{
    cancelParsing();
}

void BatchCodeParser::parseFolder(const QString& folderPath, bool recursive)
{
    if (m_isParsing)
    {
        emit batchFailed("正在处理其他任务，请稍候...");
        return;
    }

    Logger::instance().info("开始扫描文件夹: " + folderPath + ", 递归: " + (recursive ? "是" : "否"));

    QStringList files;
    scanFolder(folderPath, files, recursive);

    if (files.isEmpty())
    {
        emit batchFailed("文件夹中没有找到代码文件");
        Logger::instance().warning("文件夹中没有找到代码文件: " + folderPath);
        return;
    }

    Logger::instance().info(QString("找到 %1 个代码文件").arg(files.size()));

    parseFiles(files);
}

void BatchCodeParser::parseFiles(const QStringList& filePaths)
{
    if (m_isParsing)
    {
        emit batchFailed("正在处理其他任务，请稍候...");
        return;
    }

    m_isParsing = true;
    m_cancelled = false;
    m_processedFiles.clear();
    m_fileQueue.clear();

    m_currentResult = BatchParseResult();
    m_currentResult.totalFiles = filePaths.size();
    m_currentResult.success = false;

    m_currentProgress = BatchParseProgress();
    m_currentProgress.totalFiles = filePaths.size();
    m_currentProgress.processedFiles = 0;
    m_currentProgress.successCount = 0;
    m_currentProgress.failedCount = 0;
    m_currentProgress.skippedCount = 0;

    for (const QString& filePath : filePaths)
    {
        m_fileQueue.enqueue(filePath);
    }

    Logger::instance().info(QString("开始批量解析 %1 个文件").arg(filePaths.size()));

    processNextFile();
}

void BatchCodeParser::cancelParsing()
{
    m_cancelled = true;
    m_isParsing = false;

    m_fileQueue.clear();

    AICodeParser::instance().cancelParsing();

    Logger::instance().info("已取消批量解析");
    emit batchCancelled();
}

bool BatchCodeParser::isParsing() const
{
    return m_isParsing;
}

void BatchCodeParser::setSkipExisting(bool skip)
{
    m_skipExisting = skip;
}

void BatchCodeParser::setFileExtensions(const QStringList& extensions)
{
    m_allowedExtensions = extensions;
}

void BatchCodeParser::setExcludeDirectories(const QStringList& directories)
{
    m_excludeDirectories = directories;
}

void BatchCodeParser::setTargetProject(int projectId)
{
    m_targetProjectId = projectId;
    Logger::instance().info(QString("设置批量解析目标项目ID: %1").arg(projectId));
}

void BatchCodeParser::setProjectRootPath(const QString& rootPath)
{
    m_projectRootPath = rootPath;
    Logger::instance().info(QString("设置项目根路径: %1").arg(rootPath));
}

void BatchCodeParser::scanFolder(const QString& folderPath, QStringList& files, bool recursive)
{
    QDir dir(folderPath);
    if (!dir.exists())
    {
        Logger::instance().warning("文件夹不存在: " + folderPath);
        return;
    }

    QDirIterator::IteratorFlags flags = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it(folderPath, QDir::Files | QDir::NoSymLinks, flags);

    while (it.hasNext())
    {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        QString relativePath = dir.relativeFilePath(filePath);
        bool shouldExclude = false;

        for (const QString& excludeDir : m_excludeDirectories)
        {
            if (relativePath.contains(excludeDir + "/") || relativePath.startsWith(excludeDir))
            {
                shouldExclude = true;
                break;
            }
        }

        if (shouldExclude)
        {
            continue;
        }

        if (isFileExtensionAllowed(filePath))
        {
            files.append(filePath);
        }
    }
}

bool BatchCodeParser::isFileExtensionAllowed(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return m_allowedExtensions.contains(suffix);
}

void BatchCodeParser::processNextFile()
{
    if (m_cancelled)
    {
        finishBatch();
        return;
    }

    if (m_fileQueue.isEmpty())
    {
        finishBatch();
        return;
    }

    m_currentFile = m_fileQueue.dequeue();
    m_currentProgress.currentFile = m_currentFile;
    m_currentProgress.currentStage = "准备";
    m_currentProgress.currentMessage = "准备解析: " + QFileInfo(m_currentFile).fileName();

    emitProgress();

    Logger::instance().info(QString("开始解析文件 (%1/%2): %3")
                                .arg(m_currentProgress.processedFiles + 1)
                                .arg(m_currentProgress.totalFiles)
                                .arg(m_currentFile));

    AICodeParser::instance().parseFile(m_currentFile);
}

void BatchCodeParser::emitProgress()
{
    emit batchProgress(m_currentProgress);
}

void BatchCodeParser::finishBatch()
{
    m_isParsing = false;

    m_currentResult.success = (m_currentResult.failedCount == 0);

    Logger::instance().info(QString("批量解析完成 - 总计: %1, 成功: %2, 失败: %3, 跳过: %4")
                                .arg(m_currentResult.totalFiles)
                                .arg(m_currentResult.successCount)
                                .arg(m_currentResult.failedCount)
                                .arg(m_currentResult.skippedCount));

    emit batchComplete(m_currentResult);
}

void BatchCodeParser::onFileParseComplete(const AIParseResult& result)
{
    m_processedFiles.insert(m_currentFile);
    m_currentProgress.processedFiles++;

    if (result.success && !result.functions.isEmpty())
    {
        int savedCount = 0;

        QString relativePath = result.filePath;
        if (!m_projectRootPath.isEmpty() && result.filePath.startsWith(m_projectRootPath))
        {
            relativePath = result.filePath.mid(m_projectRootPath.length());
            if (relativePath.startsWith("/"))
            {
                relativePath = relativePath.mid(1);
            }
        }

        for (const FunctionData& funcData : result.functions)
        {
            if (m_skipExisting && m_dbManager->functionExists(funcData.key))
            {
                m_currentProgress.skippedCount++;
                m_currentResult.skippedCount++;
                Logger::instance().info("跳过已存在的函数: " + funcData.key);
            }
            else
            {
                FunctionData data = funcData;
                data.createTime = QDateTime::currentDateTime();
                data.analyzeTime = QDateTime::currentDateTime();
                data.filePath = relativePath;

                if (m_targetProjectId > 0)
                {
                    data.projectId = m_targetProjectId;
                }
                else
                {
                    ProjectInfo tempProject = m_dbManager->getOrCreateTemporaryProject();
                    data.projectId = tempProject.id;
                }

                if (m_dbManager->addFunction(data))
                {
                    savedCount++;
                    m_currentResult.allFunctions.append(data);
                }
            }
        }

        m_currentResult.successCount++;
        m_currentProgress.successCount++;

        Logger::instance().info(QString("文件解析成功: %1, 提取 %2 个函数, 保存 %3 个")
                                    .arg(m_currentFile)
                                    .arg(result.functions.size())
                                    .arg(savedCount));
    }
    else if (result.functions.isEmpty())
    {
        m_currentProgress.skippedCount++;
        m_currentResult.skippedCount++;
        Logger::instance().info("文件中未找到函数: " + m_currentFile);
    }
    else
    {
        m_currentResult.failedCount++;
        m_currentProgress.failedCount++;
        m_currentResult.failedFiles.append(m_currentFile);
        Logger::instance().warning("文件解析失败: " + m_currentFile);
    }

    emit fileParsed(m_currentFile, result);
    emitProgress();

    QTimer::singleShot(100, this, &BatchCodeParser::processNextFile);
}

void BatchCodeParser::onFileParseFailed(const QString& error)
{
    m_processedFiles.insert(m_currentFile);
    m_currentProgress.processedFiles++;
    m_currentResult.failedCount++;
    m_currentProgress.failedCount++;
    m_currentResult.failedFiles.append(m_currentFile);

    Logger::instance().error(QString("文件解析失败: %1, 错误: %2").arg(m_currentFile).arg(error));

    emitProgress();

    QTimer::singleShot(100, this, &BatchCodeParser::processNextFile);
}

void BatchCodeParser::onFileParseProgress(const QString& stage, const QString& message)
{
    m_currentProgress.currentStage = stage;
    m_currentProgress.currentMessage = message;
    emitProgress();
}
