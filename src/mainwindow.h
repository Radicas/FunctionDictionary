/**
 * @file mainwindow.h
 * @brief 主窗口类，函数数据库管理系统的主界面
 * @author Developer
 * @date 2026-02-27
 * @version 1.0
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QMap>
#include "databasemanager.h"

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    /**
     * @brief 函数列表项点击槽函数
     * @param item 列表项指针
     */
    void onFunctionItemClicked(QListWidgetItem* item);

    /**
     * @brief 增加按钮点击槽函数
     */
    void onAddButtonClicked();

    /**
     * @brief 删除按钮点击槽函数
     */
    void onDeleteButtonClicked();

private:
    /**
     * @brief 初始化界面
     */
    void setupUI();

    /**
     * @brief 加载函数列表
     */
    void loadFunctionList();

    /**
     * @brief 显示函数详情
     * @param functionData 函数数据
     */
    void displayFunctionDetail(const FunctionData& functionData);

    /**
     * @brief 将Markdown转换为HTML
     * @param markdown Markdown文本
     * @return HTML文本
     */
    QString markdownToHtml(const QString& markdown);

    QListWidget* m_functionList;       ///< 函数列表
    QTextBrowser* m_detailBrowser;     ///< 详情显示区域
    QPushButton* m_addButton;          ///< 增加按钮
    QPushButton* m_deleteButton;       ///< 删除按钮
    QMap<int, FunctionData> m_functionMap; ///< 函数数据映射（ID -> 数据）
    int m_currentFunctionId;            ///< 当前选中的函数ID
};

#endif // MAINWINDOW_H
