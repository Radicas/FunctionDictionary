/**
 * @file mainwindow.h
 * @brief 主窗口类，函数数据库管理系统的主界面
 * @author FunctionDB Team
 * @date 2026-02-27
 * @version 2.0
 *
 * @details 更新说明：
 * - v2.0: 采用依赖注入模式，解耦数据库访问
 * - v1.2: 重构为树形结构，使用QTreeView替代QListWidget
 * - v1.1: 新增主题切换功能
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common/theme/thememanager.h"
#include "core/interfaces/idatabaserepository.h"
#include "core/interfaces/iparseservice.h"
#include "core/models/functiontreemodel.h"
#include "ui/dialogs/aboutdialog/aboutdialog.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "ui/mainwindow/functionalitywidget.h"
#include "ui/markdown/markdownview.h"

#include <QAction>
#include <QActionGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    explicit MainWindow(IDatabaseManager* dbManager, IParseService* parseService, QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    void onTreeItemClicked(const QModelIndex& index);
    void onTreeItemDoubleClicked(const QModelIndex& index);
    void onTreeViewContextMenu(const QPoint& pos);
    void onAddProjectClicked();
    void onRemoveProjectClicked();
    void onAddFunctionClicked();
    void onDeleteFunctionClicked();
    void onSearchTextChanged(const QString& text);
    void onAIConfigClicked();
    void onAboutClicked();
    void onThemeChanged(QAction* action);
    void onThemeChangedSignal(ThemeType theme);
    void onDataChanged();
    void onFunctionMoved(int functionId, int targetProjectId);

   private:
    void setupUI();
    void setupMenuBar();
    void setupTreeView();
    void loadTreeData();
    void displayFunctionDetail(const FunctionData& functionData);
    void updateThemeMenuSelection(ThemeType theme);
    void expandToIndex(const QModelIndex& index);

    QFrame* createPanelFrame(const QString& title, QWidget* content, const QString& objectName);

    QTreeView* m_treeView;                       // 函数树视图
    QLineEdit* m_searchEdit;                     // 搜索框
    MarkdownView* m_detailBrowser;               // 函数详情浏览器
    FunctionalityWidget* m_functionalityWidget;  // 功能模块组件
    IDatabaseManager* m_dbManager;               // 数据库管理器
    IParseService* m_parseService;               // 解析服务
    QPushButton* m_addProjectButton;             // 添加项目按钮
    QPushButton* m_removeProjectButton;          // 删除项目按钮
    QPushButton* m_addFunctionButton;            // 添加函数按钮
    QPushButton* m_deleteFunctionButton;         // 删除函数按钮

    FunctionTreeModel* m_treeModel;        // 函数树模型
    FunctionTreeProxyModel* m_proxyModel;  // 函数树代理模型

    int m_currentFunctionId;                   // 当前选中的函数ID
    int m_currentProjectId;                    // 当前选中的项目ID
    QActionGroup* m_themeActionGroup;          // 主题切换操作组
    QMap<ThemeType, QAction*> m_themeActions;  // 主题切换操作映射
};

#endif
