/**
 * @file batchcodeparser.h
 * @brief 批量代码解析器，支持文件夹递归扫描和批量处理
 * @author Developer
 * @date 2026-03-04
 * @version 1.0
 * 
 * @details 该解析器基于 AICodeParser 实现：
 * - 递归扫描文件夹中的代码文件
 * - 维护文件处理队列
 * - 逐个调用 AICodeParser 进行解析
 * - 提供进度回调和错误处理
 */

#ifndef BATCHCODEPARSER_H
#define BATCHCODEPARSER_H

#include "aicodeparser.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QSet>

/**
 * @brief 批量解析进度信息
 */
struct BatchParseProgress {
    int totalFiles;             ///< 总文件数
    int processedFiles;         ///< 已处理文件数
    int successCount;           ///< 成功数量
    int failedCount;            ///< 失败数量
    int skippedCount;           ///< 跳过数量
    QString currentFile;        ///< 当前处理的文件
    QString currentStage;       ///< 当前阶段
    QString currentMessage;     ///< 当前消息
};

/**
 * @brief 批量解析结果
 */
struct BatchParseResult {
    bool success;                       ///< 是否全部成功
    int totalFiles;                     ///< 总文件数
    int successCount;                   ///< 成功数量
    int failedCount;                    ///< 失败数量
    int skippedCount;                   ///< 跳过数量
    QVector<FunctionData> allFunctions; ///< 所有提取的函数
    QStringList failedFiles;            ///< 失败的文件列表
    QString errorMessage;               ///< 错误信息
};

/**
 * @brief 批量代码解析器类
 * 
 * @details 该类采用单例模式，提供：
 * - 文件夹递归扫描
 * - 批量文件处理队列
 * - 进度回调
 * - 错误处理和重试
 */
class BatchCodeParser : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取BatchCodeParser的单例实例
     * @return BatchCodeParser的引用
     */
    static BatchCodeParser& instance();
    
    BatchCodeParser(const BatchCodeParser&) = delete;
    BatchCodeParser& operator=(const BatchCodeParser&) = delete;

    /**
     * @brief 解析文件夹中的所有代码文件
     * @param folderPath 文件夹路径
     * @param recursive 是否递归扫描子文件夹
     */
    void parseFolder(const QString& folderPath, bool recursive = true);

    /**
     * @brief 解析多个文件
     * @param filePaths 文件路径列表
     */
    void parseFiles(const QStringList& filePaths);

    /**
     * @brief 取消当前批量解析
     */
    void cancelParsing();

    /**
     * @brief 检查是否正在解析
     * @return 是否正在解析
     */
    bool isParsing() const;

    /**
     * @brief 设置是否跳过已存在的函数
     * @param skip 是否跳过
     */
    void setSkipExisting(bool skip);

    /**
     * @brief 设置要处理的文件扩展名
     * @param extensions 扩展名列表（如 {"cpp", "h", "py"}）
     */
    void setFileExtensions(const QStringList& extensions);

    /**
     * @brief 设置要排除的目录名
     * @param directories 目录名列表（如 {"node_modules", ".git"}）
     */
    void setExcludeDirectories(const QStringList& directories);

signals:
    /**
     * @brief 批量解析进度信号
     * @param progress 进度信息
     */
    void batchProgress(const BatchParseProgress& progress);

    /**
     * @brief 单个文件解析完成信号
     * @param filePath 文件路径
     * @param result 解析结果
     */
    void fileParsed(const QString& filePath, const AIParseResult& result);

    /**
     * @brief 批量解析完成信号
     * @param result 批量解析结果
     */
    void batchComplete(const BatchParseResult& result);

    /**
     * @brief 批量解析失败信号
     * @param error 错误信息
     */
    void batchFailed(const QString& error);

private slots:
    /**
     * @brief 单个文件解析完成槽函数
     * @param result 解析结果
     */
    void onFileParseComplete(const AIParseResult& result);

    /**
     * @brief 单个文件解析失败槽函数
     * @param error 错误信息
     */
    void onFileParseFailed(const QString& error);

    /**
     * @brief 单个文件解析进度槽函数
     * @param stage 当前阶段
     * @param message 进度消息
     */
    void onFileParseProgress(const QString& stage, const QString& message);

private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BatchCodeParser(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BatchCodeParser();

    /**
     * @brief 递归扫描文件夹
     * @param folderPath 文件夹路径
     * @param files 输出参数，找到的文件列表
     */
    void scanFolder(const QString& folderPath, QStringList& files);

    /**
     * @brief 检查文件扩展名是否在允许列表中
     * @param filePath 文件路径
     * @return 是否允许
     */
    bool isFileExtensionAllowed(const QString& filePath) const;

    /**
     * @brief 处理下一个文件
     */
    void processNextFile();

    /**
     * @brief 发送进度信号
     */
    void emitProgress();

    /**
     * @brief 完成批量解析
     */
    void finishBatch();

    QQueue<QString> m_fileQueue;            ///< 文件队列
    QSet<QString> m_processedFiles;         ///< 已处理的文件集合
    
    bool m_isParsing;                       ///< 是否正在解析
    bool m_skipExisting;                    ///< 是否跳过已存在的函数
    bool m_cancelled;                       ///< 是否已取消
    
    QStringList m_allowedExtensions;        ///< 允许的文件扩展名
    QStringList m_excludeDirectories;       ///< 排除的目录名
    
    BatchParseResult m_currentResult;       ///< 当前批量解析结果
    BatchParseProgress m_currentProgress;   ///< 当前进度
    QString m_currentFile;                  ///< 当前处理的文件
};

#endif // BATCHCODEPARSER_H
