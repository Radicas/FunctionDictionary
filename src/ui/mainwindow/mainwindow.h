/**
 * @file mainwindow.h
 * @brief 主窗口类，函数数据库管理系统的主界面
 * @author Developer
 * @date 2026-02-27
 * @version 1.2
 * 
 * 更新说明：
 * - v1.2: 重构为树形结构，使用QTreeView替代QListWidget
 * - v1.1: 新增主题切换功能
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/database/databasemanager.h"
#include "core/interfaces/iparseservice.h"
#include "core/models/functiontreemodel.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "ui/dialogs/aboutdialog/aboutdialog.h"
#include "ui/mainwindow/functionalitywidget.h"
#include "ui/markdown/markdownview.h"
#include "common/theme/thememanager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFrame>
#include <QActionGroup>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onTreeItemClicked(const QModelIndex &index);
  void onTreeItemDoubleClicked(const QModelIndex &index);
  void onAddProjectClicked();
  void onRemoveProjectClicked();
  void onAddFunctionClicked();
  void onDeleteFunctionClicked();
  void onSearchTextChanged(const QString &text);
  void onAIConfigClicked();
  void onAboutClicked();
  void onThemeChanged(QAction* action);
  void onThemeChangedSignal(ThemeType theme);
  void onDataChanged();

private:
  void setupUI();
  void setupMenuBar();
  void setupTreeView();
  void loadTreeData();
  void displayFunctionDetail(const FunctionData &functionData);
  void updateThemeMenuSelection(ThemeType theme);
  void expandToIndex(const QModelIndex &index);
  
  QFrame* createPanelFrame(const QString &title, QWidget *content, const QString &objectName);

  QTreeView *m_treeView;
  QLineEdit *m_searchEdit;
  MarkdownView *m_detailBrowser;
  FunctionalityWidget *m_functionalityWidget;
  IParseService *m_parseService;
  QPushButton *m_addProjectButton;
  QPushButton *m_removeProjectButton;
  QPushButton *m_addFunctionButton;
  QPushButton *m_deleteFunctionButton;
  
  FunctionTreeModel *m_treeModel;
  FunctionTreeProxyModel *m_proxyModel;
  
  int m_currentFunctionId;
  int m_currentProjectId;
  QActionGroup *m_themeActionGroup;
  QMap<ThemeType, QAction*> m_themeActions;
};

#endif
