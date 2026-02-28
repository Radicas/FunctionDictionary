/**
 * @file AIConfigDialog.cpp
 * @brief AI配置对话框实现
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#include "aiconfigdialog.h"
#include "logger.h"
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

AIConfigDialog::AIConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_providerComboBox(nullptr)
    , m_baseUrlLineEdit(nullptr)
    , m_apiKeyLineEdit(nullptr)
    , m_modelIdLineEdit(nullptr)
    , m_testButton(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr) {
    setupUI();
    loadConfigToUI();
    Logger::instance().info("AI配置对话框初始化完成");
}

AIConfigDialog::~AIConfigDialog() {
}

void AIConfigDialog::setupUI() {
    setWindowTitle("AI配置");
    setMinimumSize(500, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    m_providerComboBox = new QComboBox(this);
    m_providerComboBox->addItem("OpenAI Compatible");
    formLayout->addRow("API Provider:", m_providerComboBox);

    m_baseUrlLineEdit = new QLineEdit(this);
    m_baseUrlLineEdit->setPlaceholderText("http://10.0.3.22:22080/v1");
    formLayout->addRow("Base URL:", m_baseUrlLineEdit);

    m_apiKeyLineEdit = new QLineEdit(this);
    m_apiKeyLineEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyLineEdit->setPlaceholderText("请输入API Key");
    formLayout->addRow("API Key:", m_apiKeyLineEdit);

    m_modelIdLineEdit = new QLineEdit(this);
    m_modelIdLineEdit->setPlaceholderText("Qwen3-Coder-Next");
    formLayout->addRow("Model ID:", m_modelIdLineEdit);

    mainLayout->addLayout(formLayout);

    QLabel *infoLabel = new QLabel("This key is stored locally and only used to make API requests from this extension.", this);
    infoLabel->setObjectName("hintLabel");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_testButton = new QPushButton("测试连接", this);
    m_testButton->setObjectName("testButton");
    connect(m_testButton, &QPushButton::clicked, this, &AIConfigDialog::onTestConnectionClicked);
    buttonLayout->addWidget(m_testButton);

    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setObjectName("cancelButton");
    connect(m_cancelButton, &QPushButton::clicked, this, &AIConfigDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    m_saveButton = new QPushButton("保存", this);
    m_saveButton->setObjectName("primaryButton");
    m_saveButton->setProperty("primary", true);
    connect(m_saveButton, &QPushButton::clicked, this, &AIConfigDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveButton);

    mainLayout->addLayout(buttonLayout);
}

void AIConfigDialog::loadConfigToUI() {
    AIConfig config = AIConfigManager::instance().getCurrentConfig();
    m_providerComboBox->setCurrentText(config.provider);
    m_baseUrlLineEdit->setText(config.baseUrl);
    m_apiKeyLineEdit->setText(config.apiKey);
    m_modelIdLineEdit->setText(config.modelId);
}

AIConfig AIConfigDialog::getConfigFromUI() {
    AIConfig config;
    config.provider = m_providerComboBox->currentText();
    config.baseUrl = m_baseUrlLineEdit->text().trimmed();
    config.apiKey = m_apiKeyLineEdit->text().trimmed();
    config.modelId = m_modelIdLineEdit->text().trimmed();
    return config;
}

void AIConfigDialog::onSaveClicked() {
    AIConfig config = getConfigFromUI();

    if (config.baseUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入Base URL！");
        return;
    }

    if (config.apiKey.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入API Key！");
        return;
    }

    if (config.modelId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入Model ID！");
        return;
    }

    AIConfigManager::instance().saveConfig(config);
    QMessageBox::information(this, "成功", "AI配置已保存！");
    accept();
}

void AIConfigDialog::onCancelClicked() {
    reject();
}

void AIConfigDialog::onTestConnectionClicked() {
    AIConfig config = getConfigFromUI();

    if (!AIConfigManager::instance().isConfigValid(config)) {
        QMessageBox::warning(this, "警告", "请先填写完整的配置信息！");
        return;
    }

    // 禁用测试按钮，防止重复点击
    m_testButton->setEnabled(false);
    m_testButton->setText("测试中...");

    // 创建网络访问管理器
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // 构建请求URL (使用models端点测试连接)
    QUrl url(config.baseUrl + "/models");
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + config.apiKey).toUtf8());

    // 发送GET请求
    QNetworkReply *reply = manager->get(request);

    // 设置超时定时器
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply, manager, timer]() {
        reply->abort();
        QMessageBox::warning(this, "连接超时", "测试连接超时，请检查网络连接或Base URL是否正确！");
        Logger::instance().warning("AI连接测试超时");
        // 恢复测试按钮状态
        m_testButton->setEnabled(true);
        m_testButton->setText("测试连接");
        reply->deleteLater();
        manager->deleteLater();
        timer->deleteLater();
    });
    timer->start(10000); // 10秒超时

    // 处理网络响应
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, timer, config = config]() {
        timer->stop();
        timer->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            // 连接成功
            QMessageBox::information(this, "连接成功", "测试连接成功！\n当前配置：\nProvider: " + config.provider + "\nBase URL: " + config.baseUrl + "\nModel ID: " + config.modelId);
            Logger::instance().info("AI连接测试成功，Base URL: " + config.baseUrl);
        } else {
            // 连接失败
            QString errorString = reply->errorString();
            QMessageBox::warning(this, "连接失败", "测试连接失败：" + errorString + "\n请检查配置信息是否正确！");
            Logger::instance().error("AI连接测试失败，Base URL: " + config.baseUrl + ", 错误: " + errorString);
        }

        // 恢复测试按钮状态
        m_testButton->setEnabled(true);
        m_testButton->setText("测试连接");

        reply->deleteLater();
        manager->deleteLater();
    });
}
