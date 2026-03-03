/**
 * @file functiondata.h
 * @brief 函数数据模型定义
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef FUNCTIONDATA_H
#define FUNCTIONDATA_H

#include <QString>
#include <QDateTime>

/**
 * @brief 函数数据结构
 */
struct FunctionData {
    int id;                             ///< 函数ID
    QString key;                        ///< 函数名称（唯一标识）
    QString value;                      ///< 函数介绍（Markdown格式）
    QDateTime createTime;               ///< 创建时间
    
    QString signature;                  ///< 函数签名
    QString returnType;                 ///< 返回类型
    QString parameters;                 ///< 参数列表（JSON格式）
    QString filePath;                   ///< 源文件路径
    int startLine;                      ///< 起始行号
    int endLine;                        ///< 结束行号
    QString language;                   ///< 编程语言
    QString flowchart;                  ///< 流程图
    QString sequenceDiagram;            ///< 时序图
    QString structureDiagram;           ///< 结构图
    QString aiModel;                    ///< 分析使用的AI模型
    QDateTime analyzeTime;              ///< 分析时间
    
    /**
     * @brief 默认构造函数
     */
    FunctionData()
        : id(0)
        , startLine(0)
        , endLine(0)
        , createTime(QDateTime::currentDateTime())
        , analyzeTime(QDateTime::currentDateTime())
    {}
};

#endif
