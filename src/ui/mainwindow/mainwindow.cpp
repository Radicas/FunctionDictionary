/**
 * @file mainwindow.cpp
 * @brief 主窗口类实现
 * @author Developer
 * @date 2026-02-27
 * @version 1.2
 */

#include "ui/mainwindow/mainwindow.h"
#include "ui/mainwindow/functionalitywidget.h"
#include "ui/dialogs/addfunctiondialog/addfunctiondialog.h"
#include "ui/dialogs/addprojectdialog/addprojectdialog.h"
#include "core/services/parseservice.h"
#include "core/models/treeitem.h"
#include "common/logger/logger.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSplitter>
#include <QRegularExpression>
#include <QAbstractItemView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_treeView(nullptr)
    , m_searchEdit(nullptr)
    , m_detailBrowser(nullptr)
    , m_functionalityWidget(nullptr)
    , m_parseService(nullptr)
    , m_addProjectButton(nullptr)
    , m_removeProjectButton(nullptr)
    , m_addFunctionButton(nullptr)
    , m_deleteFunctionButton(nullptr)
    , m_treeModel(nullptr)
    , m_proxyModel(nullptr)
    , m_currentFunctionId(-1)
    , m_currentProjectId(-1)
    , m_themeActionGroup(nullptr) {
    m_parseService = new ParseService(this);
    setupUI();
    loadTreeData();
    
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::onThemeChangedSignal);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("函数数据库管理系统");
    setMinimumSize(1000, 700);

    setupMenuBar();

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("mainSplitter");
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);

    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("搜索函数、文件、项目...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    leftLayout->addWidget(m_searchEdit);

    m_treeView = new QTreeView(this);
    m_treeView->setObjectName("functionTreeView");
    m_treeView->setHeaderHidden(true);
    m_treeView->setAnimated(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    setupTreeView();
    leftLayout->addWidget(m_treeView);

    QFrame* functionListPanel = createPanelFrame("函数列表", leftPanel, "functionListPanel");
    splitter->addWidget(functionListPanel);

    m_detailBrowser = new MarkdownView(this);
    m_detailBrowser->setObjectName("detailBrowser");
    QFrame* detailPanel = createPanelFrame("", m_detailBrowser, "detailPanel");
    splitter->addWidget(detailPanel);

    m_functionalityWidget = new FunctionalityWidget(m_parseService, this);
    connect(m_functionalityWidget, &FunctionalityWidget::batchProcessingCompleted,
            this, &MainWindow::onDataChanged);
    QFrame* functionalityPanel = createPanelFrame("功能操作", m_functionalityWidget, "functionalityPanel");
    splitter->addWidget(functionalityPanel);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 1);

    mainLayout->addWidget(splitter);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_addProjectButton = new QPushButton("添加项目", this);
    m_addProjectButton->setObjectName("primaryButton");
    m_addProjectButton->setProperty("primary", true);
    connect(m_addProjectButton, &QPushButton::clicked, this, &MainWindow::onAddProjectClicked);
    buttonLayout->addWidget(m_addProjectButton);

    m_removeProjectButton = new QPushButton("移除项目", this);
    m_removeProjectButton->setObjectName("secondaryButton");
    connect(m_removeProjectButton, &QPushButton::clicked, this, &MainWindow::onRemoveProjectClicked);
    buttonLayout->addWidget(m_removeProjectButton);

    m_addFunctionButton = new QPushButton("增加函数", this);
    m_addFunctionButton->setObjectName("primaryButton");
    m_addFunctionButton->setProperty("primary", true);
    connect(m_addFunctionButton, &QPushButton::clicked, this, &MainWindow::onAddFunctionClicked);
    buttonLayout->addWidget(m_addFunctionButton);

    m_deleteFunctionButton = new QPushButton("删除函数", this);
    m_deleteFunctionButton->setObjectName("dangerButton");
    m_deleteFunctionButton->setProperty("danger", true);
    connect(m_deleteFunctionButton, &QPushButton::clicked, this, &MainWindow::onDeleteFunctionClicked);
    buttonLayout->addWidget(m_deleteFunctionButton);

    mainLayout->addLayout(buttonLayout);

    Logger::instance().info("主窗口初始化完成");
}

void MainWindow::setupTreeView() {
    m_treeModel = new FunctionTreeModel(this);
    m_proxyModel = new FunctionTreeProxyModel(this);
    m_proxyModel->setSourceModel(m_treeModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    
    m_treeView->setModel(m_proxyModel);
    
    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onTreeItemClicked);
    connect(m_treeView, &QTreeView::doubleClicked, this, &MainWindow::onTreeItemDoubleClicked);
    connect(m_treeModel, &FunctionTreeModel::dataRefreshNeeded, this, &MainWindow::loadTreeData);
}

void MainWindow::loadTreeData() {
    QVector<ProjectInfo> projects = DatabaseManager::instance().getAllProjects();
    QVector<FunctionData> functions = DatabaseManager::instance().getAllFunctions();
    m_treeModel->setTreeData(projects, functions);
    m_treeView->expandAll();
    Logger::instance().info("树形数据已加载");
}

QFrame* MainWindow::createPanelFrame(const QString &title, QWidget *content, const QString &objectName) {
    QFrame* frame = new QFrame(this);
    frame->setObjectName(objectName);
    frame->setFrameShape(QFrame::StyledPanel);
    
    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    if (!title.isEmpty()) {
        QLabel* titleLabel = new QLabel(title, frame);
        titleLabel->setObjectName("panelTitleLabel");
        titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(titleLabel);
        
        QFrame* separator = new QFrame(frame);
        separator->setObjectName("panelSeparator");
        separator->setFrameShape(QFrame::HLine);
        separator->setFixedHeight(1);
        layout->addWidget(separator);
    }
    
    QWidget* contentContainer = new QWidget(frame);
    contentContainer->setObjectName("panelContent");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->addWidget(content);
    layout->addWidget(contentContainer);
    
    return frame;
}

void MainWindow::onTreeItemClicked(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    TreeItemType type = m_treeModel->itemType(sourceIndex);

    if (type == TreeItemType::Function) {
        FunctionData func = m_treeModel->getFunctionData(sourceIndex);
        m_currentFunctionId = func.id;
        displayFunctionDetail(func);
        Logger::instance().info("用户选中函数: " + func.key);
    } else if (type == TreeItemType::Project) {
        ProjectInfo project = m_treeModel->getProjectInfo(sourceIndex);
        m_currentProjectId = project.id;
        m_currentFunctionId = -1;
        m_detailBrowser->setMarkdown(QString("# %1\n\n**路径:** %2\n\n**描述:** %3")
                                     .arg(project.name)
                                     .arg(project.rootPath)
                                     .arg(project.description));
        Logger::instance().info("用户选中项目: " + project.name);
    } else {
        m_currentFunctionId = -1;
        QString path = m_treeModel->getNodePath(sourceIndex);
        m_detailBrowser->setMarkdown(QString("# %1\n\n**路径:** %2")
                                     .arg(index.data().toString())
                                     .arg(path));
    }
}

void MainWindow::onTreeItemDoubleClicked(const QModelIndex& index) {
    if (m_treeView->isExpanded(index)) {
        m_treeView->collapse(index);
    } else {
        m_treeView->expand(index);
    }
}

void MainWindow::onAddProjectClicked() {
    AddProjectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ProjectInfo project = dialog.getProjectInfo();
        if (DatabaseManager::instance().addProject(project)) {
            QMessageBox::information(this, "成功", "项目添加成功！");
            loadTreeData();
        } else {
            QMessageBox::critical(this, "错误", "项目添加失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::onRemoveProjectClicked() {
    if (m_currentProjectId <= 0) {
        QMessageBox::warning(this, "警告", "请先选择要移除的项目！");
        return;
    }

    ProjectInfo project = DatabaseManager::instance().getProjectById(m_currentProjectId);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认移除",
        QString("确定要移除项目 \"%1\" 吗？\n这将同时删除该项目下的所有函数数据！").arg(project.name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteProject(m_currentProjectId)) {
            QMessageBox::information(this, "成功", "项目移除成功！");
            m_currentProjectId = -1;
            m_currentFunctionId = -1;
            m_detailBrowser->clear();
            loadTreeData();
        } else {
            QMessageBox::critical(this, "错误", "项目移除失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::onAddFunctionClicked() {
    AddFunctionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString key = dialog.getFunctionKey();
        QString value = dialog.getFunctionValue();

        FunctionData func;
        func.key = key;
        func.value = value;
        func.projectId = m_currentProjectId > 0 ? m_currentProjectId : 0;

        if (DatabaseManager::instance().addFunction(func)) {
            QMessageBox::information(this, "成功", "函数添加成功！");
            loadTreeData();
        } else {
            QMessageBox::critical(this, "错误", "函数添加失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::onDeleteFunctionClicked() {
    if (m_currentFunctionId <= 0) {
        QMessageBox::warning(this, "警告", "请先选择要删除的函数！");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除选中的函数吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteFunction(m_currentFunctionId)) {
            QMessageBox::information(this, "成功", "函数删除成功！");
            m_currentFunctionId = -1;
            m_detailBrowser->clear();
            loadTreeData();
        } else {
            QMessageBox::critical(this, "错误", "函数删除失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    m_proxyModel->setSearchKeyword(text);
    if (!text.isEmpty()) {
        m_treeView->expandAll();
    }
}

void MainWindow::onDataChanged() {
    loadTreeData();
}

void MainWindow::displayFunctionDetail(const FunctionData& functionData) {
    QString content = QString("# %1\n\n").arg(functionData.key);
    content += QString("**创建时间:** %1\n\n").arg(functionData.createTime.toString("yyyy-MM-dd hh:mm:ss"));
    if (!functionData.filePath.isEmpty()) {
        content += QString("**文件路径:** %1\n\n").arg(functionData.filePath);
    }
    content += "---\n\n";
    content += functionData.value;

    m_detailBrowser->setMarkdown(content);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();

    QMenu* settingsMenu = menuBar->addMenu("设置");

    QAction* aiConfigAction = new QAction("AI配置", this);
    connect(aiConfigAction, &QAction::triggered, this, &MainWindow::onAIConfigClicked);
    settingsMenu->addAction(aiConfigAction);
    
    QAction* clearDataAction = new QAction("清空所有数据", this);
    connect(clearDataAction, &QAction::triggered, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "确认清空",
            "确定要清空所有数据吗？此操作不可恢复！",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            if (DatabaseManager::instance().clearAllData()) {
                QMessageBox::information(this, "成功", "数据已清空！");
                m_currentFunctionId = -1;
                m_currentProjectId = -1;
                m_detailBrowser->clear();
                loadTreeData();
            } else {
                QMessageBox::critical(this, "错误", "清空数据失败：" + DatabaseManager::instance().lastError());
            }
        }
    });
    settingsMenu->addAction(clearDataAction);

    QMenu* themeMenu = settingsMenu->addMenu("主题切换");
    
    m_themeActionGroup = new QActionGroup(this);
    m_themeActionGroup->setExclusive(true);

    QAction* industrialAction = new QAction("工业风复古", this);
    industrialAction->setCheckable(true);
    industrialAction->setData(static_cast<int>(ThemeType::Industrial));
    m_themeActionGroup->addAction(industrialAction);
    themeMenu->addAction(industrialAction);
    m_themeActions[ThemeType::Industrial] = industrialAction;

    QAction* cyberpunkAction = new QAction("赛博朋克", this);
    cyberpunkAction->setCheckable(true);
    cyberpunkAction->setData(static_cast<int>(ThemeType::Cyberpunk));
    m_themeActionGroup->addAction(cyberpunkAction);
    themeMenu->addAction(cyberpunkAction);
    m_themeActions[ThemeType::Cyberpunk] = cyberpunkAction;

    QAction* modernAction = new QAction("极简现代", this);
    modernAction->setCheckable(true);
    modernAction->setData(static_cast<int>(ThemeType::Modern));
    m_themeActionGroup->addAction(modernAction);
    themeMenu->addAction(modernAction);
    m_themeActions[ThemeType::Modern] = modernAction;

    connect(m_themeActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onThemeChanged);

    updateThemeMenuSelection(ThemeManager::instance().currentTheme());

    QMenu* helpMenu = menuBar->addMenu("帮助");

    QAction* aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutClicked);
    helpMenu->addAction(aboutAction);

    Logger::instance().info("菜单栏初始化完成");
}

void MainWindow::onAIConfigClicked() {
    AIConfigDialog dialog(this);
    dialog.exec();
    Logger::instance().info("用户打开AI配置对话框");
}

void MainWindow::onAboutClicked() {
    AboutDialog dialog(this);
    dialog.exec();
    Logger::instance().info("用户打开关于对话框");
}

void MainWindow::onThemeChanged(QAction* action) {
    if (!action) {
        return;
    }

    ThemeType theme = static_cast<ThemeType>(action->data().toInt());
    ThemeManager::instance().setTheme(theme);
    ThemeManager::instance().applyTheme(qApp);
    
    Logger::instance().info("用户切换主题");
}

void MainWindow::onThemeChangedSignal(ThemeType theme) {
    updateThemeMenuSelection(theme);
}

void MainWindow::updateThemeMenuSelection(ThemeType theme) {
    if (m_themeActions.contains(theme)) {
        m_themeActions[theme]->setChecked(true);
    }
}

void MainWindow::expandToIndex(const QModelIndex& index) {
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        m_treeView->expand(parent);
        parent = parent.parent();
    }
}
