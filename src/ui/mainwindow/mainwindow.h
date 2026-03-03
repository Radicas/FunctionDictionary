/**
 * @file mainwindow.h
 * @brief 主窗口类，函数数据库管理系统的主界面
 * @author Developer
 * @date 2026-02-27
 * @version 1.1
 * 
 * 更新说明：
 * - v1.1: 新增主题切换功能
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/database/databasemanager.h"
#include "ui/dialogs/aiconfigdialog/aiconfigdialog.h"
#include "ui/dialogs/aboutdialog/aboutdialog.h"
#include "FunctionalityWidget.h"
#include "ui/markdown/markdownview.h"
#include "common/theme/thememanager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
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
  void onFunctionItemClicked(QListWidgetItem *item);
  void onAddButtonClicked();
  void onDeleteButtonClicked();
  void onAIConfigClicked();
  void onAboutClicked();
  void onThemeChanged(QAction* action);
  void onThemeChangedSignal(ThemeType theme);

private:
  void setupUI();
  void setupMenuBar();
  void loadFunctionList();
  void displayFunctionDetail(const FunctionData &functionData);
  void updateThemeMenuSelection(ThemeType theme);
  
  QFrame* createPanelFrame(const QString &title, QWidget *content, const QString &objectName);

  QListWidget *m_functionList;
  MarkdownView *m_detailBrowser;
  FunctionalityWidget *m_functionalityWidget;
  QPushButton *m_addButton;
  QPushButton *m_deleteButton;
  QMap<int, FunctionData> m_functionMap;
  int m_currentFunctionId;
  QActionGroup *m_themeActionGroup;
  QMap<ThemeType, QAction*> m_themeActions;
};

#endif
