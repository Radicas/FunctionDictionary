/**
 * @file aicodeparser.h
 * @brief AI代码解析器，使用AI从源代码文件中提取函数完整信息
 * @author Developer
 * @date 2026-03-04
 * @version 1.0
 * 
 * @details 该解析器使用AI模型分析源代码文件，一次性提取：
 * - 函数基本信息（名称、签名、参数、返回类型）
 * - 函数逻辑说明
 * - 流程图（Mermaid flowchart）
 * - 时序图（Mermaid sequenceDiagram）
 * - 模块依赖图（Mermaid graph）
 */

#ifndef AICODEPARSER_H
#define AICODEPARSER_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVector>
#include "core/models/functiondata.h"

/**
 * @brief AI代码解析结果结构体
 */
struct AIParseResult
{
    bool success;                     ///< 是否成功
    QString filePath;                 ///< 文件路径
    QString language;                 ///< 语言类型
    QVector<FunctionData> functions;  ///< 提取的函数列表
    QString errorMessage;             ///< 错误信息
    int totalLines;                   ///< 文件总行数
    QString aiModel;                  ///< 使用的AI模型
};

/**
 * @brief AI代码解析器类，使用AI从源代码中提取函数完整信息
 * 
 * @details 该类采用单例模式，提供：
 * - 基于AI的智能代码解析
 * - 一次性提取函数信息、逻辑说明、图表
 * - 支持多种编程语言
 * - 异步处理机制
 */
class AICodeParser : public QObject
{
    Q_OBJECT

   public:
    /**
     * @brief 获取AICodeParser的单例实例
     * @return AICodeParser的引用
     */
    static AICodeParser& instance();

    AICodeParser(const AICodeParser&) = delete;
    AICodeParser& operator=(const AICodeParser&) = delete;

    /**
     * @brief 解析源代码文件（异步）
     * @param filePath 源文件路径
     */
    void parseFile(const QString& filePath);

    /**
     * @brief 解析代码文本（异步）
     * @param code 代码文本
     * @param language 语言类型
     * @param filePath 文件路径（可选，用于记录来源）
     */
    void parseCode(const QString& code, const QString& language, const QString& filePath = "");

    /**
     * @brief 取消当前解析请求
     */
    void cancelParsing();

    /**
     * @brief 检查是否正在解析
     * @return 是否正在解析
     */
    bool isParsing() const;

    /**
     * @brief 检测文件语言类型
     * @param filePath 文件路径
     * @return 语言类型标识符
     */
    QString detectLanguage(const QString& filePath) const;

    /**
     * @brief 设置请求超时时间
     * @param timeoutMs 超时时间（毫秒）
     */
    void setTimeout(int timeoutMs);

   signals:
    /**
     * @brief 解析完成信号
     * @param result 解析结果
     */
    void parseComplete(const AIParseResult& result);

    /**
     * @brief 解析失败信号
     * @param error 错误信息
     */
    void parseFailed(const QString& error);

    /**
     * @brief 解析进度信号
     * @param stage 当前阶段
     * @param message 进度消息
     */
    void parseProgress(const QString& stage, const QString& message);

    /**
     * @brief 解析取消完成信号
     */
    void parseCancelled();

   private slots:
    /**
     * @brief 网络请求完成槽函数
     */
    void onReplyFinished();

    /**
     * @brief 网络错误槽函数
     * @param error 网络错误
     */
    void onNetworkError(QNetworkReply::NetworkError error);

    /**
     * @brief 超时槽函数
     */
    void onTimeout();

   private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AICodeParser(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AICodeParser();

    /**
     * @brief 构建AI请求URL
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
     * @brief 构建代码解析提示词
     * @param code 源代码
     * @param language 语言类型
     * @param filePath 文件路径
     * @return 提示词
     */
    QString buildParsePrompt(const QString& code, const QString& language, const QString& filePath) const;

    /**
     * @brief 解析AI响应
     * @param aiResponse AI响应字符串
     * @param filePath 文件路径
     * @param language 语言类型
     * @return 解析结果
     */
    AIParseResult parseAIResponse(const QString& aiResponse, const QString& filePath, const QString& language) const;

    /**
     * @brief 从JSON对象解析单个函数
     * @param jsonObj JSON对象
     * @param filePath 文件路径
     * @param language 语言类型
     * @return 函数数据
     */
    FunctionData parseFunctionFromJson(const QJsonObject& jsonObj, const QString& filePath,
                                       const QString& language) const;

    /**
     * @brief 计算代码行数
     * @param code 代码文本
     * @return 行数
     */
    int countLines(const QString& code) const;

    /**
     * @brief 读取文件内容
     * @param filePath 文件路径
     * @param content 输出参数，文件内容
     * @return 是否成功
     */
    bool readFileContent(const QString& filePath, QString& content) const;

    /**
     * @brief 处理流式响应数据
     */
    void onReadyRead();

    /**
     * @brief 解析SSE数据行
     * @param line 数据行
     * @return 是否成功解析
     */
    bool parseSSELine(const QString& line);

    QNetworkAccessManager* m_networkManager;  ///< 网络访问管理器
    QNetworkReply* m_currentReply;            ///< 当前网络回复
    QTimer* m_timeoutTimer;                   ///< 超时定时器
    QString m_currentFilePath;                ///< 当前解析的文件路径
    QString m_currentLanguage;                ///< 当前解析的语言类型
    bool m_isParsing;                         ///< 是否正在解析
    int m_timeoutMs;                          ///< 超时时间（毫秒）
    QString m_streamBuffer;                   ///< 流式响应缓冲区
    QString m_streamContent;                  ///< 流式响应完整内容
    int m_receivedTokens;                     ///< 已接收的token数
};

#endif  // AICODEPARSER_H
