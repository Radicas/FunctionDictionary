/**
 * @file functiontreemodel.h
 * @brief 函数树形模型，实现QAbstractItemModel用于QTreeView
 * @author Developer
 * @date 2026-03-05
 * @version 1.0
 */

#ifndef FUNCTIONTREEMODEL_H
#define FUNCTIONTREEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "core/models/treeitem.h"
#include "core/models/functiondata.h"
#include "core/models/projectinfo.h"

/**
 * @brief 函数树形模型类，提供树形数据结构给QTreeView
 */
class FunctionTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit FunctionTreeModel(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FunctionTreeModel() override;
    
    /**
     * @brief 获取模型索引
     * @param row 行号
     * @param column 列号
     * @param parent 父索引
     * @return 模型索引
     */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * @brief 获取父索引
     * @param index 子索引
     * @return 父索引
     */
    QModelIndex parent(const QModelIndex& index) const override;
    
    /**
     * @brief 获取行数
     * @param parent 父索引
     * @return 行数
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * @brief 获取列数
     * @param parent 父索引
     * @return 列数
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * @brief 获取数据
     * @param index 模型索引
     * @param role 数据角色
     * @return 数据
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief 获取表头数据
     * @param section 节
     * @param orientation 方向
     * @param role 数据角色
     * @return 表头数据
     */
    QVariant headerData(int section, Qt::Orientation orientation, 
                        int role = Qt::DisplayRole) const override;
    
    /**
     * @brief 设置数据并刷新
     * @param projects 项目列表
     * @param functions 函数列表
     */
    void setTreeData(const QVector<ProjectInfo>& projects, const QVector<FunctionData>& functions);
    
    /**
     * @brief 刷新数据
     */
    void refresh();
    
    /**
     * @brief 获取节点类型
     * @param index 模型索引
     * @return 节点类型
     */
    TreeItemType itemType(const QModelIndex& index) const;
    
    /**
     * @brief 获取函数数据
     * @param index 模型索引
     * @return 函数数据
     */
    FunctionData getFunctionData(const QModelIndex& index) const;
    
    /**
     * @brief 获取项目信息
     * @param index 模型索引
     * @return 项目信息
     */
    ProjectInfo getProjectInfo(const QModelIndex& index) const;
    
    /**
     * @brief 获取节点路径
     * @param index 模型索引
     * @return 节点路径
     */
    QString getNodePath(const QModelIndex& index) const;
    
    /**
     * @brief 查找函数节点索引
     * @param functionId 函数ID
     * @return 模型索引
     */
    QModelIndex findFunctionIndex(int functionId) const;
    
    /**
     * @brief 查找项目节点索引
     * @param projectId 项目ID
     * @return 模型索引
     */
    QModelIndex findProjectIndex(int projectId) const;

    /**
     * @brief 获取支持的拖放动作
     * @return 支持的动作
     */
    Qt::DropActions supportedDropActions() const override;

    /**
     * @brief 获取支持的MIME类型
     * @return MIME类型列表
     */
    QStringList mimeTypes() const override;

    /**
     * @brief 将数据编码为MIME格式
     * @param indexes 索引列表
     * @return MIME数据
     */
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

    /**
     * @brief 处理拖放数据
     * @param data MIME数据
     * @param action 拖放动作
     * @param row 行号
     * @param column 列号
     * @param parent 父索引
     * @return 是否成功
     */
    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                      int row, int column, const QModelIndex& parent) override;

signals:
    /**
     * @brief 数据需要刷新信号
     */
    void dataRefreshNeeded();

    /**
     * @brief 函数移动信号
     * @param functionId 函数ID
     * @param targetProjectId 目标项目ID
     */
    void functionMoved(int functionId, int targetProjectId);

private:
    /**
     * @brief 初始化根节点
     */
    void setupRootItem();
    
    /**
     * @brief 构建树形结构
     */
    void buildTree();
    
    /**
     * @brief 从TreeItem获取QModelIndex
     * @param item TreeItem指针
     * @return 模型索引
     */
    QModelIndex indexFromItem(TreeItem* item) const;
    
    /**
     * @brief 从QModelIndex获取TreeItem
     * @param index 模型索引
     * @return TreeItem指针
     */
    TreeItem* itemFromIndex(const QModelIndex& index) const;
    
    /**
     * @brief 递归查找函数节点
     * @param parent 父节点
     * @param functionId 函数ID
     * @return TreeItem指针
     */
    TreeItem* findFunctionItem(TreeItem* parent, int functionId) const;
    
    /**
     * @brief 递归查找项目节点
     * @param parent 父节点
     * @param projectId 项目ID
     * @return TreeItem指针
     */
    TreeItem* findProjectItem(TreeItem* parent, int projectId) const;
    
    TreeItem* m_rootItem;               ///< 根节点指针
    QVector<ProjectInfo> m_projects;    ///< 项目数据缓存
    QVector<FunctionData> m_functions;  ///< 函数数据缓存
};

/**
 * @brief 函数树过滤代理模型，用于搜索过滤
 */
class FunctionTreeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit FunctionTreeProxyModel(QObject* parent = nullptr);
    
    /**
     * @brief 设置搜索关键词
     * @param keyword 搜索关键词
     */
    void setSearchKeyword(const QString& keyword);
    
    /**
     * @brief 获取搜索关键词
     * @return 搜索关键词
     */
    QString searchKeyword() const;

protected:
    /**
     * @brief 过滤判断
     * @param sourceRow 源行号
     * @param sourceParent 源父索引
     * @return 是否接受
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    
    /**
     * @brief 递归检查子节点是否匹配
     * @param index 源索引
     * @return 是否有匹配的子节点
     */
    bool hasMatchingChildren(const QModelIndex& index) const;

private:
    QString m_searchKeyword;            ///< 搜索关键词
};

#endif
