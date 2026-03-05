/**
 * @file projectinfo.h
 * @brief 项目信息数据模型定义
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef PROJECTINFO_H
#define PROJECTINFO_H

#include <QString>
#include <QDateTime>

/**
 * @brief 项目信息结构体
 */
struct ProjectInfo {
    int id;                             ///< 项目ID
    QString name;                       ///< 项目名称
    QString rootPath;                   ///< 项目根目录路径
    QString description;                ///< 项目描述
    QDateTime createTime;               ///< 创建时间
    QDateTime updateTime;               ///< 更新时间
    
    /**
     * @brief 默认构造函数
     */
    ProjectInfo()
        : id(0)
        , createTime(QDateTime::currentDateTime())
        , updateTime(QDateTime::currentDateTime())
    {}
    
    /**
     * @brief 带参数构造函数
     * @param name 项目名称
     * @param rootPath 项目根目录路径
     */
    ProjectInfo(const QString& name, const QString& rootPath)
        : id(0)
        , name(name)
        , rootPath(rootPath)
        , createTime(QDateTime::currentDateTime())
        , updateTime(QDateTime::currentDateTime())
    {}
};

#endif
