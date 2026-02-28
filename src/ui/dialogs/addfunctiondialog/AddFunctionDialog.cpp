#include "addfunctiondialog.h"
#include "logger.h"
#include <QMessageBox>

AddFunctionDialog::AddFunctionDialog(QWidget* parent)
    : QDialog(parent)
    , m_keyEdit(nullptr)
    , m_valueEdit(nullptr)
    , m_acceptButton(nullptr)
    , m_cancelButton(nullptr) {
    setupUI();
}

void AddFunctionDialog::setupUI() {
    setWindowTitle("增加函数");
    setMinimumSize(500, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

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
