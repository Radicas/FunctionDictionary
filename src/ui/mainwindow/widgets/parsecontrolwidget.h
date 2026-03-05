/**
 * @file parsecontrolwidget.h
 * @brief 解析控制组件，负责解析流程的控制界面
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef PARSECONTROLWIDGET_H
#define PARSECONTROLWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

/**
 * @brief 解析控制组件类
 * 
 * 该组件负责：
 * - 开始/取消按钮UI
 * - 配置选项UI（递归扫描、跳过已存在）
 * - AI配置按钮
 */
class ParseControlWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit ParseControlWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ParseControlWidget();

    /**
     * @brief 设置解析状态
     * @param isParsing 是否正在解析
     */
    void setParsing(bool isParsing);

    /**
     * @brief 获取是否递归扫描
     * @return 是否递归扫描
     */
    bool isRecursive() const;

    /**
     * @brief 获取是否跳过已存在
     * @return 是否跳过已存在
     */
    bool skipExisting() const;

    /**
     * @brief 设置控件是否可用
     * @param enabled 是否可用
     */
    void setControlsEnabled(bool enabled);

    /**
     * @brief 设置递归扫描选项是否可用
     * @param enabled 是否可用
     */
    void setRecursiveEnabled(bool enabled);

signals:
    /**
     * @brief 解析请求信号
     */
    void parseRequested();

    /**
     * @brief 取消请求信号
     */
    void cancelRequested();

    /**
     * @brief AI配置请求信号
     */
    void aiConfigRequested();

private slots:
    /**
     * @brief 解析按钮点击槽函数
     */
    void onParseClicked();

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelClicked();

    /**
     * @brief AI配置按钮点击槽函数
     */
    void onAiConfigClicked();

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();

    QLabel *m_settingsTitle;           ///< 配置标题标签
    QPushButton *m_parseButton;        ///< 解析按钮
    QPushButton *m_cancelButton;       ///< 取消按钮
    QPushButton *m_aiConfigButton;     ///< AI配置按钮
    QCheckBox *m_recursiveCheckBox;    ///< 递归扫描复选框
    QCheckBox *m_skipExistingCheckBox; ///< 跳过已存在复选框
    
    bool m_isParsing;                  ///< 是否正在解析
};

#endif // PARSECONTROLWIDGET_H
