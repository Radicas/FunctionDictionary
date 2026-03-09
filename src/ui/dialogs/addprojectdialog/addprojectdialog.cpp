/**
 * @file addprojectdialog.cpp
 * @brief 添加项目对话框实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "ui/dialogs/addprojectdialog/addprojectdialog.h"
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include "common/logger/logger.h"

AddProjectDialog::AddProjectDialog(QWidget* parent)
    : QDialog(parent),
      m_nameEdit(nullptr),
      m_pathEdit(nullptr),
      m_descriptionEdit(nullptr),
      m_browseButton(nullptr),
      m_confirmButton(nullptr),
      m_cancelButton(nullptr)
{
    setupUI();
    setWindowTitle("添加项目");
    setMinimumSize(500, 300);
}

void AddProjectDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("请输入项目名称");
    formLayout->addRow("项目名称:", m_nameEdit);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("请选择项目根目录");
    m_browseButton = new QPushButton("浏览...", this);
    connect(m_browseButton, &QPushButton::clicked, this, &AddProjectDialog::onBrowseClicked);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_browseButton);
    formLayout->addRow("项目路径:", pathLayout);

    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText("请输入项目描述（可选）");
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("项目描述:", m_descriptionEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_confirmButton = new QPushButton("确定", this);
    m_confirmButton->setObjectName("primaryButton");
    connect(m_confirmButton, &QPushButton::clicked, this, &AddProjectDialog::onConfirmClicked);
    buttonLayout->addWidget(m_confirmButton);

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void AddProjectDialog::onBrowseClicked()
{
    QFileDialog dialog(this, "选择项目根目录", m_pathEdit->text().isEmpty() ? QDir::homePath() : m_pathEdit->text());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setOption(QFileDialog::DontResolveSymlinks, true);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList selectedDirs = dialog.selectedFiles();
        if (!selectedDirs.isEmpty())
        {
            QString dir = selectedDirs.first();
            m_pathEdit->setText(dir);

            if (m_nameEdit->text().isEmpty())
            {
                QDir d(dir);
                m_nameEdit->setText(d.dirName());
            }
        }
    }
}

void AddProjectDialog::onConfirmClicked()
{
    if (validateInput())
    {
        accept();
    }
}

bool AddProjectDialog::validateInput()
{
    if (m_nameEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入项目名称！");
        m_nameEdit->setFocus();
        return false;
    }

    if (m_pathEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请选择项目根目录！");
        m_browseButton->setFocus();
        return false;
    }

    QDir dir(m_pathEdit->text().trimmed());
    if (!dir.exists())
    {
        QMessageBox::warning(this, "警告", "所选目录不存在！");
        m_pathEdit->setFocus();
        return false;
    }

    return true;
}

ProjectInfo AddProjectDialog::getProjectInfo() const
{
    ProjectInfo project;
    project.name = m_nameEdit->text().trimmed();
    project.rootPath = m_pathEdit->text().trimmed();
    project.description = m_descriptionEdit->toPlainText().trimmed();
    return project;
}
