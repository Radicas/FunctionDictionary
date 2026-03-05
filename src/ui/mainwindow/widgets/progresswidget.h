/**
 * @file progresswidget.h
 * @brief 进度显示组件，负责解析进度的可视化展示
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QElapsedTimer>

/**
 * @brief 进度显示组件类
 * 
 * 该组件负责：
 * - 进度条显示
 * - 状态消息显示
 * - 统计信息显示（成功/失败/跳过）
 * - 日志视图显示
 * - 等待动画
 */
class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit ProgressWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ProgressWidget();

    /**
     * @brief 设置进度
     * @param current 当前进度
     * @param total 总数
     */
    void setProgress(int current, int total);

    /**
     * @brief 设置状态消息
     * @param message 状态消息
     * @param duration 显示时长（毫秒），0表示持续显示
     */
    void setStatusMessage(const QString& message, int duration = 0);

    /**
     * @brief 设置统计信息
     * @param success 成功数量
     * @param failed 失败数量
     * @param skipped 跳过数量
     */
    void setStatistics(int success, int failed, int skipped);

    /**
     * @brief 追加日志消息
     * @param message 日志消息
     */
    void appendLog(const QString& message);

    /**
     * @brief 清除日志
     */
    void clearLog();

    /**
     * @brief 设置等待动画
     * @param enabled 是否启用
     */
    void setWaitingAnimation(bool enabled);

    /**
     * @brief 重置所有显示
     */
    void reset();

    /**
     * @brief 设置可见性
     * @param visible 是否可见
     */
    void setVisible(bool visible) override;

private slots:
    /**
     * @brief 更新等待动画
     */
    void updateWaitingAnimation();

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();

    QProgressBar *m_progressBar;       ///< 进度条
    QLabel *m_statusLabel;             ///< 状态标签
    QLabel *m_successLabel;            ///< 成功计数标签
    QLabel *m_failedLabel;             ///< 失败计数标签
    QLabel *m_skippedLabel;            ///< 跳过计数标签
    QTextEdit *m_logView;              ///< 日志视图
    
    QTimer *m_waitingTimer;            ///< 等待动画定时器
    QElapsedTimer m_elapsedTimer;      ///< 计时器
    int m_waitingDots;                 ///< 等待动画点数
    bool m_waitingAnimationEnabled;    ///< 是否启用等待动画
};

#endif // PROGRESSWIDGET_H
