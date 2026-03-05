/**
 * @file addprojectdialog.h
 * @brief 添加项目对话框
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef ADDPROJECTDIALOG_H
#define ADDPROJECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include "core/models/projectinfo.h"

/**
 * @brief 添加项目对话框类
 */
class AddProjectDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit AddProjectDialog(QWidget* parent = nullptr);
    
    /**
     * @brief 获取项目信息
     * @return 项目信息
     */
    ProjectInfo getProjectInfo() const;

private slots:
    /**
     * @brief 浏览按钮点击槽函数
     */
    void onBrowseClicked();
    
    /**
     * @brief 确认按钮点击槽函数
     */
    void onConfirmClicked();

private:
    /**
     * @brief 初始化UI
     */
    void setupUI();
    
    /**
     * @brief 验证输入
     * @return 输入是否有效
     */
    bool validateInput();

    QLineEdit* m_nameEdit;              ///< 项目名称输入框
    QLineEdit* m_pathEdit;              ///< 项目路径输入框
    QTextEdit* m_descriptionEdit;       ///< 项目描述输入框
    QPushButton* m_browseButton;        ///< 浏览按钮
    QPushButton* m_confirmButton;       ///< 确认按钮
    QPushButton* m_cancelButton;        ///< 取消按钮
};

#endif
