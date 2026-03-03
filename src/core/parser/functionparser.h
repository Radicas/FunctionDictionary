/**
 * @file functionparser.h
 * @brief 函数解析器，支持多种编程语言的函数提取
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef FUNCTIONPARSER_H
#define FUNCTIONPARSER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QRegularExpression>
#include <functional>
#include "core/models/extractedfunction.h"

/**
 * @brief 函数解析器类，支持多种编程语言的函数提取
 * 
 * 该类使用单例模式，提供从源代码文件中提取函数定义的功能。
 * 支持多种编程语言，包括C/C++、Python、Java、JavaScript等。
 */
class FunctionParser : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取FunctionParser的单例实例
     * @return FunctionParser的引用
     */
    static FunctionParser& instance();

    FunctionParser(const FunctionParser&) = delete;
    FunctionParser& operator=(const FunctionParser&) = delete;

    /**
     * @brief 从源文件提取所有函数
     * @param filePath 源文件路径
     * @return 提取结果，包含函数列表和状态信息
     */
    ExtractionResult extractFunctions(const QString& filePath);

    /**
     * @brief 从代码文本提取函数
     * @param code 代码文本
     * @param language 语言类型
     * @return 提取结果，包含函数列表和状态信息
     */
    ExtractionResult extractFromCode(const QString& code, const QString& language);

    /**
     * @brief 检测文件语言类型
     * @param filePath 文件路径
     * @return 语言类型标识符（如"cpp"、"python"、"java"等）
     */
    QString detectLanguage(const QString& filePath) const;

    /**
     * @brief 检查是否支持指定语言
     * @param language 语言类型
     * @return 是否支持
     */
    bool isLanguageSupported(const QString& language) const;

signals:
    /**
     * @brief 提取进度信号
     * @param current 当前进度
     * @param total 总数
     * @param message 进度消息
     */
    void extractionProgress(int current, int total, const QString& message);

private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FunctionParser(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~FunctionParser();

    /**
     * @brief 初始化语言解析器映射
     */
    void initializeParsers();

    /**
     * @brief C/C++ 代码解析器
     * @param code 源代码
     * @return 提取结果
     */
    ExtractionResult parseCpp(const QString& code);

    /**
     * @brief Python 代码解析器
     * @param code 源代码
     * @return 提取结果
     */
    ExtractionResult parsePython(const QString& code);

    /**
     * @brief Java 代码解析器
     * @param code 源代码
     * @return 提取结果
     */
    ExtractionResult parseJava(const QString& code);

    /**
     * @brief JavaScript/TypeScript 代码解析器
     * @param code 源代码
     * @return 提取结果
     */
    ExtractionResult parseJavaScript(const QString& code);

    /**
     * @brief 通用正则解析器（作为后备方案）
     * @param code 源代码
     * @param language 语言类型
     * @return 提取结果
     */
    ExtractionResult parseGeneric(const QString& code, const QString& language);

    /**
     * @brief 提取函数体（平衡括号匹配）
     * @param code 完整代码
     * @param startPos 函数体起始位置
     * @return 函数体代码
     */
    QString extractFunctionBody(const QString& code, int startPos) const;

    /**
     * @brief 解析参数列表
     * @param paramsStr 参数字符串
     * @return 参数信息列表
     */
    QVector<ParameterInfo> parseParameters(const QString& paramsStr) const;

    /**
     * @brief 计算代码行数
     * @param code 代码文本
     * @return 行数
     */
    int countLines(const QString& code) const;

    /**
     * @brief 获取位置对应的行号
     * @param code 代码文本
     * @param pos 字符位置
     * @return 行号（从1开始）
     */
    int getLineNumber(const QString& code, int pos) const;

    QMap<QString, std::function<ExtractionResult(const QString&)>> m_parsers;  ///< 语言解析器映射
    QMap<QString, QStringList> m_languageExtensions;                           ///< 语言扩展名映射
};

#endif // FUNCTIONPARSER_H
