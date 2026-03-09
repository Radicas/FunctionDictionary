/**
 * @file parseresult.h
 * @brief 解析结果数据结构定义
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include "core/models/functiondata.h"

/**
 * @brief 解析进度信息结构
 */
struct ParseProgress
{
    int current;       ///< 当前进度
    int total;         ///< 总数
    QString stage;     ///< 当前阶段
    QString message;   ///< 进度消息
    int successCount;  ///< 成功数量
    int failedCount;   ///< 失败数量
    int skippedCount;  ///< 跳过数量

    /**
     * @brief 默认构造函数
     */
    ParseProgress() : current(0), total(0), successCount(0), failedCount(0), skippedCount(0) {}
};

/**
 * @brief 解析结果结构
 */
struct ParseResult
{
    bool success;                     ///< 是否成功
    QString filePath;                 ///< 文件路径
    int successCount;                 ///< 成功数量
    int failedCount;                  ///< 失败数量
    int skippedCount;                 ///< 跳过数量
    QVector<FunctionData> functions;  ///< 提取的函数列表
    QString errorMessage;             ///< 错误信息
    QDateTime timestamp;              ///< 时间戳

    /**
     * @brief 默认构造函数
     */
    ParseResult()
        : success(false), successCount(0), failedCount(0), skippedCount(0), timestamp(QDateTime::currentDateTime())
    {
    }
};

#endif  // PARSERESULT_H
