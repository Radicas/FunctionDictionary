/**
 * @file extractedfunction.h
 * @brief 提取的函数数据模型定义
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#ifndef EXTRACTEDFUNCTION_H
#define EXTRACTEDFUNCTION_H

#include <QString>
#include <QVector>

/**
 * @brief 参数信息结构
 */
struct ParameterInfo {
    QString name;        ///< 参数名称
    QString type;        ///< 参数类型
    QString defaultValue; ///< 默认值（如有）
};

/**
 * @brief 提取的函数信息结构
 */
struct ExtractedFunction {
    QString name;                          ///< 函数名称
    QString signature;                     ///< 函数签名
    QString body;                          ///< 函数体代码
    QString returnType;                    ///< 返回类型
    QVector<ParameterInfo> parameters;     ///< 参数列表
    int startLine;                         ///< 起始行号
    int endLine;                           ///< 结束行号
    QString filePath;                      ///< 所属文件路径
    QString language;                      ///< 编程语言类型

    /**
     * @brief 默认构造函数
     */
    ExtractedFunction()
        : startLine(0)
        , endLine(0)
    {}
};

/**
 * @brief 函数提取结果
 */
struct ExtractionResult {
    bool success;                              ///< 是否成功
    QString filePath;                          ///< 文件路径
    QString language;                          ///< 语言类型
    QVector<ExtractedFunction> functions;      ///< 提取的函数列表
    QString errorMessage;                      ///< 错误信息
    int totalLines;                            ///< 文件总行数

    /**
     * @brief 默认构造函数
     */
    ExtractionResult()
        : success(false)
        , totalLines(0)
    {}
};

#endif // EXTRACTEDFUNCTION_H
