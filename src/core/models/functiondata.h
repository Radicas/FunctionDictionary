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
    int id;                 ///< 函数ID
    QString key;            ///< 函数名称（唯一标识）
    QString value;          ///< 函数介绍（Markdown格式）
    QDateTime createTime;   ///< 创建时间
};

#endif
