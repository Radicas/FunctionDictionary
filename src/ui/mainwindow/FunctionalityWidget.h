/**
 * @file FunctionalityWidget.h
 * @brief 功能型widget组件，集成设置、配置加载及文件解析等功能
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef FUNCTIONALITYWIDGET_H
#define FUNCTIONALITYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QTextEdit>
#include "core/ai/aiservicemanager.h"
#include "core/database/databasemanager.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "core/parser/functionparser.h"
#include "core/batch/batchprocessmanager.h"
#include "core/models/extractedfunction.h"

class FunctionalityWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionalityWidget(QWidget *parent = nullptr);
    ~FunctionalityWidget();

signals:
    void batchProcessingCompleted();

private slots:
    /**
     * @brief 文件选择按钮点击槽函数
     */
    void onFileSelectClicked();

    /**
     * @brief 解析按钮点击槽函数
     */
    void onParseButtonClicked();

    /**
     * @brief 暂停/恢复按钮点击槽函数
     */
    void onPauseResumeClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelClicked();

    /**
     * @brief 解析完成槽函数，用于恢复界面状态
     */
    void onParseFinished();

    /**
     * @brief AI分析完成槽函数
     * @param functionName 函数名称
     * @param functionDescription 函数描述
     */
    void onAnalysisComplete(const QString& functionName, const QString& functionDescription);

    /**
     * @brief AI分析失败槽函数
     * @param error 错误信息
     */
    void onAnalysisFailed(const QString& error);

    /**
     * @brief AI分析进度槽函数
     * @param message 进度消息
     */
    void onAnalysisProgress(const QString& message);

    /**
     * @brief 函数提取进度槽函数
     * @param current 当前进度
     * @param total 总数
     * @param message 进度消息
     */
    void onExtractionProgress(int current, int total, const QString& message);

    /**
     * @brief 批量处理进度槽函数
     * @param current 当前处理的函数索引
     * @param total 总函数数量
     * @param functionName 当前处理的函数名
     */
    void onBatchProgress(int current, int total, const QString& functionName);

    /**
     * @brief 单个函数处理完成槽函数
     * @param func 函数信息
     * @param success 是否成功
     * @param message 结果消息
     */
    void onFunctionProcessed(const ExtractedFunction& func, bool success, const QString& message);

    /**
     * @brief 批量处理完成槽函数
     * @param successCount 成功数量
     * @param failedCount 失败数量
     * @param skippedCount 跳过数量
     */
    void onBatchCompleted(int successCount, int failedCount, int skippedCount);

    /**
     * @brief 批量处理状态变化槽函数
     * @param newState 新状态
     */
    void onStateChanged(BatchProcessState newState);

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();

    /**
     * @brief 设置文件解析区域
     */
    void setupFileParseSection();

    /**
     * @brief 设置配置区域
     */
    void setupSettingsSection();

    /**
     * @brief 验证文件是否有效
     * @param filePath 文件路径
     * @return 文件是否有效
     */
    bool validateFile(const QString &filePath);

    /**
     * @brief 显示状态消息
     * @param message 消息内容
     * @param duration 显示时长（毫秒），0表示持续显示
     */
    void showStatusMessage(const QString &message, int duration = 3000);

    /**
     * @brief 开始解析和分析文件
     */
    void startFileAnalysis();

    /**
     * @brief 更新UI状态
     * @param isProcessing 是否正在处理
     */
    void updateUIState(bool isProcessing);

    // 文件解析相关
    QLabel *m_parseTitle;              ///< 解析标题标签
    QLabel *m_fileInfoLabel;           ///< 文件信息标签
    QLabel *m_functionCountLabel;      ///< 函数数量标签
    QLineEdit *m_filePathEdit;         ///< 文件路径输入框
    QPushButton *m_fileSelectButton;   ///< 文件选择按钮
    QPushButton *m_parseButton;        ///< 解析按钮
    QPushButton *m_pauseResumeButton;  ///< 暂停/恢复按钮
    QPushButton *m_cancelButton;       ///< 取消按钮
    QProgressBar *m_progressBar;       ///< 进度条
    QLabel *m_statusLabel;             ///< 状态标签
    QTextEdit *m_logView;              ///< 日志视图

    // 统计信息
    QLabel *m_successCountLabel;       ///< 成功计数
    QLabel *m_failedCountLabel;        ///< 失败计数
    QLabel *m_skippedCountLabel;       ///< 跳过计数

    // 配置相关
    QLabel *m_settingsTitle;           ///< 配置标题标签
    QPushButton *m_aiConfigButton;     ///< AI配置按钮

    QVBoxLayout *m_mainLayout;         ///< 主布局
    QString m_currentFilePath;         ///< 当前选择的文件路径
    ExtractionResult m_extractionResult; ///< 提取结果
};

#endif // FUNCTIONALITYWIDGET_H