/**
 * @file FunctionalityWidget.cpp
 * @brief 功能型widget组件实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "FunctionalityWidget.h"
#include "common/logger/logger.h"
#include <QThread>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

FunctionalityWidget::FunctionalityWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    
    AIServiceManager& aiService = AIServiceManager::instance();
    connect(&aiService, &AIServiceManager::analysisComplete, 
            this, &FunctionalityWidget::onAnalysisComplete);
    connect(&aiService, &AIServiceManager::analysisFailed, 
            this, &FunctionalityWidget::onAnalysisFailed);
    connect(&aiService, &AIServiceManager::analysisProgress, 
            this, &FunctionalityWidget::onAnalysisProgress);
    
    FunctionParser& parser = FunctionParser::instance();
    connect(&parser, &FunctionParser::extractionProgress, 
            this, &FunctionalityWidget::onExtractionProgress);
    
    BatchProcessManager& batchManager = BatchProcessManager::instance();
    connect(&batchManager, &BatchProcessManager::batchProgress, 
            this, &FunctionalityWidget::onBatchProgress);
    connect(&batchManager, &BatchProcessManager::functionProcessed, 
            this, &FunctionalityWidget::onFunctionProcessed);
    connect(&batchManager, &BatchProcessManager::batchCompleted, 
            this, &FunctionalityWidget::onBatchCompleted);
    connect(&batchManager, &BatchProcessManager::stateChanged, 
            this, &FunctionalityWidget::onStateChanged);
    
    Logger::instance().info("功能型widget初始化完成");
}

FunctionalityWidget::~FunctionalityWidget() {
}

void FunctionalityWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    setupFileParseSection();
    setupSettingsSection();

    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("statusLabel");
    m_mainLayout->addWidget(m_statusLabel);

    setLayout(m_mainLayout);
}

void FunctionalityWidget::setupFileParseSection() {
    m_parseTitle = new QLabel("解析文件", this);
    m_parseTitle->setObjectName("titleLabel");
    m_mainLayout->addWidget(m_parseTitle);

    m_fileInfoLabel = new QLabel("未选择文件", this);
    m_fileInfoLabel->setObjectName("fileInfoLabel");
    m_mainLayout->addWidget(m_fileInfoLabel);

    m_functionCountLabel = new QLabel("", this);
    m_functionCountLabel->setObjectName("functionCountLabel");
    m_mainLayout->addWidget(m_functionCountLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setPlaceholderText("请选择要解析的文件");
    m_filePathEdit->setReadOnly(true);
    fileLayout->addWidget(m_filePathEdit);

    m_fileSelectButton = new QPushButton("选择", this);
    m_fileSelectButton->setObjectName("fileSelectButton");
    connect(m_fileSelectButton, &QPushButton::clicked, this, &FunctionalityWidget::onFileSelectClicked);
    fileLayout->addWidget(m_fileSelectButton);

    m_mainLayout->addLayout(fileLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_parseButton = new QPushButton("开始解析", this);
    m_parseButton->setObjectName("parseButton");
    connect(m_parseButton, &QPushButton::clicked, this, &FunctionalityWidget::onParseButtonClicked);
    buttonLayout->addWidget(m_parseButton);

    m_pauseResumeButton = new QPushButton("暂停", this);
    m_pauseResumeButton->setObjectName("pauseResumeButton");
    m_pauseResumeButton->setVisible(false);
    connect(m_pauseResumeButton, &QPushButton::clicked, this, &FunctionalityWidget::onPauseResumeClicked);
    buttonLayout->addWidget(m_pauseResumeButton);

    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setVisible(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &FunctionalityWidget::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addLayout(buttonLayout);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_mainLayout->addWidget(m_progressBar);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    m_successCountLabel = new QLabel("成功: 0", this);
    m_failedCountLabel = new QLabel("失败: 0", this);
    m_skippedCountLabel = new QLabel("跳过: 0", this);
    statsLayout->addWidget(m_successCountLabel);
    statsLayout->addWidget(m_failedCountLabel);
    statsLayout->addWidget(m_skippedCountLabel);
    m_mainLayout->addLayout(statsLayout);

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(150);
    m_logView->setVisible(false);
    m_mainLayout->addWidget(m_logView);
}

void FunctionalityWidget::setupSettingsSection() {
    m_settingsTitle = new QLabel("配置设置", this);
    m_settingsTitle->setObjectName("titleLabel");
    m_mainLayout->addWidget(m_settingsTitle);

    m_aiConfigButton = new QPushButton("AI配置", this);
    m_aiConfigButton->setObjectName("aiConfigButton");
    connect(m_aiConfigButton, &QPushButton::clicked, [this]() {
        AIConfigDialog dialog(this);
        dialog.exec();
        Logger::instance().info("用户打开AI配置对话框");
    });
    m_mainLayout->addWidget(m_aiConfigButton);

    m_mainLayout->addStretch();
}

void FunctionalityWidget::onFileSelectClicked() {
    Logger::instance().info("onFileSelectClicked 方法被调用");
    Logger::instance().info("当前线程是否为主线程: " + QString(QThread::currentThread() == qApp->thread() ? "是" : "否"));
    Logger::instance().info("window() 返回值: " + QString(window() ? "有效" : "空"));
    
    QFileDialog dialog(window(), "选择文件", QDir::homePath(),
        "所有文件 (*.*);;C/C++文件 (*.c *.cpp *.h *.hpp);;Python文件 (*.py);;Java文件 (*.java);;JavaScript文件 (*.js *.ts)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    
    Logger::instance().info("准备显示文件选择对话框");
    
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedFiles = dialog.selectedFiles();
        if (!selectedFiles.isEmpty()) {
            QString filePath = selectedFiles.first();
            Logger::instance().info("用户选择文件: " + filePath);
            m_filePathEdit->setText(filePath);
            m_currentFilePath = filePath;
            
            QFileInfo fileInfo(filePath);
            m_fileInfoLabel->setText(QString("文件: %1, 大小: %2 KB")
                .arg(fileInfo.fileName())
                .arg(fileInfo.size() / 1024.0, 0, 'f', 2));
            
            showStatusMessage("已选择文件: " + fileInfo.fileName());
        }
    } else {
        Logger::instance().info("用户取消选择文件");
    }
}

void FunctionalityWidget::onParseButtonClicked() {
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要解析的文件！");
        return;
    }

    if (!validateFile(m_currentFilePath)) {
        QMessageBox::critical(this, "错误", "所选文件无效或无法访问！");
        return;
    }

    startFileAnalysis();
}

void FunctionalityWidget::onPauseResumeClicked() {
    BatchProcessManager& batchManager = BatchProcessManager::instance();
    if (batchManager.getState() == BatchProcessState::Running) {
        batchManager.pauseProcessing();
    } else if (batchManager.getState() == BatchProcessState::Paused) {
        batchManager.resumeProcessing();
    }
}

void FunctionalityWidget::onCancelClicked() {
    BatchProcessManager& batchManager = BatchProcessManager::instance();
    batchManager.cancelProcessing();
}

void FunctionalityWidget::onParseFinished() {
    updateUIState(false);
}

void FunctionalityWidget::onAnalysisComplete(const QString& functionName, const QString& functionDescription) {
    bool success = DatabaseManager::instance().addFunction(functionName, functionDescription);

    if (success) {
        showStatusMessage("文件解析成功并保存到数据库");
        Logger::instance().info("文件解析成功并保存到数据库: " + m_currentFilePath);
    } else {
        showStatusMessage("文件解析成功，但保存到数据库失败: " + DatabaseManager::instance().lastError());
        Logger::instance().error("保存解析结果到数据库失败: " + DatabaseManager::instance().lastError());
    }

    onParseFinished();
}

void FunctionalityWidget::onAnalysisFailed(const QString& error) {
    QMessageBox::critical(this, "错误", "AI 分析失败：" + error);
    Logger::instance().error("AI 分析失败: " + error);
    
    onParseFinished();
}

void FunctionalityWidget::onAnalysisProgress(const QString& message) {
    showStatusMessage(message);
    if (message.contains("发送请求")) {
        m_progressBar->setValue(30);
    } else if (message.contains("解析响应")) {
        m_progressBar->setValue(70);
    }
}

void FunctionalityWidget::onExtractionProgress(int current, int total, const QString& message) {
    showStatusMessage(message);
    m_logView->append(message);
    
    if (total > 0) {
        m_progressBar->setValue((current * 50) / total);
    }
}

void FunctionalityWidget::onBatchProgress(int current, int total, const QString& functionName) {
    m_progressBar->setValue(50 + (current * 50) / total);
    showStatusMessage(QString("正在分析: %1 (%2/%3)").arg(functionName).arg(current).arg(total));
}

void FunctionalityWidget::onFunctionProcessed(const ExtractedFunction& func, bool success, const QString& message) {
    QString logMsg = QString("[%1] %2: %3")
        .arg(success ? "成功" : "失败")
        .arg(func.name)
        .arg(message);
    m_logView->append(logMsg);
    
    if (success) {
        FunctionData data;
        data.key = func.name;
        data.value = "";
        data.signature = func.signature;
        data.returnType = func.returnType;
        data.filePath = func.filePath;
        data.startLine = func.startLine;
        data.endLine = func.endLine;
        data.language = func.language;
        DatabaseManager::instance().addFunction(data);
        
        m_successCountLabel->setText(QString("成功: %1").arg(BatchProcessManager::instance().getSuccessCount()));
    } else {
        m_failedCountLabel->setText(QString("失败: %1").arg(BatchProcessManager::instance().getFailedCount()));
    }
    
    m_skippedCountLabel->setText(QString("跳过: %1").arg(BatchProcessManager::instance().getSkippedCount()));
}

void FunctionalityWidget::onBatchCompleted(int successCount, int failedCount, int skippedCount) {
    QString message = QString("批量处理完成！成功: %1, 失败: %2, 跳过: %3")
        .arg(successCount).arg(failedCount).arg(skippedCount);
    QMessageBox::information(this, "完成", message);
    showStatusMessage(message);
    Logger::instance().info(message);
    
    onParseFinished();
}

void FunctionalityWidget::onStateChanged(BatchProcessState newState) {
    switch (newState) {
        case BatchProcessState::Running:
            m_pauseResumeButton->setText("暂停");
            break;
        case BatchProcessState::Paused:
            m_pauseResumeButton->setText("恢复");
            break;
        default:
            break;
    }
}

bool FunctionalityWidget::validateFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable();
}

void FunctionalityWidget::showStatusMessage(const QString &message, int duration) {
    m_statusLabel->setText(message);
    if (duration > 0) {
        QTimer::singleShot(duration, [this]() {
            m_statusLabel->setText("就绪");
        });
    }
}

void FunctionalityWidget::startFileAnalysis() {
    updateUIState(true);
    m_logView->clear();
    m_logView->append("开始解析文件...");
    m_progressBar->setValue(0);
    
    m_successCountLabel->setText("成功: 0");
    m_failedCountLabel->setText("失败: 0");
    m_skippedCountLabel->setText("跳过: 0");
    
    Logger::instance().info("开始解析文件: " + m_currentFilePath);
    
    FunctionParser& parser = FunctionParser::instance();
    m_extractionResult = parser.extractFunctions(m_currentFilePath);
    
    if (!m_extractionResult.success) {
        QMessageBox::critical(this, "错误", "解析文件失败: " + m_extractionResult.errorMessage);
        Logger::instance().error("解析文件失败: " + m_extractionResult.errorMessage);
        updateUIState(false);
        return;
    }
    
    if (m_extractionResult.functions.isEmpty()) {
        QMessageBox::warning(this, "警告", "未在文件中找到任何函数！");
        Logger::instance().warning("未在文件中找到任何函数");
        updateUIState(false);
        return;
    }
    
    m_functionCountLabel->setText(QString("找到 %1 个函数").arg(m_extractionResult.functions.size()));
    m_logView->append(QString("解析完成，找到 %1 个函数").arg(m_extractionResult.functions.size()));
    
    BatchProcessManager& batchManager = BatchProcessManager::instance();
    batchManager.startBatchProcessing(m_extractionResult.functions);
}

void FunctionalityWidget::updateUIState(bool isProcessing) {
    m_parseButton->setEnabled(!isProcessing);
    m_fileSelectButton->setEnabled(!isProcessing);
    m_pauseResumeButton->setVisible(isProcessing);
    m_cancelButton->setVisible(isProcessing);
    m_progressBar->setVisible(isProcessing);
    m_logView->setVisible(isProcessing);
}

