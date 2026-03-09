/**
 * @file aiconfigdialog.cpp
 * @brief AI配置对话框实现，支持多配置管理
 * @author Developer
 * @date 2026-02-28
 * @version 2.0
 */

#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "common/logger/logger.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QGroupBox>

AIConfigDialog::AIConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_configListWidget(nullptr)
    , m_newConfigButton(nullptr)
    , m_deleteConfigButton(nullptr)
    , m_configNameLineEdit(nullptr)
    , m_providerComboBox(nullptr)
    , m_baseUrlLineEdit(nullptr)
    , m_apiKeyLineEdit(nullptr)
    , m_modelComboBox(nullptr)
    , m_fetchModelsButton(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
    , m_isNewConfig(false)
    , m_isLoading(false) {
    setupUI();
    loadConfigList();
    
    connect(&ModelListFetcher::instance(), &ModelListFetcher::modelsFetched,
            this, &AIConfigDialog::onModelsFetched);
    connect(&ModelListFetcher::instance(), &ModelListFetcher::fetchFailed,
            this, &AIConfigDialog::onFetchFailed);
    
    Logger::instance().info("AI配置对话框初始化完成");
}

AIConfigDialog::~AIConfigDialog() {
}

void AIConfigDialog::setupUI() {
    setWindowTitle("AI配置管理");
    setMinimumSize(800, 500);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QGroupBox *listGroup = new QGroupBox("配置列表", this);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);

    m_configListWidget = new QListWidget(this);
    m_configListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_configListWidget, &QListWidget::currentRowChanged,
            this, &AIConfigDialog::onConfigSelectionChanged);
    listLayout->addWidget(m_configListWidget);

    QHBoxLayout *listButtonLayout = new QHBoxLayout();
    m_newConfigButton = new QPushButton("新建", this);
    m_deleteConfigButton = new QPushButton("删除", this);
    connect(m_newConfigButton, &QPushButton::clicked, this, &AIConfigDialog::onNewConfigClicked);
    connect(m_deleteConfigButton, &QPushButton::clicked, this, &AIConfigDialog::onDeleteConfigClicked);
    listButtonLayout->addWidget(m_newConfigButton);
    listButtonLayout->addWidget(m_deleteConfigButton);
    listLayout->addLayout(listButtonLayout);

    mainLayout->addWidget(listGroup);

    QGroupBox *detailGroup = new QGroupBox("配置详情", this);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);

    QFormLayout *formLayout = new QFormLayout();

    m_configNameLineEdit = new QLineEdit(this);
    m_configNameLineEdit->setPlaceholderText("请输入配置名称");
    formLayout->addRow("配置名称:", m_configNameLineEdit);

    m_providerComboBox = new QComboBox(this);
    m_providerComboBox->addItem("其他");
    m_providerComboBox->addItem("OpenAI");
    m_providerComboBox->addItem("Azure OpenAI");
    m_providerComboBox->addItem("Claude");
    m_providerComboBox->addItem("DeepSeek");
    m_providerComboBox->addItem("Qwen");
    formLayout->addRow("API Provider:", m_providerComboBox);

    m_baseUrlLineEdit = new QLineEdit(this);
    m_baseUrlLineEdit->setPlaceholderText("https://api.openai.com/v1");
    connect(m_baseUrlLineEdit, &QLineEdit::textChanged, this, &AIConfigDialog::onCredentialsChanged);
    formLayout->addRow("Base URL:", m_baseUrlLineEdit);

    m_apiKeyLineEdit = new QLineEdit(this);
    m_apiKeyLineEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyLineEdit->setPlaceholderText("请输入API Key");
    connect(m_apiKeyLineEdit, &QLineEdit::textChanged, this, &AIConfigDialog::onCredentialsChanged);
    formLayout->addRow("API Key:", m_apiKeyLineEdit);

    QHBoxLayout *modelLayout = new QHBoxLayout();
    m_modelComboBox = new QComboBox(this);
    m_modelComboBox->setEditable(true);
    m_modelComboBox->setPlaceholderText("选择或输入模型名称");
    m_fetchModelsButton = new QPushButton("获取模型列表", this);
    connect(m_fetchModelsButton, &QPushButton::clicked, this, &AIConfigDialog::onFetchModelsClicked);
    modelLayout->addWidget(m_modelComboBox, 1);
    modelLayout->addWidget(m_fetchModelsButton);
    formLayout->addRow("模型:", modelLayout);

    detailLayout->addLayout(formLayout);

    QLabel *infoLabel = new QLabel("提示：API Key仅存储在本地，用于发起API请求。", this);
    infoLabel->setObjectName("hintLabel");
    infoLabel->setWordWrap(true);
    detailLayout->addWidget(infoLabel);

    detailLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
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

    detailLayout->addLayout(buttonLayout);

    mainLayout->addWidget(detailGroup, 1);

    updateButtonStates();
}

void AIConfigDialog::loadConfigList() {
    m_isLoading = true;
    m_configListWidget->clear();

    QStringList configNames = AIConfigManager::instance().getAllConfigNames();
    for (const QString& name : configNames) {
        m_configListWidget->addItem(name);
    }

    QString currentConfigName = AIConfigManager::instance().getCurrentConfigName();
    if (!currentConfigName.isEmpty()) {
        QList<QListWidgetItem*> items = m_configListWidget->findItems(currentConfigName, Qt::MatchExactly);
        if (!items.isEmpty()) {
            m_configListWidget->setCurrentItem(items.first());
        }
    }

    m_isLoading = false;
    updateButtonStates();
}

void AIConfigDialog::onConfigSelectionChanged(int currentRow) {
    if (m_isLoading) {
        return;
    }

    if (currentRow < 0) {
        clearUI();
        return;
    }

    QListWidgetItem* item = m_configListWidget->item(currentRow);
    if (!item) {
        return;
    }

    QString configName = item->text();
    m_currentConfigName = configName;
    m_isNewConfig = false;

    AIConfig config = AIConfigManager::instance().loadConfig(configName);
    loadConfigToUI(config);
    updateButtonStates();
}

void AIConfigDialog::onNewConfigClicked() {
    m_isLoading = true;
    
    clearUI();
    
    m_currentConfigName.clear();
    m_isNewConfig = true;
    
    m_configListWidget->clearSelection();
    
    m_configNameLineEdit->setText("新配置");
    m_configNameLineEdit->setFocus();
    m_configNameLineEdit->selectAll();
    
    m_isLoading = false;
    updateButtonStates();
}

void AIConfigDialog::onDeleteConfigClicked() {
    if (m_currentConfigName.isEmpty() && !m_isNewConfig) {
        QMessageBox::warning(this, "警告", "请先选择要删除的配置！");
        return;
    }

    QString configName = m_isNewConfig ? m_configNameLineEdit->text().trimmed() : m_currentConfigName;
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "确认删除", 
        QString("确定要删除配置 \"%1\" 吗？").arg(configName),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (AIConfigManager::instance().deleteConfig(configName)) {
            loadConfigList();
            clearUI();
            QMessageBox::information(this, "成功", "配置已删除！");
        } else {
            QMessageBox::warning(this, "失败", "删除配置失败！");
        }
    }
}

void AIConfigDialog::onSaveClicked() {
    if (!validateInput()) {
        return;
    }

    AIConfig config = getConfigFromUI();
    QString configName = config.configName;

    if (configName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入配置名称！");
        return;
    }

    if (!m_isNewConfig && configName != m_currentConfigName) {
        if (AIConfigManager::instance().hasConfig(configName)) {
            QMessageBox::warning(this, "警告", "配置名称已存在，请使用其他名称！");
            return;
        }
    }

    AIConfigManager::instance().saveConfig(configName, config);
    AIConfigManager::instance().setCurrentConfig(configName);

    loadConfigList();
    
    QMessageBox::information(this, "成功", "AI配置已保存！");
    accept();
}

void AIConfigDialog::onCancelClicked() {
    reject();
}

void AIConfigDialog::onFetchModelsClicked() {
    QString baseUrl = m_baseUrlLineEdit->text().trimmed();
    QString apiKey = m_apiKeyLineEdit->text().trimmed();

    if (baseUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先输入Base URL！");
        return;
    }

    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先输入API Key！");
        return;
    }

    m_fetchModelsButton->setEnabled(false);
    m_fetchModelsButton->setText("获取中...");
    m_modelComboBox->clear();
    m_modelComboBox->setPlaceholderText("正在获取模型列表...");

    ModelListFetcher::instance().fetchModels(baseUrl, apiKey);
}

void AIConfigDialog::onModelsFetched(const QStringList& models) {
    m_fetchModelsButton->setEnabled(true);
    m_fetchModelsButton->setText("获取模型列表");

    m_modelComboBox->clear();
    m_modelComboBox->addItems(models);

    if (!models.isEmpty()) {
        m_modelComboBox->setCurrentIndex(0);
    }

    QMessageBox::information(this, "成功", QString("成功获取 %1 个模型！").arg(models.size()));
}

void AIConfigDialog::onFetchFailed(const QString& error) {
    m_fetchModelsButton->setEnabled(true);
    m_fetchModelsButton->setText("获取模型列表");
    m_modelComboBox->setPlaceholderText("选择或输入模型名称");

    QMessageBox::warning(this, "获取失败", error);
}

void AIConfigDialog::onCredentialsChanged() {
    updateButtonStates();
}

void AIConfigDialog::loadConfigToUI(const AIConfig& config) {
    m_isLoading = true;
    
    m_configNameLineEdit->setText(config.configName);
    
    int providerIndex = m_providerComboBox->findText(config.provider);
    if (providerIndex >= 0) {
        m_providerComboBox->setCurrentIndex(providerIndex);
    } else {
        m_providerComboBox->setCurrentText(config.provider);
    }
    
    m_baseUrlLineEdit->setText(config.baseUrl);
    m_apiKeyLineEdit->setText(config.apiKey);
    
    m_modelComboBox->clear();
    if (!config.modelList.isEmpty()) {
        m_modelComboBox->addItems(config.modelList);
    }
    
    if (!config.defaultModel.isEmpty()) {
        int modelIndex = m_modelComboBox->findText(config.defaultModel);
        if (modelIndex >= 0) {
            m_modelComboBox->setCurrentIndex(modelIndex);
        } else {
            m_modelComboBox->setCurrentText(config.defaultModel);
        }
    }
    
    m_isLoading = false;
}

AIConfig AIConfigDialog::getConfigFromUI() {
    AIConfig config;
    config.configName = m_configNameLineEdit->text().trimmed();
    config.provider = m_providerComboBox->currentText();
    config.baseUrl = m_baseUrlLineEdit->text().trimmed();
    config.apiKey = m_apiKeyLineEdit->text().trimmed();
    config.defaultModel = m_modelComboBox->currentText().trimmed();
    
    for (int i = 0; i < m_modelComboBox->count(); ++i) {
        QString model = m_modelComboBox->itemText(i);
        if (!model.isEmpty()) {
            config.modelList.append(model);
        }
    }
    
    return config;
}

void AIConfigDialog::clearUI() {
    m_isLoading = true;
    
    m_configNameLineEdit->clear();
    m_providerComboBox->setCurrentIndex(0);
    m_baseUrlLineEdit->clear();
    m_apiKeyLineEdit->clear();
    m_modelComboBox->clear();
    m_modelComboBox->setPlaceholderText("选择或输入模型名称");
    
    m_currentConfigName.clear();
    m_isNewConfig = false;
    
    m_isLoading = false;
}

void AIConfigDialog::updateButtonStates() {
    bool hasSelection = m_configListWidget->currentRow() >= 0 || m_isNewConfig;
    bool hasCredentials = !m_baseUrlLineEdit->text().trimmed().isEmpty() && 
                          !m_apiKeyLineEdit->text().trimmed().isEmpty();
    
    m_deleteConfigButton->setEnabled(hasSelection && !m_isNewConfig);
    m_fetchModelsButton->setEnabled(hasCredentials);
}

bool AIConfigDialog::validateInput() {
    if (m_configNameLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入配置名称！");
        m_configNameLineEdit->setFocus();
        return false;
    }

    if (m_baseUrlLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入Base URL！");
        m_baseUrlLineEdit->setFocus();
        return false;
    }

    if (m_apiKeyLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入API Key！");
        m_apiKeyLineEdit->setFocus();
        return false;
    }

    if (m_modelComboBox->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择或输入模型名称！");
        m_modelComboBox->setFocus();
        return false;
    }

    return true;
}
