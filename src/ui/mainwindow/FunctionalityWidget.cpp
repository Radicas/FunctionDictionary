#include "FunctionalityWidget.h"
#include "logger.h"
#include <QThread>
#include <QCoreApplication>

FunctionalityWidget::FunctionalityWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    
    AIServiceManager& aiService = AIServiceManager::instance();
    connect(&aiService, &AIServiceManager::analysisComplete, 
            this, &FunctionalityWidget::onAnalysisComplete);
    connect(&aiService, &AIServiceManager::analysisFailed, 
            this, &FunctionalityWidget::onAnalysisFailed);
    connect(&aiService, &AIServiceManager::analysisProgress, 
            this, &FunctionalityWidget::onAnalysisProgress);
    
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

    m_parseButton = new QPushButton("开始解析", this);
    m_parseButton->setObjectName("parseButton");
    connect(m_parseButton, &QPushButton::clicked, this, &FunctionalityWidget::onParseButtonClicked);
    m_mainLayout->addWidget(m_parseButton);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_mainLayout->addWidget(m_progressBar);
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
            showStatusMessage("已选择文件: " + QFileInfo(filePath).fileName());
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

    m_parseButton->setEnabled(false);
    m_fileSelectButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    showStatusMessage("正在解析文件...");

    readFileAndAnalyze();
}

void FunctionalityWidget::readFileAndAnalyze() {
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件：" + file.errorString());
        onParseFinished();
        return;
    }

    QString fileContent = file.readAll();
    file.close();

    if (fileContent.isEmpty()) {
        QMessageBox::warning(this, "警告", "文件内容为空！");
        onParseFinished();
        return;
    }

    AIServiceManager::instance().analyzeCode(fileContent);
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

void FunctionalityWidget::onParseFinished() {
    m_parseButton->setEnabled(true);
    m_fileSelectButton->setEnabled(true);
    m_progressBar->setVisible(false);
    m_progressBar->setValue(0);
}

bool FunctionalityWidget::validateFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable();
}

void FunctionalityWidget::showStatusMessage(const QString &message, int duration) {
    m_statusLabel->setText(message);
    
    // 如果指定了持续时间，设置定时器清除消息
    if (duration > 0) {
        QTimer::singleShot(duration, [this]() {
            m_statusLabel->setText("就绪");
        });
    }
}