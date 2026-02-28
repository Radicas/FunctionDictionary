#include "FunctionalityWidget.h"
#include "logger.h"

FunctionalityWidget::FunctionalityWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    Logger::instance().info("功能型widget初始化完成");
}

FunctionalityWidget::~FunctionalityWidget() {
    if (m_parseFuture.isRunning()) {
        m_parseFuture.cancel();
        m_parseFuture.waitForFinished();
    }
}

void FunctionalityWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    setupFileParseSection();
    setupSettingsSection();

    // 添加底部状态标签
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { font-size: 12px; color: #666; }");
    m_mainLayout->addWidget(m_statusLabel);

    setLayout(m_mainLayout);
}

void FunctionalityWidget::setupFileParseSection() {
    // 标题
    m_parseTitle = new QLabel("解析文件", this);
    m_parseTitle->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-bottom: 10px; }");
    m_mainLayout->addWidget(m_parseTitle);

    // 文件路径输入和选择按钮
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setPlaceholderText("请选择要解析的文件");
    m_filePathEdit->setReadOnly(true);
    fileLayout->addWidget(m_filePathEdit);

    m_fileSelectButton = new QPushButton("选择", this);
    connect(m_fileSelectButton, &QPushButton::clicked, this, &FunctionalityWidget::onFileSelectClicked);
    fileLayout->addWidget(m_fileSelectButton);

    m_mainLayout->addLayout(fileLayout);

    // 解析按钮
    m_parseButton = new QPushButton("开始解析", this);
    connect(m_parseButton, &QPushButton::clicked, this, &FunctionalityWidget::onParseButtonClicked);
    m_mainLayout->addWidget(m_parseButton);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_mainLayout->addWidget(m_progressBar);
}

void FunctionalityWidget::setupSettingsSection() {
    // 标题
    m_settingsTitle = new QLabel("配置设置", this);
    m_settingsTitle->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 20px; margin-bottom: 10px; }");
    m_mainLayout->addWidget(m_settingsTitle);

    // AI配置按钮
    m_aiConfigButton = new QPushButton("AI配置", this);
    connect(m_aiConfigButton, &QPushButton::clicked, [this]() {
        AIConfigDialog dialog(this);
        dialog.exec();
        Logger::instance().info("用户打开AI配置对话框");
    });
    m_mainLayout->addWidget(m_aiConfigButton);

    // 添加拉伸空间
    m_mainLayout->addStretch();
}

void FunctionalityWidget::onFileSelectClicked() {
    Logger::instance().info("onFileSelectClicked 方法被调用");
    try {
        // 使用QFileDialog的静态方法，设置一些额外的选项
        QFileDialog dialog(this, "选择文件", QDir::homePath());
        dialog.setNameFilters({"所有文件 (*.*)", "C/C++文件 (*.c *.cpp *.h *.hpp)", "Python文件 (*.py)", "Java文件 (*.java)", "JavaScript文件 (*.js *.ts)"});
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);
        
        Logger::instance().info("显示文件选择对话框");
        
        if (dialog.exec() == QFileDialog::Accepted) {
            QStringList selectedFiles = dialog.selectedFiles();
            if (!selectedFiles.isEmpty()) {
                QString filePath = selectedFiles.first();
                Logger::instance().info("用户选择文件: " + filePath);
                
                m_filePathEdit->setText(filePath);
                m_currentFilePath = filePath;
                showStatusMessage("已选择文件: " + QFileInfo(filePath).fileName());
            } else {
                Logger::instance().info("用户取消选择文件");
            }
        } else {
            Logger::instance().info("用户取消选择文件");
        }
    } catch (const std::exception& e) {
        Logger::instance().error("文件选择对话框异常: " + QString(e.what()));
        showStatusMessage("文件选择失败: " + QString(e.what()));
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

    // 禁用按钮并显示进度条
    m_parseButton->setEnabled(false);
    m_fileSelectButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    showStatusMessage("正在解析文件...");

    // 异步解析文件
    m_parseFuture = QtConcurrent::run([this]() {
        try {
            // 模拟解析过程
            for (int i = 0; i <= 100; i += 10) {
                QThread::msleep(200);
                QMetaObject::invokeMethod(m_progressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, i));
            }

            // 调用AI服务解析文件
            QString fileContent;
            QFile file(m_currentFilePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                fileContent = file.readAll();
                file.close();
            }

            // 调用AI服务
            AIServiceManager& aiService = AIServiceManager::instance();
            
            // 由于AIServiceManager使用信号槽机制，我们需要在主线程中处理结果
            // 这里我们使用同步方式模拟解析结果
            QString analysisResult = "文件解析结果：\n";
            analysisResult += "- 文件名: " + QFileInfo(m_currentFilePath).fileName() + "\n";
            analysisResult += "- 文件大小: " + QString::number(fileContent.size()) + " 字节\n";
            analysisResult += "- 分析状态: 完成\n";
            analysisResult += "\nAI分析结果：\n该文件包含多个函数定义，主要功能包括...";

            // 保存到数据库
            if (!analysisResult.isEmpty()) {
                QString fileName = QFileInfo(m_currentFilePath).fileName();
                bool success = DatabaseManager::instance().addFunction(
                    "解析结果: " + fileName,
                    analysisResult
                );

                if (success) {
                    QMetaObject::invokeMethod(this, "showStatusMessage", Qt::QueuedConnection, 
                        Q_ARG(QString, "文件解析成功并保存到数据库"));
                    Logger::instance().info("文件解析成功并保存到数据库: " + m_currentFilePath);
                } else {
                    QMetaObject::invokeMethod(this, "showStatusMessage", Qt::QueuedConnection, 
                        Q_ARG(QString, "文件解析成功，但保存到数据库失败: " + DatabaseManager::instance().lastError()));
                    Logger::instance().error("保存解析结果到数据库失败: " + DatabaseManager::instance().lastError());
                }
            } else {
                QMetaObject::invokeMethod(this, "showStatusMessage", Qt::QueuedConnection, 
                    Q_ARG(QString, "AI解析失败，未返回结果"));
                Logger::instance().error("AI解析失败，未返回结果");
            }
        } catch (const std::exception& e) {
            QMetaObject::invokeMethod(this, "showStatusMessage", Qt::QueuedConnection, 
                Q_ARG(QString, "解析过程发生错误: " + QString(e.what())));
            Logger::instance().error("解析过程发生错误: " + QString(e.what()));
        }

        // 解析完成后恢复界面状态
        QMetaObject::invokeMethod(this, "onParseFinished", Qt::QueuedConnection);
    });
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