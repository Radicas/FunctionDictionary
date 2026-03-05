/**
 * @file functionalitywidget.h
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
#include <QComboBox>
#include <QCheckBox>
#include <QElapsedTimer>
#include "core/ai/aiservicemanager.h"
#include "core/database/databasemanager.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "core/parser/aicodeparser.h"
#include "core/parser/batchcodeparser.h"

/**
 * @brief 解析模式枚举
 */
enum class ParseMode {
    SingleFile,     ///< 单文件模式
    Folder          ///< 文件夹模式
};

class FunctionalityWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionalityWidget(QWidget *parent = nullptr);
    ~FunctionalityWidget();

signals:
    void batchProcessingCompleted();

private slots:
    /**
     * @brief 文件/文件夹选择按钮点击槽函数
     */
    void onSelectClicked();

    /**
     * @brief 解析按钮点击槽函数
     */
    void onParseButtonClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelClicked();

    /**
     * @brief 解析模式改变槽函数
     * @param index 模式索引
     */
    void onModeChanged(int index);

    /**
     * @brief 解析完成槽函数
     */
    void onParseFinished();

    /**
     * @brief AI代码解析完成槽函数（单文件）
     * @param result 解析结果
     */
    void onAIParseComplete(const AIParseResult& result);

    /**
     * @brief AI代码解析失败槽函数（单文件）
     * @param error 错误信息
     */
    void onAIParseFailed(const QString& error);

    /**
     * @brief AI代码解析进度槽函数（单文件）
     * @param stage 当前阶段
     * @param message 进度消息
     */
    void onAIParseProgress(const QString& stage, const QString& message);

    /**
     * @brief 批量解析进度槽函数
     * @param progress 进度信息
     */
    void onBatchProgress(const BatchParseProgress& progress);

    /**
     * @brief 单个文件解析完成槽函数（批量模式）
     * @param filePath 文件路径
     * @param result 解析结果
     */
    void onFileParsed(const QString& filePath, const AIParseResult& result);

    /**
     * @brief 批量解析完成槽函数
     * @param result 批量解析结果
     */
    void onBatchComplete(const BatchParseResult& result);

    /**
     * @brief 批量解析失败槽函数
     * @param error 错误信息
     */
    void onBatchFailed(const QString& error);

    /**
     * @brief AI解析取消完成槽函数
     */
    void onAIParseCancelled();

    /**
     * @brief 批量解析取消完成槽函数
     */
    void onBatchCancelled();

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
     * @brief 验证路径是否有效
     * @param path 路径
     * @return 路径是否有效
     */
    bool validatePath(const QString &path);

    /**
     * @brief 显示状态消息
     * @param message 消息内容
     * @param duration 显示时长（毫秒），0表示持续显示
     */
    void showStatusMessage(const QString &message, int duration = 3000);

    /**
     * @brief 开始解析
     */
    void startParsing();

    /**
     * @brief 更新UI状态
     * @param isProcessing 是否正在处理
     */
    void updateUIState(bool isProcessing);

    /**
     * @brief 更新等待动画
     */
    void updateWaitingAnimation();

    QLabel *m_modeLabel;               ///< 模式标签
    QComboBox *m_modeComboBox;         ///< 模式选择下拉框
    QLabel *m_pathLabel;               ///< 路径标签
    QLabel *m_fileInfoLabel;           ///< 文件信息标签
    QLabel *m_functionCountLabel;      ///< 函数数量标签
    QLineEdit *m_pathEdit;             ///< 路径输入框
    QPushButton *m_selectButton;       ///< 选择按钮
    QPushButton *m_parseButton;        ///< 解析按钮
    QPushButton *m_cancelButton;       ///< 取消按钮
    QProgressBar *m_progressBar;       ///< 进度条
    QLabel *m_statusLabel;             ///< 状态标签
    QTextEdit *m_logView;              ///< 日志视图

    QLabel *m_successCountLabel;       ///< 成功计数
    QLabel *m_failedCountLabel;        ///< 失败计数
    QLabel *m_skippedCountLabel;       ///< 跳过计数

    QLabel *m_settingsTitle;           ///< 配置标题标签
    QPushButton *m_aiConfigButton;     ///< AI配置按钮
    QCheckBox *m_recursiveCheckBox;    ///< 递归扫描复选框
    QCheckBox *m_skipExistingCheckBox; ///< 跳过已存在复选框

    QVBoxLayout *m_mainLayout;         ///< 主布局
    QString m_currentPath;             ///< 当前选择的路径
    ParseMode m_currentMode;           ///< 当前解析模式
    int m_successCount;                ///< 成功计数
    int m_failedCount;                 ///< 失败计数
    int m_skippedCount;                ///< 跳过计数
    
    QTimer *m_waitingTimer;            ///< 等待动画定时器
    int m_waitingDots;                 ///< 等待动画点数
    QElapsedTimer m_elapsedTimer;      ///< 计时器
};

#endif // FUNCTIONALITYWIDGET_H
