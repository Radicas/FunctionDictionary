/**
 * @file fileselectorwidget.h
 * @brief 文件选择器组件，负责文件和文件夹的选择
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef FILESELECTORWIDGET_H
#define FILESELECTORWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

/**
 * @brief 解析模式枚举
 */
enum class ParseMode {
    SingleFile,     ///< 单文件模式
    Folder          ///< 文件夹模式
};

/**
 * @brief 文件选择器组件类
 * 
 * 该组件负责：
 * - 提供文件/文件夹选择UI
 * - 路径验证
 * - 模式切换（单文件/文件夹）
 */
class FileSelectorWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit FileSelectorWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~FileSelectorWidget();

    /**
     * @brief 获取当前选择的路径
     * @return 选择的文件或文件夹路径
     */
    QString selectedPath() const;

    /**
     * @brief 获取当前解析模式
     * @return 解析模式
     */
    ParseMode currentMode() const;

    /**
     * @brief 设置解析模式
     * @param mode 解析模式
     */
    void setMode(ParseMode mode);

    /**
     * @brief 清除选择
     */
    void clearSelection();

    /**
     * @brief 设置控件是否可用
     * @param enabled 是否可用
     */
    void setEnabled(bool enabled);

signals:
    /**
     * @brief 路径选择完成信号
     * @param path 选择的路径
     */
    void pathSelected(const QString& path);

    /**
     * @brief 模式改变信号
     * @param mode 新的解析模式
     */
    void modeChanged(ParseMode mode);

private slots:
    /**
     * @brief 选择按钮点击槽函数
     */
    void onSelectClicked();

    /**
     * @brief 模式改变槽函数
     * @param index 模式索引
     */
    void onModeChanged(int index);

private:
    /**
     * @brief 设置UI界面
     */
    void setupUI();

    /**
     * @brief 验证路径是否有效
     * @param path 路径
     * @return 路径是否有效
     */
    bool validatePath(const QString &path);

    /**
     * @brief 更新文件信息显示
     * @param path 文件或文件夹路径
     */
    void updateFileInfo(const QString &path);

    QLabel *m_modeLabel;               ///< 模式标签
    QComboBox *m_modeComboBox;         ///< 模式选择下拉框
    QLabel *m_pathLabel;               ///< 路径标签
    QLabel *m_fileInfoLabel;           ///< 文件信息标签
    QLineEdit *m_pathEdit;             ///< 路径输入框
    QPushButton *m_selectButton;       ///< 选择按钮
    
    QString m_currentPath;             ///< 当前选择的路径
    ParseMode m_currentMode;           ///< 当前解析模式
};

#endif // FILESELECTORWIDGET_H
