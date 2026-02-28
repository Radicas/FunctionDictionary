/**
 * @file aiconfigdialog.h
 * @brief AI配置对话框类，用于配置AI服务的各项参数
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef AICONFIGDIALOG_H
#define AICONFIGDIALOG_H

#include "../../../core/ai/aiconfigmanager.h"
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

/**
 * @brief AI配置对话框类
 */
class AIConfigDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit AIConfigDialog(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AIConfigDialog();

private slots:
    /**
     * @brief 保存按钮点击槽函数
     */
    void onSaveClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelClicked();

    /**
     * @brief 测试连接按钮点击槽函数
     */
    void onTestConnectionClicked();

private:
    /**
     * @brief 初始化界面
     */
    void setupUI();

    /**
     * @brief 加载配置到界面
     */
    void loadConfigToUI();

    /**
     * @brief 从界面获取配置
     * @return AI配置
     */
    AIConfig getConfigFromUI();

    QComboBox *m_providerComboBox;    ///< API提供商选择框
    QLineEdit *m_baseUrlLineEdit;      ///< Base URL输入框
    QLineEdit *m_apiKeyLineEdit;       ///< API Key输入框
    QLineEdit *m_modelIdLineEdit;      ///< Model ID输入框
    QPushButton *m_testButton;         ///< 测试连接按钮
    QPushButton *m_saveButton;         ///< 保存按钮
    QPushButton *m_cancelButton;       ///< 取消按钮
};

#endif // AICONFIGDIALOG_H
