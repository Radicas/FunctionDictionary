/**
 * @file progresswidget.cpp
 * @brief 进度显示组件实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "ui/mainwindow/widgets/progresswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent), m_waitingTimer(nullptr), m_waitingDots(0), m_waitingAnimationEnabled(false)
{
    setupUI();
    
    m_waitingTimer = new QTimer(this);
    connect(m_waitingTimer, &QTimer::timeout, this, &ProgressWidget::updateWaitingAnimation);
}

ProgressWidget::~ProgressWidget()
{
}

void ProgressWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    m_successLabel = new QLabel("成功: 0", this);
    m_failedLabel = new QLabel("失败: 0", this);
    m_skippedLabel = new QLabel("跳过: 0", this);
    statsLayout->addWidget(m_successLabel);
    statsLayout->addWidget(m_failedLabel);
    statsLayout->addWidget(m_skippedLabel);
    mainLayout->addLayout(statsLayout);

    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("statusLabel");
    mainLayout->addWidget(m_statusLabel);

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(150);
    m_logView->setVisible(false);
    mainLayout->addWidget(m_logView);

    setLayout(mainLayout);
}

void ProgressWidget::setProgress(int current, int total)
{
    if (total > 0) {
        int progressValue = (current * 100) / total;
        m_progressBar->setValue(progressValue);
    }
}

void ProgressWidget::setStatusMessage(const QString& message, int duration)
{
    m_statusLabel->setText(message);
    if (duration > 0) {
        QTimer::singleShot(duration, [this]() {
            m_statusLabel->setText("就绪");
        });
    }
}

void ProgressWidget::setStatistics(int success, int failed, int skipped)
{
    m_successLabel->setText(QString("成功: %1").arg(success));
    m_failedLabel->setText(QString("失败: %1").arg(failed));
    m_skippedLabel->setText(QString("跳过: %1").arg(skipped));
}

void ProgressWidget::appendLog(const QString& message)
{
    m_logView->append(message);
}

void ProgressWidget::clearLog()
{
    m_logView->clear();
}

void ProgressWidget::setWaitingAnimation(bool enabled)
{
    m_waitingAnimationEnabled = enabled;
    
    if (enabled) {
        m_waitingDots = 0;
        m_elapsedTimer.start();
        m_waitingTimer->start(500);
    } else {
        m_waitingTimer->stop();
    }
}

void ProgressWidget::reset()
{
    m_progressBar->setValue(0);
    m_statusLabel->setText("就绪");
    m_successLabel->setText("成功: 0");
    m_failedLabel->setText("失败: 0");
    m_skippedLabel->setText("跳过: 0");
    m_logView->clear();
    setWaitingAnimation(false);
}

void ProgressWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    m_progressBar->setVisible(visible);
    m_logView->setVisible(visible);
}

void ProgressWidget::updateWaitingAnimation()
{
    if (!m_waitingAnimationEnabled) {
        return;
    }
    
    m_waitingDots = (m_waitingDots % 3) + 1;
    QString dots = QString(".").repeated(m_waitingDots);
    
    qint64 elapsed = m_elapsedTimer.elapsed() / 1000;
    QString timeStr;
    if (elapsed < 60) {
        timeStr = QString("%1秒").arg(elapsed);
    } else {
        timeStr = QString("%1分%2秒").arg(elapsed / 60).arg(elapsed % 60);
    }
    
    QString message = QString("AI正在分析代码%1 已用时: %2").arg(dots).arg(timeStr);
    m_statusLabel->setText(message);
}
