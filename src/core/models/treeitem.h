/**
 * @file treeitem.h
 * @brief 树节点数据结构定义，用于QAbstractItemModel
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QString>
#include <QVariant>
#include "core/models/functiondata.h"
#include "core/models/projectinfo.h"

/**
 * @brief 树节点类型枚举
 */
enum class TreeItemType
{
    Root,       ///< 根节点
    Project,    ///< 项目节点
    Directory,  ///< 目录节点
    File,       ///< 文件节点
    Function    ///< 函数节点
};

/**
 * @brief 树节点类，用于构建树形数据结构
 */
class TreeItem
{
   public:
    /**
     * @brief 构造函数
     * @param type 节点类型
     * @param parent 父节点指针
     */
    explicit TreeItem(TreeItemType type, TreeItem* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~TreeItem();

    /**
     * @brief 获取子节点
     * @param index 子节点索引
     * @return 子节点指针
     */
    TreeItem* child(int index);

    /**
     * @brief 获取子节点数量
     * @return 子节点数量
     */
    int childCount() const;

    /**
     * @brief 获取子节点索引
     * @param child 子节点指针
     * @return 子节点索引，未找到返回-1
     */
    int indexOfChild(TreeItem* child) const;

    /**
     * @brief 获取父节点
     * @return 父节点指针
     */
    TreeItem* parent() const;

    /**
     * @brief 添加子节点
     * @param child 子节点指针
     */
    void appendChild(TreeItem* child);

    /**
     * @brief 插入子节点
     * @param index 插入位置
     * @param child 子节点指针
     */
    void insertChild(int index, TreeItem* child);

    /**
     * @brief 移除子节点
     * @param index 子节点索引
     */
    void removeChild(int index);

    /**
     * @brief 清空所有子节点
     */
    void clearChildren();

    /**
     * @brief 获取节点类型
     * @return 节点类型
     */
    TreeItemType type() const;

    /**
     * @brief 获取显示文本
     * @return 显示文本
     */
    QString displayText() const;

    /**
     * @brief 设置显示文本
     * @param text 显示文本
     */
    void setDisplayText(const QString& text);

    /**
     * @brief 获取节点路径（用于目录和文件节点）
     * @return 节点路径
     */
    QString path() const;

    /**
     * @brief 设置节点路径
     * @param path 节点路径
     */
    void setPath(const QString& path);

    /**
     * @brief 获取函数数据（仅函数节点有效）
     * @return 函数数据
     */
    FunctionData functionData() const;

    /**
     * @brief 设置函数数据
     * @param data 函数数据
     */
    void setFunctionData(const FunctionData& data);

    /**
     * @brief 获取项目信息（仅项目节点有效）
     * @return 项目信息
     */
    ProjectInfo projectInfo() const;

    /**
     * @brief 设置项目信息
     * @param info 项目信息
     */
    void setProjectInfo(const ProjectInfo& info);

    /**
     * @brief 获取函数ID（仅函数节点有效）
     * @return 函数ID
     */
    int functionId() const;

    /**
     * @brief 获取项目ID（仅项目节点有效）
     * @return 项目ID
     */
    int projectId() const;

    /**
     * @brief 检查是否匹配搜索关键词
     * @param keyword 搜索关键词
     * @return 是否匹配
     */
    bool matchesSearch(const QString& keyword) const;

   private:
    TreeItemType m_type;          ///< 节点类型
    TreeItem* m_parent;           ///< 父节点指针
    QList<TreeItem*> m_children;  ///< 子节点列表
    QString m_displayText;        ///< 显示文本
    QString m_path;               ///< 节点路径
    FunctionData m_functionData;  ///< 函数数据
    ProjectInfo m_projectInfo;    ///< 项目信息
};

#endif
