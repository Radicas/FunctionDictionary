/**
 * @file AboutDialog.cpp
 * @brief 关于对话框实现
 * @author Radica
 * @date 2026-02-28
 * @version 1.0.0
 */

#include "aboutdialog.h"
#include "versionmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

/**
 * @brief 构造函数
 * @param parent 父窗口
 */
AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
}

/**
 * @brief 析构函数
 */
AboutDialog::~AboutDialog() {
}

/**
 * @brief 设置UI界面
 */
void AboutDialog::setupUI() {
    setWindowTitle("关于");
    setMinimumSize(400, 250);
    setMaximumSize(500, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 创建项目说明部分
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setStyleSheet("font-size: 12px; color: #333;");
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setText("一个专为开发者设计的工具。\n帮助开发者快速存储和检索常用函数代码，提高开发效率。");
    mainLayout->addWidget(descriptionLabel);

    // 创建版本号信息
    VersionManager version(1, 0, 0);
    QLabel *versionLabel = new QLabel(QString("版本号：%1").arg(QString::fromStdString(version.toString())), this);
    versionLabel->setStyleSheet("font-size: 12px; color: #666;");
    versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(versionLabel);

    // 创建作者信息
    QLabel *authorLabel = new QLabel("作者：Radica", this);
    authorLabel->setStyleSheet("font-size: 12x; color: #666;");
    authorLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(authorLabel);

    mainLayout->addStretch();

    // 创建按钮部分
    QPushButton *closeButton = new QPushButton("关闭", this);
    closeButton->setMinimumWidth(100);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    setLayout(mainLayout);
}