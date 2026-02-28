#include "logger.h"
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() : m_logFile(nullptr), m_logStream(nullptr), m_initialized(false) {
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    int oldMode = _setmode(_fileno(stdout), _O_U8TEXT);
    if (oldMode == -1) {
        oldMode = _setmode(_fileno(stdout), _O_TEXT);
    }
    
    oldMode = _setmode(_fileno(stderr), _O_U8TEXT);
    if (oldMode == -1) {
        oldMode = _setmode(_fileno(stderr), _O_TEXT);
    }
#endif
}

Logger::~Logger() {
    QMutexLocker locker(&m_mutex);
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void Logger::init(const QString& logFilePath) {
    QMutexLocker locker(&m_mutex);
    if (m_initialized) {
        return;
    }

    QFileInfo fileInfo(logFilePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_logFile = new QFile(logFilePath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "无法打开日志文件:" << logFilePath;
        delete m_logFile;
        m_logFile = nullptr;
        return;
    }

    m_logStream = new QTextStream(m_logFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_logStream->setCodec("UTF-8");
#endif
    m_initialized = true;

    info("日志系统初始化成功");
}

void Logger::log(LogLevel level, const QString& message) {
    QMutexLocker locker(&m_mutex);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    if (m_initialized && m_logStream) {
        *m_logStream << logMessage << "\n";
        m_logStream->flush();
    }

    switch (level) {
        case Debug:
            qDebug() << logMessage;
            break;
        case Info:
            qInfo() << logMessage;
            break;
        case Warning:
            qWarning() << logMessage;
            break;
        case Error:
            qCritical() << logMessage;
            break;
    }
}

void Logger::debug(const QString& message) {
    log(Debug, message);
}

void Logger::info(const QString& message) {
    log(Info, message);
}

void Logger::warning(const QString& message) {
    log(Warning, message);
}

void Logger::error(const QString& message) {
    log(Error, message);
}

QString Logger::levelToString(LogLevel level) {
    switch (level) {
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Warning:
            return "WARNING";
        case Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
