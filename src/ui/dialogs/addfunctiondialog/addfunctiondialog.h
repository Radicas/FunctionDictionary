/**
 * @file addfunctiondialog.h
 * @brief 增加函数对话框，用于添加新的函数信息
 * @author FunctionDB Team
 * @date 2026-02-27
 * @version 2.0
 * 
 * @details 更新说明：
 * - v2.0: 采用依赖注入模式，解耦数据库访问
 */

#ifndef ADDFUNCTIONDIALOG_H
#define ADDFUNCTIONDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include "core/interfaces/idatabaserepository.h"

class AddFunctionDialog : public QDialog
{
    Q_OBJECT

   public:
    explicit AddFunctionDialog(IDatabaseManager* dbManager, QWidget* parent = nullptr);

    /**
     * @brief 获取输入的函数名称
     * @return 函数名称
     */
    QString getFunctionKey() const;

    /**
     * @brief 获取输入的函数介绍
     * @return 函数介绍（Markdown格式）
     */
    QString getFunctionValue() const;

    /**
     * @brief 获取选中的项目ID
     * @return 项目ID，如果未选中则返回-1
     */
    int getProjectId() const;

    /**
     * @brief 设置默认选中的项目
     * @param projectId 项目ID
     */
    void setSelectedProject(int projectId);

   private slots:
    /**
     * @brief 确认按钮点击槽函数
     */
    void onAcceptClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onRejectClicked();

   private:
    /**
     * @brief 初始化界面
     */
    void setupUI();

    /**
     * @brief 加载项目列表
     */
    void loadProjects();

    IDatabaseManager* m_dbManager;  ///< 数据库管理器（依赖注入）
    QLineEdit* m_keyEdit;           ///< 函数名称输入框
    QTextEdit* m_valueEdit;         ///< 函数介绍文本框
    QComboBox* m_projectCombo;      ///< 项目选择下拉框
    QPushButton* m_acceptButton;    ///< 确认按钮
    QPushButton* m_cancelButton;    ///< 取消按钮
};

#endif  // ADDFUNCTIONDIALOG_H
