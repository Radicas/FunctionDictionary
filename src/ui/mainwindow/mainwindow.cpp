#include "ui/mainwindow/mainwindow.h"
#include "ui/mainwindow/functionalitywidget.h"
#include "ui/dialogs/addfunctiondialog/addfunctiondialog.h"
#include "core/services/parseservice.h"
#include "common/logger/logger.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSplitter>
#include <QRegularExpression>
#include <QAbstractItemView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_functionList(nullptr)
    , m_detailBrowser(nullptr)
    , m_functionalityWidget(nullptr)
    , m_parseService(nullptr)
    , m_addButton(nullptr)
    , m_deleteButton(nullptr)
    , m_currentFunctionId(-1)
    , m_themeActionGroup(nullptr) {
    m_parseService = new ParseService(this);
    setupUI();
    loadFunctionList();
    
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::onThemeChangedSignal);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("函数数据库管理系统");
    setMinimumSize(900, 600);

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

    m_functionList = new QListWidget(this);
    m_functionList->setObjectName("functionList");
    m_functionList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_functionList, &QListWidget::itemClicked, this, &MainWindow::onFunctionItemClicked);
    QFrame* functionListPanel = createPanelFrame("函数列表", m_functionList, "functionListPanel");
    splitter->addWidget(functionListPanel);

    m_detailBrowser = new MarkdownView(this);
    m_detailBrowser->setObjectName("detailBrowser");
    QFrame* detailPanel = createPanelFrame("", m_detailBrowser, "detailPanel");
    splitter->addWidget(detailPanel);

    m_functionalityWidget = new FunctionalityWidget(m_parseService, this);
    connect(m_functionalityWidget, &FunctionalityWidget::batchProcessingCompleted,
            this, &MainWindow::loadFunctionList);
    QFrame* functionalityPanel = createPanelFrame("功能操作", m_functionalityWidget, "functionalityPanel");
    splitter->addWidget(functionalityPanel);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 1);

    mainLayout->addWidget(splitter);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_addButton = new QPushButton("增加", this);
    m_addButton->setObjectName("primaryButton");
    m_addButton->setProperty("primary", true);
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    buttonLayout->addWidget(m_addButton);

    m_deleteButton = new QPushButton("删除", this);
    m_deleteButton->setObjectName("dangerButton");
    m_deleteButton->setProperty("danger", true);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    buttonLayout->addWidget(m_deleteButton);

    mainLayout->addLayout(buttonLayout);

    Logger::instance().info("主窗口初始化完成");
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

void MainWindow::loadFunctionList() {
    m_functionList->clear();
    m_functionMap.clear();

    QVector<FunctionData> functions = DatabaseManager::instance().getAllFunctions();

    for (const FunctionData& func : functions) {
        QListWidgetItem* item = new QListWidgetItem(func.key);
        item->setData(Qt::UserRole, func.id);
        m_functionList->addItem(item);
        m_functionMap[func.id] = func;
    }

    Logger::instance().info("已加载 " + QString::number(functions.size()) + " 个函数");
}

void MainWindow::onFunctionItemClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    m_currentFunctionId = id;

    if (m_functionMap.contains(id)) {
        displayFunctionDetail(m_functionMap[id]);
    }

    Logger::instance().info("用户选中函数，ID: " + QString::number(id));
}

void MainWindow::displayFunctionDetail(const FunctionData& functionData) {
    QString content = QString("# %1\n\n").arg(functionData.key);
    content += QString("**创建时间:** %1\n\n").arg(functionData.createTime.toString("yyyy-MM-dd hh:mm:ss"));
    content += "---\n\n";
    content += functionData.value;

    m_detailBrowser->setMarkdown(content);
}

void MainWindow::onAddButtonClicked() {
    AddFunctionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString key = dialog.getFunctionKey();
        QString value = dialog.getFunctionValue();

        if (DatabaseManager::instance().addFunction(key, value)) {
            QMessageBox::information(this, "成功", "函数添加成功！");
            loadFunctionList();
        } else {
            QMessageBox::critical(this, "错误", "函数添加失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::onDeleteButtonClicked() {
    QList<QListWidgetItem*> selectedItems = m_functionList->selectedItems();

    if (selectedItems.isEmpty()) {
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
        QVector<int> ids;
        for (QListWidgetItem* item : selectedItems) {
            int id = item->data(Qt::UserRole).toInt();
            ids.append(id);
        }

        if (DatabaseManager::instance().deleteFunctions(ids)) {
            QMessageBox::information(this, "成功", "函数删除成功！");
            m_currentFunctionId = -1;
            m_detailBrowser->clear();
            loadFunctionList();
        } else {
            QMessageBox::critical(this, "错误", "函数删除失败：" + DatabaseManager::instance().lastError());
        }
    }
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();

    QMenu* settingsMenu = menuBar->addMenu("设置");

    QAction* aiConfigAction = new QAction("AI配置", this);
    connect(aiConfigAction, &QAction::triggered, this, &MainWindow::onAIConfigClicked);
    settingsMenu->addAction(aiConfigAction);

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
