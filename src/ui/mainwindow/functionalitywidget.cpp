/**
 * @file functionalitywidget.cpp
 * @brief 功能型widget组件实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "ui/mainwindow/functionalitywidget.h"
#include "common/logger/logger.h"
#include <QThread>
#include <QCoreApplication>

FunctionalityWidget::FunctionalityWidget(QWidget *parent)
    : QWidget(parent), m_currentMode(ParseMode::SingleFile), m_successCount(0), m_failedCount(0), m_skippedCount(0)
{
    setupUI();

    AICodeParser &aiParser = AICodeParser::instance();
    connect(&aiParser, &AICodeParser::parseComplete,
            this, &FunctionalityWidget::onAIParseComplete);
    connect(&aiParser, &AICodeParser::parseFailed,
            this, &FunctionalityWidget::onAIParseFailed);
    connect(&aiParser, &AICodeParser::parseProgress,
            this, &FunctionalityWidget::onAIParseProgress);

    BatchCodeParser &batchParser = BatchCodeParser::instance();
    connect(&batchParser, &BatchCodeParser::batchProgress,
            this, &FunctionalityWidget::onBatchProgress);
    connect(&batchParser, &BatchCodeParser::fileParsed,
            this, &FunctionalityWidget::onFileParsed);
    connect(&batchParser, &BatchCodeParser::batchComplete,
            this, &FunctionalityWidget::onBatchComplete);
    connect(&batchParser, &BatchCodeParser::batchFailed,
            this, &FunctionalityWidget::onBatchFailed);
    connect(&batchParser, &BatchCodeParser::batchCancelled,
            this, &FunctionalityWidget::onBatchCancelled);

    connect(&aiParser, &AICodeParser::parseCancelled,
            this, &FunctionalityWidget::onAIParseCancelled);

    Logger::instance().info("功能型widget初始化完成");
}

FunctionalityWidget::~FunctionalityWidget()
{
}

void FunctionalityWidget::setupUI()
{
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

void FunctionalityWidget::setupFileParseSection()
{
    m_modeLabel = new QLabel("解析模式", this);
    m_modeLabel->setObjectName("titleLabel");
    m_mainLayout->addWidget(m_modeLabel);

    m_modeComboBox = new QComboBox(this);
    m_modeComboBox->addItem("单文件", static_cast<int>(ParseMode::SingleFile));
    m_modeComboBox->addItem("文件夹", static_cast<int>(ParseMode::Folder));
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FunctionalityWidget::onModeChanged);
    m_mainLayout->addWidget(m_modeComboBox);

    m_pathLabel = new QLabel("解析文件", this);
    m_pathLabel->setObjectName("titleLabel");
    m_mainLayout->addWidget(m_pathLabel);

    m_fileInfoLabel = new QLabel("未选择", this);
    m_fileInfoLabel->setObjectName("fileInfoLabel");
    m_mainLayout->addWidget(m_fileInfoLabel);

    m_functionCountLabel = new QLabel("", this);
    m_functionCountLabel->setObjectName("functionCountLabel");
    m_mainLayout->addWidget(m_functionCountLabel);

    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("请选择要解析的文件或文件夹");
    m_pathEdit->setReadOnly(true);
    pathLayout->addWidget(m_pathEdit);

    m_selectButton = new QPushButton("选择", this);
    m_selectButton->setObjectName("fileSelectButton");
    connect(m_selectButton, &QPushButton::clicked, this, &FunctionalityWidget::onSelectClicked);
    pathLayout->addWidget(m_selectButton);

    m_mainLayout->addLayout(pathLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_parseButton = new QPushButton("开始解析", this);
    m_parseButton->setObjectName("parseButton");
    connect(m_parseButton, &QPushButton::clicked, this, &FunctionalityWidget::onParseButtonClicked);
    buttonLayout->addWidget(m_parseButton);

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

void FunctionalityWidget::setupSettingsSection()
{
    m_settingsTitle = new QLabel("配置设置", this);
    m_settingsTitle->setObjectName("titleLabel");
    m_mainLayout->addWidget(m_settingsTitle);

    m_recursiveCheckBox = new QCheckBox("递归扫描子文件夹", this);
    m_recursiveCheckBox->setChecked(true);
    m_recursiveCheckBox->setEnabled(false);
    m_mainLayout->addWidget(m_recursiveCheckBox);

    m_skipExistingCheckBox = new QCheckBox("跳过已存在的函数", this);
    m_skipExistingCheckBox->setChecked(true);
    m_mainLayout->addWidget(m_skipExistingCheckBox);

    m_aiConfigButton = new QPushButton("AI配置", this);
    m_aiConfigButton->setObjectName("aiConfigButton");
    connect(m_aiConfigButton, &QPushButton::clicked, [this]()
            {
        AIConfigDialog dialog(this);
        dialog.exec();
        Logger::instance().info("用户打开AI配置对话框"); });
    m_mainLayout->addWidget(m_aiConfigButton);

    m_mainLayout->addStretch();
}

void FunctionalityWidget::onModeChanged(int index)
{
    m_currentMode = static_cast<ParseMode>(m_modeComboBox->itemData(index).toInt());

    if (m_currentMode == ParseMode::SingleFile)
    {
        m_pathLabel->setText("解析文件");
        m_recursiveCheckBox->setEnabled(false);
    }
    else
    {
        m_pathLabel->setText("解析文件夹");
        m_recursiveCheckBox->setEnabled(true);
    }

    m_pathEdit->clear();
    m_fileInfoLabel->setText("未选择");
    m_functionCountLabel->setText("");
    m_currentPath.clear();

    Logger::instance().info(QString("切换解析模式: %1").arg(m_currentMode == ParseMode::SingleFile ? "单文件" : "文件夹"));
}

void FunctionalityWidget::onSelectClicked()
{
    Logger::instance().info("onSelectClicked 方法被调用");

    if (m_currentMode == ParseMode::SingleFile)
    {
        QFileDialog dialog(window(), "选择文件", QDir::homePath(),
                           "所有文件 (*.*);;C/C++文件;;Python文件;;Java文件;;JavaScript文件");
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);

        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selectedFiles = dialog.selectedFiles();
            if (!selectedFiles.isEmpty())
            {
                QString filePath = selectedFiles.first();
                Logger::instance().info("用户选择文件: " + filePath);
                m_pathEdit->setText(filePath);
                m_currentPath = filePath;

                QFileInfo fileInfo(filePath);
                m_fileInfoLabel->setText(QString("文件: %1, 大小: %2 KB")
                                             .arg(fileInfo.fileName())
                                             .arg(fileInfo.size() / 1024.0, 0, 'f', 2));

                showStatusMessage("已选择文件: " + fileInfo.fileName());
            }
        }
    }
    else
    {
        QFileDialog dialog(window(), "选择文件夹", QDir::homePath());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);

        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selectedDirs = dialog.selectedFiles();
            if (!selectedDirs.isEmpty())
            {
                QString folderPath = selectedDirs.first();
                Logger::instance().info("用户选择文件夹: " + folderPath);
                m_pathEdit->setText(folderPath);
                m_currentPath = folderPath;

                QDir dir(folderPath);
                m_fileInfoLabel->setText(QString("文件夹: %1").arg(dir.dirName()));

                showStatusMessage("已选择文件夹: " + dir.dirName());
            }
        }
    }
}

void FunctionalityWidget::onParseButtonClicked()
{
    if (m_currentPath.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请先选择要解析的文件或文件夹！");
        return;
    }

    if (!validatePath(m_currentPath))
    {
        QMessageBox::critical(this, "错误", "所选路径无效或无法访问！");
        return;
    }

    startParsing();
}

void FunctionalityWidget::onCancelClicked()
{
    showStatusMessage("正在取消解析...");
    
    if (m_currentMode == ParseMode::SingleFile)
    {
        AICodeParser::instance().cancelParsing();
    }
    else
    {
        BatchCodeParser::instance().cancelParsing();
    }
}

void FunctionalityWidget::onParseFinished()
{
    updateUIState(false);
}

void FunctionalityWidget::onAIParseComplete(const AIParseResult &result)
{
    m_logView->append(QString("解析完成，找到 %1 个函数").arg(result.functions.size()));
    m_progressBar->setValue(90);

    m_successCount = 0;
    m_failedCount = 0;
    m_skippedCount = 0;
    bool skipExisting = m_skipExistingCheckBox->isChecked();

    for (const FunctionData &funcData : result.functions)
    {
        if (skipExisting && DatabaseManager::instance().functionExists(funcData.key))
        {
            m_skippedCount++;
            m_logView->append(QString("[跳过] %1 - 已存在").arg(funcData.key));
        }
        else
        {
            FunctionData data = funcData;
            data.createTime = QDateTime::currentDateTime();
            data.analyzeTime = QDateTime::currentDateTime();

            if (DatabaseManager::instance().addFunction(data))
            {
                m_successCount++;
                m_logView->append(QString("[成功] %1").arg(funcData.key));
            }
            else
            {
                m_failedCount++;
                m_logView->append(QString("[失败] %1 - %2").arg(funcData.key).arg(DatabaseManager::instance().lastError()));
            }
        }
    }

    m_successCountLabel->setText(QString("成功: %1").arg(m_successCount));
    m_failedCountLabel->setText(QString("失败: %1").arg(m_failedCount));
    m_skippedCountLabel->setText(QString("跳过: %1").arg(m_skippedCount));

    m_progressBar->setValue(100);

    QString message = QString("解析完成！成功: %1, 失败: %2, 跳过: %3")
                          .arg(m_successCount)
                          .arg(m_failedCount)
                          .arg(m_skippedCount);
    QMessageBox::information(this, "完成", message);
    showStatusMessage(message);
    Logger::instance().info(message);

    emit batchProcessingCompleted();
    onParseFinished();
}

void FunctionalityWidget::onAIParseFailed(const QString &error)
{
    QMessageBox::critical(this, "错误", "AI 解析失败：" + error);
    Logger::instance().error("AI 解析失败: " + error);
    m_logView->append(QString("[错误] %1").arg(error));
    onParseFinished();
}

void FunctionalityWidget::onAIParseProgress(const QString &stage, const QString &message)
{
    QString logMsg = QString("[%1] %2").arg(stage).arg(message);
    showStatusMessage(message);
    m_logView->append(logMsg);

    if (stage == "读取文件")
    {
        m_progressBar->setValue(10);
    }
    else if (stage == "构建请求")
    {
        m_progressBar->setValue(20);
    }
    else if (stage == "发送请求")
    {
        m_progressBar->setValue(30);
        m_logView->append("正在等待AI响应，这可能需要一些时间...");
    }
    else if (stage == "解析响应")
    {
        m_progressBar->setValue(70);
    }
    else if (stage == "保存数据")
    {
        m_progressBar->setValue(90);
    }
}

void FunctionalityWidget::onBatchProgress(const BatchParseProgress &progress)
{
    QString logMsg = QString("[%1/%2] %3: %4")
                         .arg(progress.processedFiles)
                         .arg(progress.totalFiles)
                         .arg(progress.currentStage)
                         .arg(progress.currentMessage);

    showStatusMessage(progress.currentMessage);
    m_logView->append(logMsg);

    int progressValue = 0;
    if (progress.totalFiles > 0)
    {
        progressValue = (progress.processedFiles * 100) / progress.totalFiles;
    }
    m_progressBar->setValue(progressValue);

    m_successCountLabel->setText(QString("成功: %1").arg(progress.successCount));
    m_failedCountLabel->setText(QString("失败: %1").arg(progress.failedCount));
    m_skippedCountLabel->setText(QString("跳过: %1").arg(progress.skippedCount));
}

void FunctionalityWidget::onFileParsed(const QString &filePath, const AIParseResult &result)
{
    QFileInfo fileInfo(filePath);
    QString logMsg = QString("[完成] %1: 提取 %2 个函数")
                         .arg(fileInfo.fileName())
                         .arg(result.functions.size());
    m_logView->append(logMsg);
}

void FunctionalityWidget::onBatchComplete(const BatchParseResult &result)
{
    QString message = QString("批量解析完成！\n总计: %1 个文件\n成功: %2\n失败: %3\n跳过: %4\n提取函数: %5 个")
                          .arg(result.totalFiles)
                          .arg(result.successCount)
                          .arg(result.failedCount)
                          .arg(result.skippedCount)
                          .arg(result.allFunctions.size());

    if (!result.failedFiles.isEmpty())
    {
        message += "\n\n失败的文件:\n" + result.failedFiles.join("\n");
    }

    QMessageBox::information(this, "完成", message);
    showStatusMessage(QString("批量解析完成，共处理 %1 个文件").arg(result.totalFiles));
    Logger::instance().info(message);

    emit batchProcessingCompleted();
    onParseFinished();
}

void FunctionalityWidget::onBatchFailed(const QString &error)
{
    QMessageBox::critical(this, "错误", "批量解析失败：" + error);
    Logger::instance().error("批量解析失败: " + error);
    m_logView->append(QString("[错误] %1").arg(error));
    onParseFinished();
}

void FunctionalityWidget::onAIParseCancelled()
{
    showStatusMessage("已取消解析");
    m_logView->append("[取消] AI解析已取消");
    onParseFinished();
}

void FunctionalityWidget::onBatchCancelled()
{
    showStatusMessage("已取消批量解析");
    m_logView->append("[取消] 批量解析已取消");
    onParseFinished();
}

bool FunctionalityWidget::validatePath(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && (fileInfo.isFile() || fileInfo.isDir());
}

void FunctionalityWidget::showStatusMessage(const QString &message, int duration)
{
    m_statusLabel->setText(message);
    if (duration > 0)
    {
        QTimer::singleShot(duration, [this]()
                           { m_statusLabel->setText("就绪"); });
    }
}

void FunctionalityWidget::startParsing()
{
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    if (!AIConfigManager::instance().isConfigValid(config)) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("AI配置缺失");
        msgBox.setText("AI配置不完整，请先配置AI服务！");
        msgBox.setInformativeText("是否立即配置AI服务？");
        QPushButton *configBtn = msgBox.addButton("立即配置", QMessageBox::ActionRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == configBtn) {
            AIConfigDialog dialog(this);
            dialog.exec();
            Logger::instance().info("用户打开AI配置对话框");
        }
        return;
    }
    
    updateUIState(true);
    m_logView->clear();
    m_progressBar->setValue(0);

    m_successCount = 0;
    m_failedCount = 0;
    m_skippedCount = 0;

    m_successCountLabel->setText("成功: 0");
    m_failedCountLabel->setText("失败: 0");
    m_skippedCountLabel->setText("跳过: 0");

    bool skipExisting = m_skipExistingCheckBox->isChecked();

    if (m_currentMode == ParseMode::SingleFile)
    {
        m_logView->append("开始解析文件...");
        Logger::instance().info("开始AI解析文件: " + m_currentPath);
        AICodeParser::instance().parseFile(m_currentPath);
    }
    else
    {
        bool recursive = m_recursiveCheckBox->isChecked();
        m_logView->append(QString("开始批量解析文件夹%1...")
                              .arg(recursive ? "（递归）" : ""));
        Logger::instance().info(QString("开始批量解析文件夹: %1, 递归: %2")
                                    .arg(m_currentPath)
                                    .arg(recursive));

        BatchCodeParser::instance().setSkipExisting(skipExisting);
        BatchCodeParser::instance().parseFolder(m_currentPath, recursive);
    }
}

void FunctionalityWidget::updateUIState(bool isProcessing)
{
    m_parseButton->setEnabled(!isProcessing);
    m_selectButton->setEnabled(!isProcessing);
    m_modeComboBox->setEnabled(!isProcessing);
    m_recursiveCheckBox->setEnabled(!isProcessing && m_currentMode == ParseMode::Folder);
    m_skipExistingCheckBox->setEnabled(!isProcessing);
    m_cancelButton->setVisible(isProcessing);
    m_progressBar->setVisible(isProcessing);
    m_logView->setVisible(isProcessing);
}
