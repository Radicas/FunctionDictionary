/**
 * @file parseservice.h
 * @brief 解析服务实现类
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef PARSESERVICE_H
#define PARSESERVICE_H

#include "core/interfaces/iparseservice.h"
#include "core/parser/aicodeparser.h"
#include "core/parser/batchcodeparser.h"
#include "core/database/databasemanager.h"

/**
 * @brief 解析服务实现类
 * 
 * 该类负责：
 * - 协调 AICodeParser 和 BatchCodeParser
 * - 调用 DatabaseManager 存储结果
 * - 处理解析结果（跳过已存在等）
 * - 错误处理和重试
 */
class ParseService : public IParseService {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ParseService(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ParseService();

    /**
     * @brief 解析单个文件
     * @param filePath 文件路径
     */
    void parseFile(const QString& filePath) override;

    /**
     * @brief 解析文件夹
     * @param folderPath 文件夹路径
     * @param recursive 是否递归扫描
     */
    void parseFolder(const QString& folderPath, bool recursive) override;

    /**
     * @brief 取消解析
     */
    void cancelParsing() override;

    /**
     * @brief 检查是否正在解析
     * @return 是否正在解析
     */
    bool isParsing() const override;

    /**
     * @brief 设置是否跳过已存在的函数
     * @param skip 是否跳过
     */
    void setSkipExisting(bool skip) override;

private slots:
    /**
     * @brief AI代码解析完成槽函数（单文件）
     * @param result 解析结果
     */
    void onAIParseComplete(const AIParseResult& result);

    /**
     * @brief AI代码解析失败槽函数（单文件）
     * @param error 错误信息
     */
    void onAIParseFailed(const QString& error);

    /**
     * @brief AI代码解析进度槽函数（单文件）
     * @param stage 当前阶段
     * @param message 进度消息
     */
    void onAIParseProgress(const QString& stage, const QString& message);

    /**
     * @brief AI解析取消完成槽函数
     */
    void onAIParseCancelled();

    /**
     * @brief 批量解析进度槽函数
     * @param progress 进度信息
     */
    void onBatchProgress(const BatchParseProgress& progress);

    /**
     * @brief 单个文件解析完成槽函数（批量模式）
     * @param filePath 文件路径
     * @param result 解析结果
     */
    void onFileParsed(const QString& filePath, const AIParseResult& result);

    /**
     * @brief 批量解析完成槽函数
     * @param result 批量解析结果
     */
    void onBatchComplete(const BatchParseResult& result);

    /**
     * @brief 批量解析失败槽函数
     * @param error 错误信息
     */
    void onBatchFailed(const QString& error);

    /**
     * @brief 批量解析取消完成槽函数
     */
    void onBatchCancelled();

private:
    /**
     * @brief 处理单文件解析结果
     * @param result AI解析结果
     * @return 解析结果
     */
    ParseResult processSingleFileResult(const AIParseResult& result);

    /**
     * @brief 处理批量解析结果
     * @param result 批量解析结果
     * @return 解析结果
     */
    ParseResult processBatchResult(const BatchParseResult& result);

    bool m_skipExisting;                ///< 是否跳过已存在的函数
    bool m_isParsing;                   ///< 是否正在解析
    QString m_currentFilePath;          ///< 当前解析的文件路径
};

#endif // PARSESERVICE_H
