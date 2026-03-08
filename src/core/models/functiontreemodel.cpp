/**
 * @file functiontreemodel.cpp
 * @brief 函数树形模型实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "core/models/functiontreemodel.h"
#include "common/logger/logger.h"
#include <QDir>
#include <QFileInfo>
#include <QMimeData>

FunctionTreeModel::FunctionTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootItem(nullptr)
{
    setupRootItem();
}

FunctionTreeModel::~FunctionTreeModel()
{
    delete m_rootItem;
}

void FunctionTreeModel::setupRootItem()
{
    m_rootItem = new TreeItem(TreeItemType::Root);
}

QModelIndex FunctionTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    
    TreeItem* parentItem = itemFromIndex(parent);
    TreeItem* childItem = parentItem->child(row);
    
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    
    return QModelIndex();
}

QModelIndex FunctionTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    
    TreeItem* childItem = itemFromIndex(index);
    TreeItem* parentItem = childItem->parent();
    
    if (parentItem == m_rootItem) {
        return QModelIndex();
    }
    
    return indexFromItem(parentItem);
}

int FunctionTreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem = itemFromIndex(parent);
    return parentItem->childCount();
}

int FunctionTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant FunctionTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    
    TreeItem* item = itemFromIndex(index);
    
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return item->displayText();
    }
    
    if (role == Qt::DecorationRole) {
        switch (item->type()) {
            case TreeItemType::Project:
                return QVariant::fromValue(QString("📁"));
            case TreeItemType::Directory:
                return QVariant::fromValue(QString("📂"));
            case TreeItemType::File:
                return QVariant::fromValue(QString("📄"));
            case TreeItemType::Function:
                return QVariant::fromValue(QString("🔧"));
            default:
                return QVariant();
        }
    }
    
    if (role == Qt::ToolTipRole) {
        QString tooltip = item->displayText();
        if (item->type() == TreeItemType::Function) {
            FunctionData func = item->functionData();
            if (!func.filePath.isEmpty()) {
                tooltip += "\n" + func.filePath;
            }
        } else if (item->type() == TreeItemType::File || item->type() == TreeItemType::Directory) {
            tooltip += "\n" + item->path();
        }
        return tooltip;
    }
    
    return QVariant();
}

QVariant FunctionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

void FunctionTreeModel::setTreeData(const QVector<ProjectInfo>& projects, const QVector<FunctionData>& functions)
{
    beginResetModel();
    m_projects = projects;
    m_functions = functions;
    delete m_rootItem;
    setupRootItem();
    buildTree();
    endResetModel();
    Logger::instance().info("函数树模型数据已更新");
}

void FunctionTreeModel::refresh()
{
    emit dataRefreshNeeded();
}

void FunctionTreeModel::buildTree()
{
    QMap<int, TreeItem*> projectItems;
    
    QVector<ProjectInfo> normalProjects;
    QVector<ProjectInfo> tempProjects;
    
    for (const ProjectInfo& project : m_projects) {
        if (project.rootPath == "__temporary__") {
            tempProjects.append(project);
        } else {
            normalProjects.append(project);
        }
    }
    
    for (const ProjectInfo& project : normalProjects) {
        TreeItem* projectItem = new TreeItem(TreeItemType::Project, m_rootItem);
        projectItem->setProjectInfo(project);
        m_rootItem->appendChild(projectItem);
        projectItems[project.id] = projectItem;
    }
    
    for (const ProjectInfo& project : tempProjects) {
        TreeItem* projectItem = new TreeItem(TreeItemType::Project, m_rootItem);
        projectItem->setProjectInfo(project);
        m_rootItem->appendChild(projectItem);
        projectItems[project.id] = projectItem;
    }
    
    QMap<int, QMap<QString, TreeItem*>> fileItems;
    
    for (const FunctionData& func : m_functions) {
        int projectId = func.projectId;
        
        TreeItem* projectItem = projectItems.value(projectId, nullptr);
        if (!projectItem) {
            TreeItem* uncatItem = projectItems.value(0, nullptr);
            if (!uncatItem) {
                ProjectInfo uncatProject;
                uncatProject.id = 0;
                uncatProject.name = "未分类";
                uncatProject.rootPath = "";
                uncatItem = new TreeItem(TreeItemType::Project, m_rootItem);
                uncatItem->setProjectInfo(uncatProject);
                m_rootItem->appendChild(uncatItem);
                projectItems[0] = uncatItem;
            }
            projectItem = uncatItem;
        }
        
        QString relativePath = func.filePath;
        if (relativePath.isEmpty()) {
            relativePath = "未分类";
        }
        
        TreeItem* fileItem = fileItems[projectId].value(relativePath, nullptr);
        if (!fileItem) {
            QStringList pathParts = relativePath.split('/', Qt::SkipEmptyParts);
            TreeItem* currentParent = projectItem;
            QString currentPath;
            
            for (int i = 0; i < pathParts.size(); ++i) {
                const QString& part = pathParts[i];
                if (currentPath.isEmpty()) {
                    currentPath = part;
                } else {
                    currentPath = currentPath + "/" + part;
                }
                
                bool isFile = (i == pathParts.size() - 1) && 
                              (part.contains('.') || part == "未分类");
                
                TreeItemType itemType = isFile ? TreeItemType::File : TreeItemType::Directory;
                
                bool found = false;
                for (int j = 0; j < currentParent->childCount(); ++j) {
                    TreeItem* child = currentParent->child(j);
                    if (child->displayText() == part) {
                        currentParent = child;
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    TreeItem* newItem = new TreeItem(itemType, currentParent);
                    newItem->setDisplayText(part);
                    newItem->setPath(currentPath);
                    currentParent->appendChild(newItem);
                    currentParent = newItem;
                    
                    if (isFile) {
                        fileItems[projectId][relativePath] = newItem;
                    }
                }
            }
            
            fileItem = currentParent;
        }
        
        TreeItem* funcItem = new TreeItem(TreeItemType::Function, fileItem);
        funcItem->setFunctionData(func);
        fileItem->appendChild(funcItem);
    }
    
    Logger::instance().info(QString("构建树完成: %1个项目, %2个函数")
                            .arg(m_projects.size()).arg(m_functions.size()));
}

QModelIndex FunctionTreeModel::indexFromItem(TreeItem* item) const
{
    if (!item || item == m_rootItem) {
        return QModelIndex();
    }
    
    TreeItem* parentItem = item->parent();
    if (!parentItem) {
        return QModelIndex();
    }
    
    int row = parentItem->indexOfChild(item);
    return createIndex(row, 0, item);
}

TreeItem* FunctionTreeModel::itemFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }
    
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    return item ? item : m_rootItem;
}

TreeItemType FunctionTreeModel::itemType(const QModelIndex& index) const
{
    TreeItem* item = itemFromIndex(index);
    return item->type();
}

FunctionData FunctionTreeModel::getFunctionData(const QModelIndex& index) const
{
    TreeItem* item = itemFromIndex(index);
    if (item->type() == TreeItemType::Function) {
        return item->functionData();
    }
    return FunctionData();
}

ProjectInfo FunctionTreeModel::getProjectInfo(const QModelIndex& index) const
{
    TreeItem* item = itemFromIndex(index);
    if (item->type() == TreeItemType::Project) {
        return item->projectInfo();
    }
    return ProjectInfo();
}

QString FunctionTreeModel::getNodePath(const QModelIndex& index) const
{
    TreeItem* item = itemFromIndex(index);
    return item->path();
}

QModelIndex FunctionTreeModel::findFunctionIndex(int functionId) const
{
    TreeItem* item = findFunctionItem(m_rootItem, functionId);
    if (item) {
        return indexFromItem(item);
    }
    return QModelIndex();
}

TreeItem* FunctionTreeModel::findFunctionItem(TreeItem* parent, int functionId) const
{
    for (int i = 0; i < parent->childCount(); ++i) {
        TreeItem* child = parent->child(i);
        if (child->type() == TreeItemType::Function && child->functionId() == functionId) {
            return child;
        }
        TreeItem* found = findFunctionItem(child, functionId);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

QModelIndex FunctionTreeModel::findProjectIndex(int projectId) const
{
    TreeItem* item = findProjectItem(m_rootItem, projectId);
    if (item) {
        return indexFromItem(item);
    }
    return QModelIndex();
}

TreeItem* FunctionTreeModel::findProjectItem(TreeItem* parent, int projectId) const
{
    for (int i = 0; i < parent->childCount(); ++i) {
        TreeItem* child = parent->child(i);
        if (child->type() == TreeItemType::Project && child->projectId() == projectId) {
            return child;
        }
    }
    return nullptr;
}

FunctionTreeProxyModel::FunctionTreeProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void FunctionTreeProxyModel::setSearchKeyword(const QString& keyword)
{
    m_searchKeyword = keyword;
    beginFilterChange();
    endFilterChange();
}

QString FunctionTreeProxyModel::searchKeyword() const
{
    return m_searchKeyword;
}

bool FunctionTreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_searchKeyword.isEmpty()) {
        return true;
    }
    
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item && item->matchesSearch(m_searchKeyword)) {
        return true;
    }
    
    return hasMatchingChildren(index);
}

bool FunctionTreeProxyModel::hasMatchingChildren(const QModelIndex& index) const
{
    int childCount = sourceModel()->rowCount(index);
    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRow(i, index)) {
            return true;
        }
    }
    return false;
}

Qt::DropActions FunctionTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList FunctionTreeModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-functiondata";
    return types;
}

QMimeData* FunctionTreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    
    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            TreeItem* item = itemFromIndex(index);
            if (item && item->type() == TreeItemType::Function) {
                FunctionData func = item->functionData();
                QString data = QString("%1|%2").arg(func.id).arg(func.key);
                encodedData.append(data.toUtf8());
                encodedData.append('\n');
            }
        }
    }
    
    mimeData->setData("application/x-functiondata", encodedData);
    return mimeData;
}

bool FunctionTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                      int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    
    if (action == Qt::IgnoreAction) {
        return true;
    }
    
    if (!data->hasFormat("application/x-functiondata")) {
        return false;
    }
    
    if (!parent.isValid()) {
        return false;
    }
    
    TreeItem* targetItem = itemFromIndex(parent);
    if (!targetItem) {
        return false;
    }
    
    int targetProjectId = -1;
    QString targetProjectName;
    
    if (targetItem->type() == TreeItemType::Project) {
        ProjectInfo project = targetItem->projectInfo();
        targetProjectId = project.id;
        targetProjectName = project.name;
    } else {
        TreeItem* current = targetItem;
        while (current && current->type() != TreeItemType::Project) {
            current = current->parent();
        }
        if (current) {
            ProjectInfo project = current->projectInfo();
            targetProjectId = project.id;
            targetProjectName = project.name;
        }
    }
    
    if (targetProjectId < 0) {
        return false;
    }
    
    QByteArray encodedData = data->data("application/x-functiondata");
    QString dataStr = QString::fromUtf8(encodedData);
    QStringList items = dataStr.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& item : items) {
        QStringList parts = item.split('|');
        if (parts.size() >= 2) {
            int functionId = parts[0].toInt();
            emit functionMoved(functionId, targetProjectId);
        }
    }
    
    Logger::instance().info(QString("拖拽函数到项目: %1").arg(targetProjectName));
    
    return true;
}
