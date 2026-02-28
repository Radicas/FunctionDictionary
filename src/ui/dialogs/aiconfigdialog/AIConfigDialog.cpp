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
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_testButton = new QPushButton("测试连接", this);
    connect(m_testButton, &QPushButton::clicked, this, &AIConfigDialog::onTestConnectionClicked);
    buttonLayout->addWidget(m_testButton);

    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &AIConfigDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    m_saveButton = new QPushButton("保存", this);
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

    QMessageBox::information(this, "提示", "连接测试功能将在后续版本中实现。\n当前配置：\nProvider: " + config.provider + "\nBase URL: " + config.baseUrl + "\nModel ID: " + config.modelId);
    Logger::instance().info("用户测试AI连接，Base URL: " + config.baseUrl);
}
