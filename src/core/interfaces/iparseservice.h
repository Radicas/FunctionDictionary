/**
 * @file iparseservice.h
 * @brief 解析服务接口定义
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef IPARSESERVICE_H
#define IPARSESERVICE_H

#include <QObject>
#include <QString>
#include "core/models/parseresult.h"

/**
 * @brief 解析服务接口类
 * 
 * 该接口定义了解析服务的标准操作，用于解耦UI层与具体实现。
 */
class IParseService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit IParseService(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief 析构函数
     */
    virtual ~IParseService() = default;

    /**
     * @brief 解析单个文件
     * @param filePath 文件路径
     */
    virtual void parseFile(const QString& filePath) = 0;

    /**
     * @brief 解析文件夹
     * @param folderPath 文件夹路径
     * @param recursive 是否递归扫描
     */
    virtual void parseFolder(const QString& folderPath, bool recursive) = 0;

    /**
     * @brief 取消解析
     */
    virtual void cancelParsing() = 0;

    /**
     * @brief 检查是否正在解析
     * @return 是否正在解析
     */
    virtual bool isParsing() const = 0;

    /**
     * @brief 设置是否跳过已存在的函数
     * @param skip 是否跳过
     */
    virtual void setSkipExisting(bool skip) = 0;

signals:
    /**
     * @brief 解析完成信号
     * @param result 解析结果
     */
    void parseComplete(const ParseResult& result);

    /**
     * @brief 解析进度信号
     * @param progress 进度信息
     */
    void parseProgress(const ParseProgress& progress);

    /**
     * @brief 解析失败信号
     * @param error 错误信息
     */
    void parseFailed(const QString& error);

    /**
     * @brief 解析取消完成信号
     */
    void parseCancelled();
};

#endif // IPARSESERVICE_H
