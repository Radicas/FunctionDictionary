/**
 * @file aiconfigdialog.h
 * @brief AI配置对话框类，用于配置AI服务的各项参数，支持多配置管理
 * @author Developer
 * @date 2026-02-28
 * @version 2.0
 */

#ifndef AICONFIGDIALOG_H
#define AICONFIGDIALOG_H

#include "core/ai/aiconfigmanager.h"
#include "core/ai/modellistfetcher.h"
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>

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
     * @brief 配置选择变化槽函数
     * @param currentRow 当前行
     */
    void onConfigSelectionChanged(int currentRow);

    /**
     * @brief 新建配置按钮点击槽函数
     */
    void onNewConfigClicked();

    /**
     * @brief 删除配置按钮点击槽函数
     */
    void onDeleteConfigClicked();

    /**
     * @brief 保存按钮点击槽函数
     */
    void onSaveClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelClicked();

    /**
     * @brief 获取模型列表按钮点击槽函数
     */
    void onFetchModelsClicked();

    /**
     * @brief 模型列表获取成功槽函数
     * @param models 模型列表
     */
    void onModelsFetched(const QStringList& models);

    /**
     * @brief 模型列表获取失败槽函数
     * @param error 错误信息
     */
    void onFetchFailed(const QString& error);

    /**
     * @brief Base URL或API Key变化槽函数
     */
    void onCredentialsChanged();

private:
    /**
     * @brief 初始化界面
     */
    void setupUI();

    /**
     * @brief 加载配置列表
     */
    void loadConfigList();

    /**
     * @brief 加载配置到界面
     * @param config 配置对象
     */
    void loadConfigToUI(const AIConfig& config);

    /**
     * @brief 从界面获取配置
     * @return AI配置
     */
    AIConfig getConfigFromUI();

    /**
     * @brief 清空界面输入
     */
    void clearUI();

    /**
     * @brief 更新按钮状态
     */
    void updateButtonStates();

    /**
     * @brief 验证当前输入
     * @return 是否有效
     */
    bool validateInput();

    QListWidget *m_configListWidget;    ///< 配置列表控件
    QPushButton *m_newConfigButton;     ///< 新建配置按钮
    QPushButton *m_deleteConfigButton;  ///< 删除配置按钮

    QLineEdit *m_configNameLineEdit;    ///< 配置名称输入框
    QComboBox *m_providerComboBox;      ///< API提供商选择框
    QLineEdit *m_baseUrlLineEdit;       ///< Base URL输入框
    QLineEdit *m_apiKeyLineEdit;        ///< API Key输入框
    QComboBox *m_modelComboBox;         ///< 模型选择下拉框
    QPushButton *m_fetchModelsButton;   ///< 获取模型列表按钮

    QPushButton *m_saveButton;          ///< 保存按钮
    QPushButton *m_cancelButton;        ///< 取消按钮

    QString m_currentConfigName;        ///< 当前编辑的配置名称
    bool m_isNewConfig;                 ///< 是否为新建配置
    bool m_isLoading;                   ///< 是否正在加载配置
};

#endif
