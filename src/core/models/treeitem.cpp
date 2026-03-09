/**
 * @file treeitem.cpp
 * @brief 树节点数据结构实现
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#include "core/models/treeitem.h"

TreeItem::TreeItem(TreeItemType type, TreeItem* parent) : m_type(type), m_parent(parent) {}

TreeItem::~TreeItem()
{
    clearChildren();
}

TreeItem* TreeItem::child(int index)
{
    if (index < 0 || index >= m_children.size())
    {
        return nullptr;
    }
    return m_children.at(index);
}

int TreeItem::childCount() const
{
    return m_children.size();
}

int TreeItem::indexOfChild(TreeItem* child) const
{
    return m_children.indexOf(child);
}

TreeItem* TreeItem::parent() const
{
    return m_parent;
}

void TreeItem::appendChild(TreeItem* child)
{
    if (child)
    {
        child->m_parent = this;
        m_children.append(child);
    }
}

void TreeItem::insertChild(int index, TreeItem* child)
{
    if (child && index >= 0 && index <= m_children.size())
    {
        child->m_parent = this;
        m_children.insert(index, child);
    }
}

void TreeItem::removeChild(int index)
{
    if (index >= 0 && index < m_children.size())
    {
        TreeItem* child = m_children.takeAt(index);
        delete child;
    }
}

void TreeItem::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

TreeItemType TreeItem::type() const
{
    return m_type;
}

QString TreeItem::displayText() const
{
    return m_displayText;
}

void TreeItem::setDisplayText(const QString& text)
{
    m_displayText = text;
}

QString TreeItem::path() const
{
    return m_path;
}

void TreeItem::setPath(const QString& path)
{
    m_path = path;
}

FunctionData TreeItem::functionData() const
{
    return m_functionData;
}

void TreeItem::setFunctionData(const FunctionData& data)
{
    m_functionData = data;
    m_displayText = data.key;
}

ProjectInfo TreeItem::projectInfo() const
{
    return m_projectInfo;
}

void TreeItem::setProjectInfo(const ProjectInfo& info)
{
    m_projectInfo = info;
    m_displayText = info.name;
    m_path = info.rootPath;
}

int TreeItem::functionId() const
{
    return m_functionData.id;
}

int TreeItem::projectId() const
{
    return m_projectInfo.id;
}

bool TreeItem::matchesSearch(const QString& keyword) const
{
    if (keyword.isEmpty())
    {
        return true;
    }

    QString lowerKeyword = keyword.toLower();

    if (m_displayText.toLower().contains(lowerKeyword))
    {
        return true;
    }

    if (m_type == TreeItemType::Function)
    {
        if (m_functionData.value.toLower().contains(lowerKeyword))
        {
            return true;
        }
        if (m_functionData.filePath.toLower().contains(lowerKeyword))
        {
            return true;
        }
    }

    if (m_type == TreeItemType::File || m_type == TreeItemType::Directory)
    {
        if (m_path.toLower().contains(lowerKeyword))
        {
            return true;
        }
    }

    if (m_type == TreeItemType::Project)
    {
        if (m_projectInfo.name.toLower().contains(lowerKeyword))
        {
            return true;
        }
        if (m_projectInfo.rootPath.toLower().contains(lowerKeyword))
        {
            return true;
        }
        if (m_projectInfo.description.toLower().contains(lowerKeyword))
        {
            return true;
        }
    }

    return false;
}
