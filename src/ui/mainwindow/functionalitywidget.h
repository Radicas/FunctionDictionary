/**
 * @file functionalitywidget.h
 * @brief 功能型widget组件，协调子组件和服务
 * @author FunctionDB Team
 * @date 2026-03-05
 * @version 3.0
 * 
 * @details 更新说明：
 * - v3.0: 采用依赖注入模式，解耦数据库访问
 * - v2.0: 重构为协调者模式，职责分离
 */

#ifndef FUNCTIONALITYWIDGET_H
#define FUNCTIONALITYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>
#include "ui/mainwindow/widgets/fileselectorwidget.h"
#include "ui/mainwindow/widgets/progresswidget.h"
#include "ui/mainwindow/widgets/parsecontrolwidget.h"
#include "core/interfaces/iparseservice.h"
#include "core/interfaces/idatabaserepository.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"

class FunctionalityWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionalityWidget(IDatabaseManager* dbManager, IParseService* parseService, QWidget *parent = nullptr);
    ~FunctionalityWidget();

signals:
    /**
     * @brief 批量处理完成信号
     */
    void batchProcessingCompleted();

public slots:
    /**
     * @brief 刷新项目列表
     */
    void refreshProjectList();

private slots:
    /**
     * @brief 路径选择完成槽函数
     * @param path 选择的路径
     */
    void onPathSelected(const QString& path);

    /**
     * @brief 模式改变槽函数
     * @param mode 解析模式
     */
    void onModeChanged(ParseMode mode);

    /**
     * @brief 解析请求槽函数
     */
    void onParseRequested();

    /**
     * @brief 取消请求槽函数
     */
    void onCancelRequested();

    /**
     * @brief AI配置请求槽函数
     */
    void onAiConfigRequested();

    /**
     * @brief 解析完成槽函数
     * @param result 解析结果
     */
    void onParseComplete(const ParseResult& result);

    /**
     * @brief 解析进度槽函数
     * @param progress 进度信息
     */
    void onParseProgress(const ParseProgress& progress);

    /**
     * @brief 解析失败槽函数
     * @param error 错误信息
     */
    void onParseFailed(const QString& error);

    /**
     * @brief 解析取消完成槽函数
     */
    void onParseCancelled();

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 开始解析
     */
    void startParsing();

    /**
     * @brief 更新UI状态
     * @param isProcessing 是否正在处理
     */
    void updateUIState(bool isProcessing);

    /**
     * @brief 验证AI配置
     * @return 配置是否有效
     */
    bool validateAIConfig();

    /**
     * @brief 加载项目列表
     */
    void loadProjects();

    FileSelectorWidget *m_fileSelector;      ///< 文件选择器组件
    ProgressWidget *m_progressWidget;        ///< 进度显示组件
    ParseControlWidget *m_controlWidget;     ///< 解析控制组件
    QLabel *m_projectLabel;                  ///< 项目选择标签
    QComboBox *m_projectCombo;               ///< 项目选择下拉框
    
    IDatabaseManager* m_dbManager;           ///< 数据库管理器（依赖注入）
    IParseService* m_parseService;           ///< 解析服务（依赖注入）
    
    QVBoxLayout *m_mainLayout;               ///< 主布局
};

#endif // FUNCTIONALITYWIDGET_H
