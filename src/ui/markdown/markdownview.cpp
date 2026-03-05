#include "ui/markdown/markdownview.h"
#include "common/logger/logger.h"
#include <QFile>
#include <QUrl>
#include <QRegularExpression>
#include <QTimer>
#include <QWebEngineSettings>

MarkdownView::MarkdownView(QWidget *parent)
    : QWebEngineView(parent)
    , m_theme(Theme::Light)
    , m_templateLoaded(false) {
    initializeWebEngine();
    connectSignals();
    loadTemplate();
    Logger::instance().info("MarkdownView 初始化完成");
}

MarkdownView::~MarkdownView() {
    Logger::instance().info("MarkdownView 析构");
}

void MarkdownView::initializeWebEngine() {
    QWebEnginePage *page = this->page();
    
    page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    page->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    page->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    
    setContextMenuPolicy(Qt::NoContextMenu);
}

void MarkdownView::connectSignals() {
    connect(this, &QWebEngineView::loadFinished, this, &MarkdownView::onLoadFinished);
}

void MarkdownView::loadTemplate() {
    QString resourcePath = ":/markdown/resources/template.html";
    
    Logger::instance().debug("尝试加载 Markdown 模板，路径: " + resourcePath);
    
    QFile templateFile(resourcePath);
    
    if (!templateFile.exists()) {
        Logger::instance().error("Markdown 模板文件不存在: " + resourcePath);
        Logger::instance().error("可能原因: 1) 资源文件未正确编译 2) .qrc 文件配置错误 3) CMAKE_AUTORCC 未启用");
        QString fallbackHtml = generateHtml("");
        setHtml(fallbackHtml);
        return;
    }
    
    if (templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = templateFile.readAll();
        templateFile.close();
        
        if (data.isEmpty()) {
            Logger::instance().error("Markdown 模板文件为空: " + resourcePath);
            QString fallbackHtml = generateHtml("");
            setHtml(fallbackHtml);
            return;
        }
        
        QString htmlTemplate = QString::fromUtf8(data);
        setHtml(htmlTemplate, QUrl("qrc:/markdown/"));
        Logger::instance().info("Markdown 模板加载成功，大小: " + QString::number(data.size()) + " 字节");
    } else {
        Logger::instance().error("无法打开 Markdown 模板文件: " + resourcePath);
        Logger::instance().error("错误码: " + QString::number(templateFile.error()));
        Logger::instance().error("错误信息: " + templateFile.errorString());
        QString fallbackHtml = generateHtml("");
        setHtml(fallbackHtml);
    }
}

void MarkdownView::setMarkdown(const QString &content) {
    m_currentContent = content;
    
    if (!m_templateLoaded) {
        loadTemplate();
        QTimer::singleShot(100, [this, content]() {
            QString escapedContent = escapeHtml(content);
            QString js = QString("renderMarkdown(`%1`)").arg(escapedContent);
            page()->runJavaScript(js);
        });
    } else {
        QString escapedContent = escapeHtml(content);
        QString js = QString("renderMarkdown(`%1`)").arg(escapedContent);
        page()->runJavaScript(js);
    }
    
    Logger::instance().debug("设置 Markdown 内容，长度: " + QString::number(content.length()));
}

void MarkdownView::setHtmlContent(const QString &html) {
    QString fullHtml = generateHtml(html);
    setHtml(fullHtml, QUrl("qrc:/markdown/"));
}

void MarkdownView::clear() {
    m_currentContent.clear();
    page()->runJavaScript("document.getElementById('content').innerHTML = ''");
}

void MarkdownView::setTheme(Theme theme) {
    m_theme = theme;
    
    QString themeName = (theme == Theme::Dark) ? "dark" : "light";
    QString js = QString("setTheme('%1')").arg(themeName);
    page()->runJavaScript(js);
    
    Logger::instance().info("切换 Markdown 主题: " + themeName);
}

MarkdownView::Theme MarkdownView::currentTheme() const {
    return m_theme;
}

void MarkdownView::scrollToAnchor(const QString &anchor) {
    QString js = QString("document.getElementById('%1').scrollIntoView({behavior: 'smooth'})").arg(anchor);
    page()->runJavaScript(js);
}

QString MarkdownView::markdown() const {
    return m_currentContent;
}

void MarkdownView::onLoadFinished(bool success) {
    m_templateLoaded = success;
    
    if (success && !m_currentContent.isEmpty()) {
        QString escapedContent = escapeHtml(m_currentContent);
        QString js = QString("renderMarkdown(`%1`)").arg(escapedContent);
        page()->runJavaScript(js);
    }
    
    emit loadFinished(success);
    Logger::instance().debug("页面加载完成: " + QString(success ? "成功" : "失败"));
}

QString MarkdownView::generateHtml(const QString &markdown) {
    QString escapedMarkdown = escapeHtml(markdown);
    
    QString html = QString(R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        :root {
            --bg-color: #ffffff;
            --text-color: #24292f;
            --code-bg: #f6f8fa;
            --border-color: #d0d7de;
            --link-color: #0969da;
        }
        
        @media (prefers-color-scheme: dark) {
            :root {
                --bg-color: #0d1117;
                --text-color: #c9d1d9;
                --code-bg: #161b22;
                --border-color: #30363d;
                --link-color: #58a6ff;
            }
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Noto Sans', Helvetica, Arial, sans-serif;
            font-size: 14px;
            line-height: 1.6;
            color: var(--text-color);
            background-color: var(--bg-color);
            padding: 16px;
        }
        
        .markdown-body {
            max-width: 100%;
        }
        
        h1, h2, h3, h4, h5, h6 {
            margin-top: 24px;
            margin-bottom: 16px;
            font-weight: 600;
            line-height: 1.25;
        }
        
        h1 { font-size: 2em; border-bottom: 1px solid var(--border-color); padding-bottom: .3em; }
        h2 { font-size: 1.5em; border-bottom: 1px solid var(--border-color); padding-bottom: .3em; }
        h3 { font-size: 1.25em; }
        h4 { font-size: 1em; }
        h5 { font-size: .875em; }
        h6 { font-size: .85em; color: #57606a; }
        
        p {
            margin-bottom: 16px;
        }
        
        a {
            color: var(--link-color);
            text-decoration: none;
        }
        
        a:hover {
            text-decoration: underline;
        }
        
        code {
            padding: .2em .4em;
            margin: 0;
            font-size: 85%;
            background-color: var(--code-bg);
            border-radius: 6px;
            font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace;
        }
        
        pre {
            padding: 16px;
            overflow: auto;
            font-size: 85%;
            line-height: 1.45;
            background-color: var(--code-bg);
            border-radius: 6px;
            margin-bottom: 16px;
        }
        
        pre code {
            padding: 0;
            background-color: transparent;
        }
        
        blockquote {
            padding: 0 1em;
            color: #57606a;
            border-left: .25em solid var(--border-color);
            margin-bottom: 16px;
        }
        
        ul, ol {
            padding-left: 2em;
            margin-bottom: 16px;
        }
        
        li {
            margin-top: .25em;
        }
        
        table {
            border-spacing: 0;
            border-collapse: collapse;
            margin-bottom: 16px;
            width: 100%;
        }
        
        table th, table td {
            padding: 6px 13px;
            border: 1px solid var(--border-color);
        }
        
        table th {
            font-weight: 600;
            background-color: var(--code-bg);
        }
        
        table tr:nth-child(2n) {
            background-color: var(--code-bg);
        }
        
        img {
            max-width: 100%;
            height: auto;
        }
        
        hr {
            height: .25em;
            padding: 0;
            margin: 24px 0;
            background-color: var(--border-color);
            border: 0;
        }
        
        .mermaid {
            text-align: center;
            margin: 16px 0;
        }
    </style>
</head>
<body>
    <div class="markdown-body" id="content">%1</div>
    <script>
        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }
        
        function renderMarkdown(content) {
            document.getElementById('content').innerHTML = content;
        }
        
        function setTheme(theme) {
            document.documentElement.setAttribute('data-theme', theme);
        }
    </script>
</body>
</html>)").arg(escapedMarkdown);
    
    return html;
}

QString MarkdownView::escapeHtml(const QString &text) {
    QString result = text;
    result.replace("\\", "\\\\");
    result.replace("`", "\\`");
    result.replace("$", "\\$");
    return result;
}
