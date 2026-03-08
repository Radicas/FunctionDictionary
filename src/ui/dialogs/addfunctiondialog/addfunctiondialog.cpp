#include "ui/dialogs/addfunctiondialog/addfunctiondialog.h"
#include "core/database/databasemanager.h"
#include "common/logger/logger.h"
#include <QMessageBox>

AddFunctionDialog::AddFunctionDialog(QWidget* parent)
    : QDialog(parent)
    , m_keyEdit(nullptr)
    , m_valueEdit(nullptr)
    , m_projectCombo(nullptr)
    , m_acceptButton(nullptr)
    , m_cancelButton(nullptr) {
    setupUI();
    loadProjects();
}

void AddFunctionDialog::setupUI() {
    setWindowTitle("增加函数");
    setMinimumSize(500, 450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    m_projectCombo = new QComboBox(this);
    m_projectCombo->setPlaceholderText("请选择项目");
    formLayout->addRow("所属项目:", m_projectCombo);

    m_keyEdit = new QLineEdit(this);
    m_keyEdit->setPlaceholderText("请输入函数名称");
    formLayout->addRow("函数名称:", m_keyEdit);

    m_valueEdit = new QTextEdit(this);
    m_valueEdit->setPlaceholderText("请输入函数介绍（支持Markdown格式）");
    m_valueEdit->setMinimumHeight(200);
    formLayout->addRow("函数介绍:", m_valueEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_acceptButton = new QPushButton("确认", this);
    m_acceptButton->setObjectName("primaryButton");
    m_acceptButton->setProperty("primary", true);
    connect(m_acceptButton, &QPushButton::clicked, this, &AddFunctionDialog::onAcceptClicked);
    buttonLayout->addWidget(m_acceptButton);

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &AddFunctionDialog::onRejectClicked);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    Logger::instance().info("增加函数对话框初始化完成");
}

void AddFunctionDialog::loadProjects() {
    if (!m_projectCombo) {
        return;
    }

    m_projectCombo->clear();
    
    QVector<ProjectInfo> projects = DatabaseManager::instance().getAllProjects();
    
    ProjectInfo tempProject = DatabaseManager::instance().getOrCreateTemporaryProject();
    
    m_projectCombo->addItem("待整理", tempProject.id);
    
    for (const ProjectInfo& project : projects) {
        if (project.rootPath != "__temporary__") {
            m_projectCombo->addItem(project.name, project.id);
        }
    }
    
    if (m_projectCombo->count() > 0) {
        m_projectCombo->setCurrentIndex(0);
    }
}

QString AddFunctionDialog::getFunctionKey() const {
    if (m_keyEdit) {
        return m_keyEdit->text().trimmed();
    }
    return QString();
}

QString AddFunctionDialog::getFunctionValue() const {
    if (m_valueEdit) {
        return m_valueEdit->toPlainText();
    }
    return QString();
}

int AddFunctionDialog::getProjectId() const {
    if (m_projectCombo && m_projectCombo->currentIndex() >= 0) {
        return m_projectCombo->currentData().toInt();
    }
    return -1;
}

void AddFunctionDialog::setSelectedProject(int projectId) {
    if (!m_projectCombo) {
        return;
    }

    for (int i = 0; i < m_projectCombo->count(); ++i) {
        if (m_projectCombo->itemData(i).toInt() == projectId) {
            m_projectCombo->setCurrentIndex(i);
            return;
        }
    }
}

void AddFunctionDialog::onAcceptClicked() {
    QString key = getFunctionKey();
    QString value = getFunctionValue();

    if (key.isEmpty()) {
        QMessageBox::warning(this, "警告", "函数名称不能为空！");
        Logger::instance().warning("用户尝试添加空函数名称");
        return;
    }

    if (value.isEmpty()) {
        QMessageBox::warning(this, "警告", "函数介绍不能为空！");
        Logger::instance().warning("用户尝试添加空函数介绍");
        return;
    }

    Logger::instance().info("用户确认添加函数: " + key);
    accept();
}

void AddFunctionDialog::onRejectClicked() {
    Logger::instance().info("用户取消添加函数");
    reject();
}
