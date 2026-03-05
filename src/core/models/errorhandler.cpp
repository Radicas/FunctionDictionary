/**
 * @file errorhandler.cpp
 * @brief 错误处理器实现
 * @author Developer
 * @date 2026-03-03
 * @version 1.0
 */

#include "core/models/errorhandler.h"

ErrorHandler& ErrorHandler::instance() {
    static ErrorHandler handler;
    return handler;
}

ErrorHandler::ErrorHandler(QObject* parent) : QObject(parent) {
    initErrorActions();
}

void ErrorHandler::initErrorActions() {
    QMutexLocker locker(&m_mutex);
    
    m_errorActions[ProcessErrorType::None] = ErrorAction::SaveAndContinue;
    m_errorActions[ProcessErrorType::FileNotFound] = ErrorAction::Skip;
    m_errorActions[ProcessErrorType::FileReadError] = ErrorAction::Retry;
    m_errorActions[ProcessErrorType::ParseError] = ErrorAction::Skip;
    m_errorActions[ProcessErrorType::AIConfigError] = ErrorAction::Abort;
    m_errorActions[ProcessErrorType::AIRequestError] = ErrorAction::Retry;
    m_errorActions[ProcessErrorType::AIRateLimitError] = ErrorAction::Pause;
    m_errorActions[ProcessErrorType::AITimeoutError] = ErrorAction::Retry;
    m_errorActions[ProcessErrorType::AIResponseError] = ErrorAction::Retry;
    m_errorActions[ProcessErrorType::DatabaseError] = ErrorAction::SaveAndContinue;
    m_errorActions[ProcessErrorType::DuplicateError] = ErrorAction::Skip;
    m_errorActions[ProcessErrorType::UnknownError] = ErrorAction::Skip;
}

ErrorAction ErrorHandler::handleError(const ProcessError& error) {
    QMutexLocker locker(&m_mutex);
    
    Logger::instance().error(QString("发生错误: %1, 类型: %2, 函数: %3, 重试次数: %4")
        .arg(error.message)
        .arg(errorTypeToString(error.type))
        .arg(error.functionName)
        .arg(error.retryCount));
    
    emit errorOccurred(error);
    
    ErrorAction action = m_errorActions.value(error.type, ErrorAction::Skip);
    
    if (action == ErrorAction::Retry && error.retryCount >= 3) {
        Logger::instance().warning(QString("已达到最大重试次数，跳过: %1").arg(error.functionName));
        action = ErrorAction::Skip;
    }
    
    return action;
}

int ErrorHandler::getRetryDelay(int retryCount) const {
    int delay = 1000 * (1 << retryCount);
    return qMin(delay, 60000);
}

QString ErrorHandler::errorTypeToString(ProcessErrorType type) {
    switch (type) {
        case ProcessErrorType::None:
            return "无错误";
        case ProcessErrorType::FileNotFound:
            return "文件不存在";
        case ProcessErrorType::FileReadError:
            return "文件读取错误";
        case ProcessErrorType::ParseError:
            return "解析错误";
        case ProcessErrorType::AIConfigError:
            return "AI配置错误";
        case ProcessErrorType::AIRequestError:
            return "AI请求错误";
        case ProcessErrorType::AIRateLimitError:
            return "AI速率限制错误";
        case ProcessErrorType::AITimeoutError:
            return "AI超时错误";
        case ProcessErrorType::AIResponseError:
            return "AI响应错误";
        case ProcessErrorType::DatabaseError:
            return "数据库错误";
        case ProcessErrorType::DuplicateError:
            return "重复错误";
        case ProcessErrorType::UnknownError:
        default:
            return "未知错误";
    }
}
