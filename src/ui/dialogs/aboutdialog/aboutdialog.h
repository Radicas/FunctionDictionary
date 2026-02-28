/**
 * @file aboutdialog.h
 * @brief 关于对话框类
 * @author Radica
 * @date 2026-02-28
 * @version 1.0.0
 */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

/**
 * @class AboutDialog
 * @brief 关于对话框，显示项目说明、版本号和作者信息
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit AboutDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~AboutDialog();

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();
};

#endif // ABOUTDIALOG_H