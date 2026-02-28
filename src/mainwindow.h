/**
 * @file mainwindow.h
 * @brief 主窗口类，函数数据库管理系统的主界面
 * @author Developer
 * @date 2026-02-27
 * @version 1.0
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "databasemanager.h"
#include "aiconfigdialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

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

private:
  void setupUI();
  void setupMenuBar();
  void loadFunctionList();
  void displayFunctionDetail(const FunctionData &functionData);

  QListWidget *m_functionList;
  QTextBrowser *m_detailBrowser;
  QPushButton *m_addButton;
  QPushButton *m_deleteButton;
  QMap<int, FunctionData> m_functionMap;
  int m_currentFunctionId;
};

#endif
