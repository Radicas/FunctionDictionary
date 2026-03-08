/**
 * @file idatabaserepository.h
 * @brief 数据库仓库接口定义，用于解耦数据访问层
 * @author FunctionDB Team
 * @date 2026-03-08
 * @version 1.0
 * 
 * @details 该文件定义了数据访问的抽象接口，
 * 采用Repository模式实现数据访问层与业务逻辑层的解耦。
 * 支持依赖注入，便于单元测试和实现替换。
 */

#ifndef IDATABASEREPOSITORY_H
#define IDATABASEREPOSITORY_H

#include <QString>
#include <QVector>
#include <QSet>
#include "core/models/functiondata.h"
#include "core/models/projectinfo.h"
#include "core/models/batchconfig.h"

/**
 * @brief 项目仓库接口
 * 
 * @details 定义项目数据的访问接口，遵循Repository模式。
 * UI层和业务层通过此接口访问项目数据，不直接依赖DatabaseManager。
 */
class IProjectRepository {
public:
    virtual ~IProjectRepository() = default;
    
    virtual bool addProject(ProjectInfo& project) = 0;
    virtual bool updateProject(const ProjectInfo& project) = 0;
    virtual bool deleteProject(int projectId) = 0;
    virtual QVector<ProjectInfo> getAllProjects() = 0;
    virtual ProjectInfo getProjectById(int projectId) = 0;
    virtual bool projectPathExists(const QString& rootPath) = 0;
    virtual ProjectInfo getOrCreateTemporaryProject() = 0;
    virtual bool isTemporaryProject(int projectId) = 0;
};

/**
 * @brief 函数仓库接口
 * 
 * @details 定义函数数据的访问接口，遵循Repository模式。
 * UI层和业务层通过此接口访问函数数据，不直接依赖DatabaseManager。
 */
class IFunctionRepository {
public:
    virtual ~IFunctionRepository() = default;
    
    virtual bool addFunction(const FunctionData& func) = 0;
    virtual bool addFunction(const QString& key, const QString& value) = 0;
    virtual bool deleteFunction(int id) = 0;
    virtual bool deleteFunctions(const QVector<int>& ids) = 0;
    virtual bool deleteFunctionsByProject(int projectId) = 0;
    virtual QVector<FunctionData> getAllFunctions() = 0;
    virtual FunctionData getFunctionById(int id) = 0;
    virtual FunctionData getFunctionByKey(const QString& key) = 0;
    virtual QVector<FunctionData> getFunctionsByProject(int projectId) = 0;
    virtual bool functionExists(const QString& key) = 0;
    virtual bool functionExistsByKeyAndPath(const QString& key, const QString& filePath) = 0;
    virtual int addFunctionsBatch(const QVector<FunctionData>& functions) = 0;
    virtual bool upsertFunction(const FunctionData& func) = 0;
    virtual bool updateFunctionProject(int functionId, int newProjectId) = 0;
};

/**
 * @brief 处理状态仓库接口
 * 
 * @details 定义处理状态的访问接口，用于批量处理的状态跟踪。
 */
class IProcessStateRepository {
public:
    virtual ~IProcessStateRepository() = default;
    
    virtual bool saveProcessState(const QString& filePath, const QString& functionName,
                                   const QString& status, const QString& errorMessage = "") = 0;
    virtual QVector<ProcessStateRecord> getProcessState(const QString& filePath) = 0;
    virtual bool clearProcessState(const QString& filePath) = 0;
    virtual QSet<QString> getProcessedFunctions(const QString& filePath) = 0;
};

/**
 * @brief 数据库管理接口
 * 
 * @details 定义数据库初始化和管理的接口。
 */
class IDatabaseManager : public IProjectRepository, 
                          public IFunctionRepository, 
                          public IProcessStateRepository {
public:
    virtual bool init(const QString& dbPath) = 0;
    virtual bool isInitialized() const = 0;
    virtual QString lastError() const = 0;
    virtual bool clearAllData() = 0;
};

#endif // IDATABASEREPOSITORY_H
