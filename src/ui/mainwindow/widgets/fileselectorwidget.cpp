/**
 * @file fileselectorwidget.cpp
 * @brief 文件选择器组件实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "ui/mainwindow/widgets/fileselectorwidget.h"
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "common/logger/logger.h"

FileSelectorWidget::FileSelectorWidget(QWidget* parent) : QWidget(parent), m_currentMode(ParseMode::SingleFile)
{
    setupUI();
    Logger::instance().info("文件选择器组件初始化完成");
}

FileSelectorWidget::~FileSelectorWidget() {}

void FileSelectorWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    // 使用字体度量作为基准单位
    QFontMetrics fm(font());
    int baseUnit = fm.height();
    int spacing = qMax(6, baseUnit / 2);
    mainLayout->setSpacing(spacing);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 响应式最小尺寸
    setMinimumWidth(qMax(180, baseUnit * 12));
    setMinimumHeight(qMax(100, baseUnit * 8));

    m_modeLabel = new QLabel("解析模式", this);
    m_modeLabel->setObjectName("titleLabel");
    mainLayout->addWidget(m_modeLabel);

    m_modeComboBox = new QComboBox(this);
    m_modeComboBox->addItem("单文件", static_cast<int>(ParseMode::SingleFile));
    m_modeComboBox->addItem("文件夹", static_cast<int>(ParseMode::Folder));
    m_modeComboBox->setMinimumWidth(200);
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &FileSelectorWidget::onModeChanged);
    mainLayout->addWidget(m_modeComboBox);

    m_pathLabel = new QLabel("解析文件", this);
    m_pathLabel->setObjectName("titleLabel");
    mainLayout->addWidget(m_pathLabel);

    m_fileInfoLabel = new QLabel("未选择", this);
    m_fileInfoLabel->setObjectName("fileInfoLabel");
    m_fileInfoLabel->setMinimumHeight(20);
    m_fileInfoLabel->setWordWrap(true);
    mainLayout->addWidget(m_fileInfoLabel);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("请选择要解析的文件或文件夹");
    m_pathEdit->setReadOnly(true);
    m_pathEdit->setMinimumWidth(150);
    pathLayout->addWidget(m_pathEdit);

    m_selectButton = new QPushButton("选择", this);
    m_selectButton->setObjectName("fileSelectButton");
    // 使用相对尺寸
    int buttonMinWidth = qMax(50, baseUnit * 3);
    int buttonMinHeight = qMax(24, baseUnit + 6);
    m_selectButton->setMinimumWidth(buttonMinWidth);
    m_selectButton->setMinimumHeight(buttonMinHeight);
    connect(m_selectButton, &QPushButton::clicked, this, &FileSelectorWidget::onSelectClicked);
    pathLayout->addWidget(m_selectButton);

    mainLayout->addLayout(pathLayout);
    setLayout(mainLayout);
}

QString FileSelectorWidget::selectedPath() const
{
    return m_currentPath;
}

ParseMode FileSelectorWidget::currentMode() const
{
    return m_currentMode;
}

void FileSelectorWidget::setMode(ParseMode mode)
{
    m_currentMode = mode;
    m_modeComboBox->setCurrentIndex(static_cast<int>(mode));

    if (mode == ParseMode::SingleFile)
    {
        m_pathLabel->setText("解析文件");
    }
    else
    {
        m_pathLabel->setText("解析文件夹");
    }

    Logger::instance().info(QString("切换解析模式: %1").arg(mode == ParseMode::SingleFile ? "单文件" : "文件夹"));
}

void FileSelectorWidget::clearSelection()
{
    m_pathEdit->clear();
    m_fileInfoLabel->setText("未选择");
    m_currentPath.clear();
}

void FileSelectorWidget::setEnabled(bool enabled)
{
    m_modeComboBox->setEnabled(enabled);
    m_selectButton->setEnabled(enabled);
    m_pathEdit->setEnabled(enabled);
}

void FileSelectorWidget::onSelectClicked()
{
    Logger::instance().info("onSelectClicked 方法被调用");

    if (m_currentMode == ParseMode::SingleFile)
    {
        QFileDialog dialog(window(), "选择文件", QDir::homePath(),
                           "所有文件 (*.*);;C/C++文件;;Python文件;;Java文件;;JavaScript文件");
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);

        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selectedFiles = dialog.selectedFiles();
            if (!selectedFiles.isEmpty())
            {
                QString filePath = selectedFiles.first();
                Logger::instance().info("用户选择文件: " + filePath);
                m_pathEdit->setText(filePath);
                m_currentPath = filePath;
                updateFileInfo(filePath);
                emit pathSelected(filePath);
            }
        }
    }
    else
    {
        QFileDialog dialog(window(), "选择文件夹", QDir::homePath());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);

        if (dialog.exec() == QDialog::Accepted)
        {
            QStringList selectedDirs = dialog.selectedFiles();
            if (!selectedDirs.isEmpty())
            {
                QString folderPath = selectedDirs.first();
                Logger::instance().info("用户选择文件夹: " + folderPath);
                m_pathEdit->setText(folderPath);
                m_currentPath = folderPath;
                updateFileInfo(folderPath);
                emit pathSelected(folderPath);
            }
        }
    }
}

void FileSelectorWidget::onModeChanged(int index)
{
    m_currentMode = static_cast<ParseMode>(m_modeComboBox->itemData(index).toInt());

    if (m_currentMode == ParseMode::SingleFile)
    {
        m_pathLabel->setText("解析文件");
    }
    else
    {
        m_pathLabel->setText("解析文件夹");
    }

    clearSelection();
    emit modeChanged(m_currentMode);
}

bool FileSelectorWidget::validatePath(const QString& path)
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && (fileInfo.isFile() || fileInfo.isDir());
}

void FileSelectorWidget::updateFileInfo(const QString& path)
{
    QFileInfo fileInfo(path);

    if (fileInfo.isFile())
    {
        m_fileInfoLabel->setText(
            QString("文件: %1, 大小: %2 KB").arg(fileInfo.fileName()).arg(fileInfo.size() / 1024.0, 0, 'f', 2));
    }
    else if (fileInfo.isDir())
    {
        QDir dir(path);
        m_fileInfoLabel->setText(QString("文件夹: %1").arg(dir.dirName()));
    }
}
