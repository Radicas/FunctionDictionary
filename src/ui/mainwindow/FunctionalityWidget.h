/**
 * @file FunctionalityWidget.h
 * @brief 功能型widget组件，集成设置、配置加载及文件解析等功能
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 */

#ifndef FUNCTIONALITYWIDGET_H
#define FUNCTIONALITYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFuture>
#include <QtConcurrent>
#include <QTimer>
#include <QThread>
#include <QApplication>
#include "aiservicemanager.h"
#include "databasemanager.h"
#include "aiconfigdialog.h"

class FunctionalityWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionalityWidget(QWidget *parent = nullptr);
    ~FunctionalityWidget();

private slots:
    void onFileSelectClicked();
    void onParseButtonClicked();
    void onParseFinished();

private:
    void setupUI();
    void setupFileParseSection();
    void setupSettingsSection();
    bool validateFile(const QString &filePath);
    void showStatusMessage(const QString &message, int duration = 3000);

    // 文件解析相关
    QLabel *m_parseTitle;
    QLineEdit *m_filePathEdit;
    QPushButton *m_fileSelectButton;
    QPushButton *m_parseButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // 配置相关
    QLabel *m_settingsTitle;
    QPushButton *m_aiConfigButton;

    QVBoxLayout *m_mainLayout;
    QFuture<void> m_parseFuture;
    QString m_currentFilePath;
};

#endif // FUNCTIONALITYWIDGET_H