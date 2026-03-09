/**
 * @file markdownview.h
 * @brief Markdown渲染视图组件，基于QWebEngineView实现完整的Markdown渲染功能
 * @author Developer
 * @date 2026-03-02
 * @version 1.0
 * 
 * @details 该组件提供以下功能：
 *          - 完整的GitHub Flavored Markdown (GFM)支持
 *          - 代码语法高亮（支持190+编程语言）
 *          - Mermaid图表渲染
 *          - 数学公式渲染（KaTeX）
 *          - 表格、图片等富媒体内容支持
 *          - 明暗主题切换
 */

#ifndef MARKDOWNVIEW_H
#define MARKDOWNVIEW_H

#include <QString>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWidget>

/**
 * @brief Markdown渲染视图类
 * 
 * 基于QWebEngineView实现的Markdown渲染组件，支持完整的GFM语法、
 * 代码高亮、Mermaid图表和数学公式。
 */
class MarkdownView : public QWebEngineView
{
    Q_OBJECT

   public:
    /**
     * @brief 主题类型枚举
     */
    enum class Theme
    {
        Light,  ///< 明亮主题
        Dark    ///< 暗黑主题
    };

    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MarkdownView(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MarkdownView() override;

    /**
     * @brief 设置Markdown内容
     * @param content Markdown格式的文本内容
     */
    void setMarkdown(const QString& content);

    /**
     * @brief 设置HTML内容
     * @param html HTML格式的文本内容
     */
    void setHtmlContent(const QString& html);

    /**
     * @brief 清除内容
     */
    void clear();

    /**
     * @brief 设置主题
     * @param theme 主题类型
     */
    void setTheme(Theme theme);

    /**
     * @brief 获取当前主题
     * @return 当前主题类型
     */
    Theme currentTheme() const;

    /**
     * @brief 滚动到指定锚点
     * @param anchor 锚点名称
     */
    void scrollToAnchor(const QString& anchor);

    /**
     * @brief 获取当前Markdown内容
     * @return 当前Markdown内容
     */
    QString markdown() const;

   signals:
    /**
     * @brief 链接点击信号
     * @param url 被点击的链接URL
     */
    void linkClicked(const QUrl& url);

    /**
     * @brief 内容加载完成信号
     * @param success 是否加载成功
     */
    void loadFinished(bool success);

   private slots:
    /**
     * @brief 页面加载完成槽函数
     * @param success 是否加载成功
     */
    void onLoadFinished(bool success);

   private:
    /**
     * @brief 初始化WebEngine视图
     */
    void initializeWebEngine();

    /**
     * @brief 加载HTML模板
     */
    void loadTemplate();

    /**
     * @brief 生成完整的HTML内容
     * @param markdown Markdown内容
     * @return 完整的HTML文档
     */
    QString generateHtml(const QString& markdown);

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 转义HTML特殊字符
     * @param text 原始文本
     * @return 转义后的文本
     */
    QString escapeHtml(const QString& text);

    Theme m_theme;             ///< 当前主题
    QString m_currentContent;  ///< 当前Markdown内容
    bool m_templateLoaded;     ///< 模板是否已加载
};

#endif  // MARKDOWNVIEW_H
