/**
 * @file logger.h
 * @brief 日志系统模块，用于记录程序运行日志
 * @author Developer
 * @date 2026-02-27
 * @version 1.0
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QMutexLocker>

/**
 * @brief 日志级别枚举
 */
enum LogLevel {
    Debug,   ///< 调试信息
    Info,    ///< 一般信息
    Warning, ///< 警告信息
    Error    ///< 错误信息
};

/**
 * @brief 日志系统类，提供日志记录功能
 */
class Logger {
public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger单例引用
     */
    static Logger& instance();

    /**
     * @brief 初始化日志系统
     * @param logFilePath 日志文件路径
     */
    void init(const QString& logFilePath);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, const QString& message);

    /**
     * @brief 记录调试信息
     * @param message 日志消息
     */
    void debug(const QString& message);

    /**
     * @brief 记录一般信息
     * @param message 日志消息
     */
    void info(const QString& message);

    /**
     * @brief 记录警告信息
     * @param message 日志消息
     */
    void warning(const QString& message);

    /**
     * @brief 记录错误信息
     * @param message 日志消息
     */
    void error(const QString& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 将日志级别转换为字符串
     * @param level 日志级别
     * @return 日志级别字符串
     */
    QString levelToString(LogLevel level);

    QFile* m_logFile;         ///< 日志文件指针
    QTextStream* m_logStream; ///< 日志文件流
    QMutex m_mutex;           ///< 互斥锁，保证线程安全
    bool m_initialized;       ///< 初始化标志
};

#endif // LOGGER_H
