/**
 * @file parsecontrolwidget.cpp
 * @brief 解析控制组件实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "ui/mainwindow/widgets/parsecontrolwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "common/logger/logger.h"

ParseControlWidget::ParseControlWidget(QWidget* parent) : QWidget(parent), m_isParsing(false)
{
    setupUI();
    Logger::instance().info("解析控制组件初始化完成");
}

ParseControlWidget::~ParseControlWidget() {}

void ParseControlWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_settingsTitle = new QLabel("配置设置", this);
    m_settingsTitle->setObjectName("titleLabel");
    mainLayout->addWidget(m_settingsTitle);

    m_recursiveCheckBox = new QCheckBox("递归扫描子文件夹", this);
    m_recursiveCheckBox->setChecked(true);
    m_recursiveCheckBox->setEnabled(false);
    mainLayout->addWidget(m_recursiveCheckBox);

    m_skipExistingCheckBox = new QCheckBox("跳过已存在的函数", this);
    m_skipExistingCheckBox->setChecked(true);
    mainLayout->addWidget(m_skipExistingCheckBox);

    m_aiConfigButton = new QPushButton("AI配置", this);
    m_aiConfigButton->setObjectName("aiConfigButton");
    connect(m_aiConfigButton, &QPushButton::clicked, this, &ParseControlWidget::onAiConfigClicked);
    mainLayout->addWidget(m_aiConfigButton);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_parseButton = new QPushButton("开始解析", this);
    m_parseButton->setObjectName("parseButton");
    connect(m_parseButton, &QPushButton::clicked, this, &ParseControlWidget::onParseClicked);
    buttonLayout->addWidget(m_parseButton);

    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setVisible(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &ParseControlWidget::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void ParseControlWidget::setParsing(bool isParsing)
{
    m_isParsing = isParsing;

    m_parseButton->setEnabled(!isParsing);
    m_parseButton->setVisible(!isParsing);
    m_cancelButton->setVisible(isParsing);
    m_aiConfigButton->setEnabled(!isParsing);
    m_recursiveCheckBox->setEnabled(!isParsing && !m_recursiveCheckBox->isEnabled());
    m_skipExistingCheckBox->setEnabled(!isParsing);
}

bool ParseControlWidget::isRecursive() const
{
    return m_recursiveCheckBox->isChecked();
}

bool ParseControlWidget::skipExisting() const
{
    return m_skipExistingCheckBox->isChecked();
}

void ParseControlWidget::setControlsEnabled(bool enabled)
{
    m_parseButton->setEnabled(enabled && !m_isParsing);
    m_aiConfigButton->setEnabled(enabled && !m_isParsing);
    m_skipExistingCheckBox->setEnabled(enabled && !m_isParsing);
}

void ParseControlWidget::setRecursiveEnabled(bool enabled)
{
    m_recursiveCheckBox->setEnabled(enabled && !m_isParsing);
}

void ParseControlWidget::onParseClicked()
{
    Logger::instance().info("用户点击开始解析按钮");
    emit parseRequested();
}

void ParseControlWidget::onCancelClicked()
{
    Logger::instance().info("用户点击取消按钮");
    emit cancelRequested();
}

void ParseControlWidget::onAiConfigClicked()
{
    Logger::instance().info("用户点击AI配置按钮");
    emit aiConfigRequested();
}
