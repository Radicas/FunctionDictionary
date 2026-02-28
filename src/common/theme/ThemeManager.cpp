/**
 * @file ThemeManager.cpp
 * @brief 主题管理器实现
 * @author Radica
 * @date 2026-02-28
 * @version 1.0.0
 */

#include "thememanager.h"
#include "logger.h"
#include <QFile>
#include <QDebug>

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : m_currentTheme(ThemeType::Light)
    , m_initialized(false) {
    initColorScheme();
}

ThemeManager::~ThemeManager() {
}

bool ThemeManager::init() {
    if (m_initialized) {
        return true;
    }

    m_styleSheet = loadStyleSheet(":/styles/theme.qss");
    if (m_styleSheet.isEmpty()) {
        Logger::instance().warning("主题管理器：无法加载样式表，使用默认样式");
        m_initialized = true;
        return true;
    }

    m_initialized = true;
    Logger::instance().info("主题管理器初始化完成");
    return true;
}

void ThemeManager::applyTheme(QApplication* app) {
    if (!app) {
        Logger::instance().error("主题管理器：QApplication指针为空");
        return;
    }

    if (!m_initialized) {
        init();
    }

    app->setStyleSheet(m_styleSheet);
    applyColorScheme();

    Logger::instance().info("主题已应用到应用程序");
}

void ThemeManager::setTheme(ThemeType theme) {
    if (m_currentTheme == theme) {
        return;
    }

    m_currentTheme = theme;
    initColorScheme();
    emit themeChanged(theme);

    Logger::instance().info("主题已切换为: " + QString(theme == ThemeType::Light ? "浅色" : 
                                                           theme == ThemeType::Dark ? "深色" : "系统"));
}

ThemeType ThemeManager::currentTheme() const {
    return m_currentTheme;
}

QString ThemeManager::loadStyleSheet(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        Logger::instance().error("主题管理器：无法打开样式文件: " + resourcePath);
        return QString();
    }

    QString styleSheet = QString::fromUtf8(file.readAll());
    file.close();

    return styleSheet;
}

QColor ThemeManager::themeColor(const QString& colorName) const {
    return m_colorScheme.value(colorName, QColor());
}

QString ThemeManager::currentStyleSheet() const {
    return m_styleSheet;
}

void ThemeManager::initColorScheme() {
    m_colorScheme.clear();

    if (m_currentTheme == ThemeType::Light) {
        m_colorScheme["primary"] = QColor("#2563EB");
        m_colorScheme["primaryHover"] = QColor("#1D4ED8");
        m_colorScheme["primaryPressed"] = QColor("#1E40AF");
        
        m_colorScheme["secondary"] = QColor("#64748B");
        m_colorScheme["secondaryHover"] = QColor("#475569");
        
        m_colorScheme["accent"] = QColor("#8B5CF6");
        
        m_colorScheme["background"] = QColor("#FFFFFF");
        m_colorScheme["backgroundSecondary"] = QColor("#F8FAFC");
        m_colorScheme["backgroundTertiary"] = QColor("#F1F5F9");
        
        m_colorScheme["surface"] = QColor("#FFFFFF");
        m_colorScheme["surfaceHover"] = QColor("#F1F5F9");
        
        m_colorScheme["textPrimary"] = QColor("#1E293B");
        m_colorScheme["textSecondary"] = QColor("#64748B");
        m_colorScheme["textMuted"] = QColor("#94A3B8");
        m_colorScheme["textOnPrimary"] = QColor("#FFFFFF");
        
        m_colorScheme["border"] = QColor("#E2E8F0");
        m_colorScheme["borderFocus"] = QColor("#2563EB");
        
        m_colorScheme["success"] = QColor("#10B981");
        m_colorScheme["warning"] = QColor("#F59E0B");
        m_colorScheme["error"] = QColor("#EF4444");
        m_colorScheme["info"] = QColor("#3B82F6");
        
        m_colorScheme["shadow"] = QColor(0, 0, 0, 25);
    } else {
        m_colorScheme["primary"] = QColor("#3B82F6");
        m_colorScheme["primaryHover"] = QColor("#60A5FA");
        m_colorScheme["primaryPressed"] = QColor("#2563EB");
        
        m_colorScheme["secondary"] = QColor("#94A3B8");
        m_colorScheme["secondaryHover"] = QColor("#CBD5E1");
        
        m_colorScheme["accent"] = QColor("#A78BFA");
        
        m_colorScheme["background"] = QColor("#0F172A");
        m_colorScheme["backgroundSecondary"] = QColor("#1E293B");
        m_colorScheme["backgroundTertiary"] = QColor("#334155");
        
        m_colorScheme["surface"] = QColor("#1E293B");
        m_colorScheme["surfaceHover"] = QColor("#334155");
        
        m_colorScheme["textPrimary"] = QColor("#F1F5F9");
        m_colorScheme["textSecondary"] = QColor("#94A3B8");
        m_colorScheme["textMuted"] = QColor("#64748B");
        m_colorScheme["textOnPrimary"] = QColor("#FFFFFF");
        
        m_colorScheme["border"] = QColor("#334155");
        m_colorScheme["borderFocus"] = QColor("#3B82F6");
        
        m_colorScheme["success"] = QColor("#34D399");
        m_colorScheme["warning"] = QColor("#FBBF24");
        m_colorScheme["error"] = QColor("#F87171");
        m_colorScheme["info"] = QColor("#60A5FA");
        
        m_colorScheme["shadow"] = QColor(0, 0, 0, 50);
    }
}

void ThemeManager::applyColorScheme() {
    QPalette palette;
    
    palette.setColor(QPalette::Window, m_colorScheme["background"]);
    palette.setColor(QPalette::WindowText, m_colorScheme["textPrimary"]);
    palette.setColor(QPalette::Base, m_colorScheme["surface"]);
    palette.setColor(QPalette::AlternateBase, m_colorScheme["backgroundSecondary"]);
    palette.setColor(QPalette::ToolTipBase, m_colorScheme["surface"]);
    palette.setColor(QPalette::ToolTipText, m_colorScheme["textPrimary"]);
    palette.setColor(QPalette::Text, m_colorScheme["textPrimary"]);
    palette.setColor(QPalette::Button, m_colorScheme["surface"]);
    palette.setColor(QPalette::ButtonText, m_colorScheme["textPrimary"]);
    palette.setColor(QPalette::BrightText, m_colorScheme["error"]);
    palette.setColor(QPalette::Link, m_colorScheme["primary"]);
    palette.setColor(QPalette::Highlight, m_colorScheme["primary"]);
    palette.setColor(QPalette::HighlightedText, m_colorScheme["textOnPrimary"]);
    
    qApp->setPalette(palette);
}
