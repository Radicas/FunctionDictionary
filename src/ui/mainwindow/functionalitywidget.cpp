/**
 * @file functionalitywidget.cpp
 * @brief 功能型widget组件实现（协调者）
 * @author FunctionDB Team
 * @date 2026-03-05
 * @version 3.0
 */

#include "ui/mainwindow/functionalitywidget.h"
#include <QFontMetrics>
#include "common/logger/logger.h"
#include "core/ai/aiconfigmanager.h"

FunctionalityWidget::FunctionalityWidget(IDatabaseManager* dbManager, IParseService* parseService, QWidget* parent)
    : QWidget(parent), m_dbManager(dbManager), m_parseService(parseService)
{
    setupUI();
    connectSignals();
    loadProjects();
    Logger::instance().info("功能型widget初始化完成（协调者模式）");
}

FunctionalityWidget::~FunctionalityWidget() {}

void FunctionalityWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    // 使用字体度量作为基准单位
    QFontMetrics fm(font());
    int baseUnit = fm.height();
    int spacing = qMax(8, baseUnit / 2);
    int margin = qMax(6, baseUnit / 2);
    m_mainLayout->setSpacing(spacing);
    m_mainLayout->setContentsMargins(margin, margin, margin, margin);

    // 响应式最小尺寸
    setMinimumWidth(qMax(200, baseUnit * 15));
    setMinimumHeight(qMax(300, baseUnit * 25));

    m_fileSelector = new FileSelectorWidget(this);
    m_mainLayout->addWidget(m_fileSelector);

    QHBoxLayout* projectLayout = new QHBoxLayout();
    m_projectLabel = new QLabel("目标项目:", this);
    m_projectCombo = new QComboBox(this);
    m_projectCombo->setMinimumWidth(200);
    projectLayout->addWidget(m_projectLabel);
    projectLayout->addWidget(m_projectCombo);
    projectLayout->addStretch();
    m_mainLayout->addLayout(projectLayout);

    m_progressWidget = new ProgressWidget(this);
    m_mainLayout->addWidget(m_progressWidget);

    m_controlWidget = new ParseControlWidget(this);
    m_mainLayout->addWidget(m_controlWidget);

    setLayout(m_mainLayout);
}

void FunctionalityWidget::loadProjects()
{
    m_projectCombo->clear();

    QVector<ProjectInfo> projects = m_dbManager->getAllProjects();

    m_projectCombo->addItem("待整理", -1);

    for (const ProjectInfo& project : projects)
    {
        if (project.rootPath != "__temporary__")
        {
            m_projectCombo->addItem(project.name, project.id);
        }
    }

    if (m_projectCombo->count() > 0)
    {
        m_projectCombo->setCurrentIndex(0);
    }

    Logger::instance().info(QString("已加载 %1 个项目").arg(projects.size()));
}

void FunctionalityWidget::refreshProjectList()
{
    loadProjects();
    Logger::instance().info("项目列表已刷新");
}

void FunctionalityWidget::connectSignals()
{
    connect(m_fileSelector, &FileSelectorWidget::pathSelected, this, &FunctionalityWidget::onPathSelected);
    connect(m_fileSelector, &FileSelectorWidget::modeChanged, this, &FunctionalityWidget::onModeChanged);

    connect(m_controlWidget, &ParseControlWidget::parseRequested, this, &FunctionalityWidget::onParseRequested);
    connect(m_controlWidget, &ParseControlWidget::cancelRequested, this, &FunctionalityWidget::onCancelRequested);
    connect(m_controlWidget, &ParseControlWidget::aiConfigRequested, this, &FunctionalityWidget::onAiConfigRequested);

    if (m_parseService)
    {
        connect(m_parseService, &IParseService::parseComplete, this, &FunctionalityWidget::onParseComplete);
        connect(m_parseService, &IParseService::parseProgress, this, &FunctionalityWidget::onParseProgress);
        connect(m_parseService, &IParseService::parseFailed, this, &FunctionalityWidget::onParseFailed);
        connect(m_parseService, &IParseService::parseCancelled, this, &FunctionalityWidget::onParseCancelled);
    }
}

void FunctionalityWidget::onPathSelected(const QString& path)
{
    Logger::instance().info("路径已选择: " + path);
}

void FunctionalityWidget::onModeChanged(ParseMode mode)
{
    m_controlWidget->setRecursiveEnabled(mode == ParseMode::Folder);
    Logger::instance().info(QString("模式已切换: %1").arg(mode == ParseMode::SingleFile ? "单文件" : "文件夹"));
}

void FunctionalityWidget::onParseRequested()
{
    QString path = m_fileSelector->selectedPath();

    if (path.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请先选择要解析的文件或文件夹！");
        return;
    }

    if (!validateAIConfig())
    {
        return;
    }

    startParsing();
}

void FunctionalityWidget::onCancelRequested()
{
    if (m_parseService)
    {
        m_parseService->cancelParsing();
    }
    m_progressWidget->setStatusMessage("正在取消解析...");
}

void FunctionalityWidget::onAiConfigRequested()
{
    AIConfigDialog dialog(this);
    dialog.exec();
    Logger::instance().info("用户打开AI配置对话框");
}

void FunctionalityWidget::onParseComplete(const ParseResult& result)
{
    updateUIState(false);
    m_progressWidget->setWaitingAnimation(false);

    QString message = QString("解析完成！成功: %1, 失败: %2, 跳过: %3")
                          .arg(result.successCount)
                          .arg(result.failedCount)
                          .arg(result.skippedCount);

    m_progressWidget->setStatusMessage(message);
    m_progressWidget->appendLog(message);

    QMessageBox::information(this, "完成", message);
    Logger::instance().info(message);

    emit batchProcessingCompleted();
}

void FunctionalityWidget::onParseProgress(const ParseProgress& progress)
{
    m_progressWidget->setProgress(progress.current, progress.total);
    m_progressWidget->setStatistics(progress.successCount, progress.failedCount, progress.skippedCount);

    QString logMsg = QString("[%1] %2").arg(progress.stage).arg(progress.message);
    m_progressWidget->setStatusMessage(progress.message);
    m_progressWidget->appendLog(logMsg);
}

void FunctionalityWidget::onParseFailed(const QString& error)
{
    updateUIState(false);
    m_progressWidget->setWaitingAnimation(false);

    QMessageBox::critical(this, "错误", "解析失败：" + error);
    Logger::instance().error("解析失败: " + error);
    m_progressWidget->appendLog(QString("[错误] %1").arg(error));
}

void FunctionalityWidget::onParseCancelled()
{
    updateUIState(false);
    m_progressWidget->setWaitingAnimation(false);

    m_progressWidget->setStatusMessage("已取消解析");
    m_progressWidget->appendLog("[取消] 解析已取消");
    Logger::instance().info("解析已取消");
}

void FunctionalityWidget::startParsing()
{
    updateUIState(true);
    m_progressWidget->reset();
    m_progressWidget->setVisible(true);
    m_progressWidget->setWaitingAnimation(true);

    QString path = m_fileSelector->selectedPath();
    ParseMode mode = m_fileSelector->currentMode();
    bool skipExisting = m_controlWidget->skipExisting();

    int projectId = m_projectCombo->currentData().toInt();

    if (m_parseService)
    {
        m_parseService->setSkipExisting(skipExisting);
        m_parseService->setTargetProject(projectId);

        if (mode == ParseMode::SingleFile)
        {
            m_progressWidget->appendLog("开始解析文件...");
            Logger::instance().info("开始解析文件: " + path);
            m_parseService->parseFile(path);
        }
        else
        {
            bool recursive = m_controlWidget->isRecursive();
            m_progressWidget->appendLog(QString("开始批量解析文件夹%1...").arg(recursive ? "（递归）" : ""));
            Logger::instance().info(
                QString("开始批量解析文件夹: %1, 递归: %2, 项目ID: %3").arg(path).arg(recursive).arg(projectId));
            m_parseService->parseFolder(path, recursive);
        }
    }
}

void FunctionalityWidget::updateUIState(bool isProcessing)
{
    m_fileSelector->setEnabled(!isProcessing);
    m_controlWidget->setParsing(isProcessing);
}

bool FunctionalityWidget::validateAIConfig()
{
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    if (!AIConfigManager::instance().isConfigValid(config))
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("AI配置缺失");
        msgBox.setText("AI配置不完整，请先配置AI服务！");
        msgBox.setInformativeText("是否立即配置AI服务？");
        QPushButton* configBtn = msgBox.addButton("立即配置", QMessageBox::ActionRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);

        msgBox.exec();

        if (msgBox.clickedButton() == configBtn)
        {
            AIConfigDialog dialog(this);
            dialog.exec();
            Logger::instance().info("用户打开AI配置对话框");
        }
        return false;
    }
    return true;
}
