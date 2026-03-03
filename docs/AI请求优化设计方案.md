# AI请求发送功能优化设计方案

## 一、现有架构分析

### 1.1 当前问题

通过分析现有代码，发现以下问题：

| 问题 | 描述 | 影响 |
|------|------|------|
| **单函数处理** | `AIServiceManager::analyzeCode` 只能处理单个代码块 | 无法批量处理源文件中的多个函数 |
| **无函数提取** | `FunctionalityWidget::readFileAndAnalyze` 直接将整个文件内容发送给AI | AI处理效率低，响应质量差 |
| **同步阻塞** | 处理过程中UI完全阻塞 | 用户体验差 |
| **无进度追踪** | 缺少详细的处理进度反馈 | 用户无法了解处理状态 |
| **无断点续传** | 失败后需从头开始 | 大文件处理风险高 |

### 1.2 现有数据模型

```cpp
// FunctionData 结构体（现有）
struct FunctionData {
    int id;                 // 函数ID
    QString key;            // 函数名称（唯一标识）
    QString value;          // 函数介绍（Markdown格式）
    QDateTime createTime;   // 创建时间
};
```

---

## 二、优化方案总体架构

### 2.1 架构图

```
┌─────────────────────────────────────────────────────────────────────┐
│                         用户界面层 (UI Layer)                        │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    FunctionalityWidget                       │   │
│  │  - 文件选择与预览                                             │   │
│  │  - 处理进度展示                                               │   │
│  │  - 结果反馈与统计                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                       业务逻辑层 (Business Layer)                    │
│  ┌──────────────────────┐    ┌──────────────────────────────────┐  │
│  │   FunctionParser     │    │     BatchProcessManager          │  │
│  │   (函数提取器)        │    │     (批量处理管理器)              │  │
│  │  - 多语言支持         │    │  - 任务队列管理                   │  │
│  │  - AST解析           │    │  - 并发控制                       │  │
│  │  - 函数边界识别       │    │  - 进度追踪                       │  │
│  └──────────────────────┘    │  - 断点续传                       │  │
│                               └──────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        服务层 (Service Layer)                        │
│  ┌──────────────────────┐    ┌──────────────────────────────────┐  │
│  │  AIServiceManager    │    │     DatabaseManager              │  │
│  │  (AI服务管理器)       │    │     (数据库管理器)                │  │
│  │  - 请求队列          │    │  - 批量插入                       │  │
│  │  - 重试机制          │    │  - 事务支持                       │  │
│  │  - 速率限制          │    │  - 去重处理                       │  │
│  └──────────────────────┘    └──────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 三、核心模块设计

### 3.1 函数提取逻辑 (FunctionParser)

#### 3.1.1 新增数据模型

```cpp
/**
 * @file functionextract.h
 * @brief 函数提取结果数据模型
 */

/**
 * @brief 参数信息结构
 */
struct ParameterInfo {
    QString name;                    ///< 参数名称
    QString type;                    ///< 参数类型
    QString defaultValue;            ///< 默认值（如有）
};

/**
 * @brief 提取的函数信息结构
 */
struct ExtractedFunction {
    QString name;                    ///< 函数名称
    QString signature;               ///< 函数签名
    QString body;                    ///< 函数体代码
    QString returnType;              ///< 返回类型
    QVector<ParameterInfo> parameters; ///< 参数列表
    int startLine;                   ///< 起始行号
    int endLine;                     ///< 结束行号
    QString filePath;                ///< 所属文件路径
    QString language;                ///< 编程语言类型
};

/**
 * @brief 函数提取结果
 */
struct ExtractionResult {
    bool success;                    ///< 是否成功
    QString filePath;                ///< 文件路径
    QString language;                ///< 语言类型
    QVector<ExtractedFunction> functions; ///< 提取的函数列表
    QString errorMessage;            ///< 错误信息
    int totalLines;                  ///< 文件总行数
};
```

#### 3.1.2 函数解析器类设计

```cpp
/**
 * @file functionparser.h
 * @brief 函数解析器，支持多种编程语言的函数提取
 */
class FunctionParser : public QObject {
    Q_OBJECT

public:
    static FunctionParser& instance();
    
    /**
     * @brief 从源文件提取所有函数
     * @param filePath 源文件路径
     * @return 提取结果
     */
    ExtractionResult extractFunctions(const QString& filePath);
    
    /**
     * @brief 从代码文本提取函数
     * @param code 代码文本
     * @param language 语言类型
     * @return 提取结果
     */
    ExtractionResult extractFromCode(const QString& code, const QString& language);
    
    /**
     * @brief 检测文件语言类型
     * @param filePath 文件路径
     * @return 语言类型标识符
     */
    QString detectLanguage(const QString& filePath) const;

signals:
    /**
     * @brief 提取进度信号
     * @param current 当前进度
     * @param total 总数
     * @param message 进度消息
     */
    void extractionProgress(int current, int total, const QString& message);

private:
    // 语言特定解析器
    QMap<QString, std::function<ExtractionResult(const QString&)>> m_parsers;
    
    // C/C++ 解析器
    ExtractionResult parseCpp(const QString& code);
    
    // Python 解析器
    ExtractionResult parsePython(const QString& code);
    
    // Java 解析器
    ExtractionResult parseJava(const QString& code);
    
    // JavaScript/TypeScript 解析器
    ExtractionResult parseJavaScript(const QString& code);
    
    // 通用正则解析器（作为后备方案）
    ExtractionResult parseGeneric(const QString& code, const QString& language);
};
```

#### 3.1.3 C/C++ 函数提取策略

```cpp
ExtractionResult FunctionParser::parseCpp(const QString& code) {
    ExtractionResult result;
    result.language = "cpp";
    
    // 策略: 使用正则表达式匹配函数定义
    // 匹配模式: [返回类型] [函数名]([参数列表]) [const] [{]
    QRegularExpression funcPattern(
        R"((?:^|\n)\s*([\w:]+(?:\s*[*&]+)?)\s+(\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?\{)"
    );
    
    QRegularExpressionMatchIterator it = funcPattern.globalMatch(code);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ExtractedFunction func;
        func.returnType = match.captured(1).trimmed();
        func.name = match.captured(2).trimmed();
        func.signature = func.returnType + " " + func.name + "(" + match.captured(3) + ")";
        
        // 提取函数体（需要平衡括号匹配）
        int bodyStart = match.capturedEnd();
        func.body = extractFunctionBody(code, bodyStart);
        func.startLine = code.left(match.capturedStart()).count('\n') + 1;
        func.endLine = func.startLine + func.body.count('\n');
        
        result.functions.append(func);
    }
    
    result.success = !result.functions.isEmpty();
    return result;
}
```

---

### 3.2 分批处理策略 (BatchProcessManager)

#### 3.2.1 批量处理配置与状态

```cpp
/**
 * @brief 批量处理配置
 */
struct BatchProcessConfig {
    int maxConcurrentRequests = 1;    ///< 最大并发请求数
    int requestTimeout = 60000;       ///< 请求超时时间（毫秒）
    int maxRetryCount = 3;            ///< 最大重试次数
    int retryDelay = 2000;            ///< 重试延迟（毫秒）
    int requestInterval = 1000;       ///< 请求间隔（毫秒，用于速率限制）
    bool skipExisting = true;         ///< 是否跳过已存在的函数
    bool enableCheckpoint = true;     ///< 是否启用断点续传
};

/**
 * @brief 批量处理状态
 */
enum class BatchProcessState {
    Idle,       ///< 空闲
    Running,    ///< 运行中
    Paused,     ///< 已暂停
    Completed,  ///< 已完成
    Cancelled   ///< 已取消
};
```

#### 3.2.2 批量处理管理器

```cpp
/**
 * @file batchprocessmanager.h
 * @brief 批量处理管理器，负责协调函数的批量AI分析
 */
class BatchProcessManager : public QObject {
    Q_OBJECT

public:
    static BatchProcessManager& instance();
    
    /**
     * @brief 开始批量处理
     * @param functions 待处理的函数列表
     */
    void startBatchProcessing(const QVector<ExtractedFunction>& functions);
    
    /**
     * @brief 暂停处理
     */
    void pauseProcessing();
    
    /**
     * @brief 恢复处理
     */
    void resumeProcessing();
    
    /**
     * @brief 取消处理
     */
    void cancelProcessing();
    
    /**
     * @brief 获取当前处理状态
     */
    BatchProcessState getState() const;
    
    /**
     * @brief 设置并发配置
     */
    void setConfig(const BatchProcessConfig& config);

signals:
    /**
     * @brief 整体进度信号
     * @param current 当前处理的函数索引
     * @param total 总函数数量
     * @param functionName 当前处理的函数名
     */
    void batchProgress(int current, int total, const QString& functionName);
    
    /**
     * @brief 单个函数处理完成信号
     * @param func 函数信息
     * @param success 是否成功
     * @param message 结果消息
     */
    void functionProcessed(const ExtractedFunction& func, bool success, const QString& message);
    
    /**
     * @brief 批量处理完成信号
     * @param successCount 成功数量
     * @param failedCount 失败数量
     * @param skippedCount 跳过数量
     */
    void batchCompleted(int successCount, int failedCount, int skippedCount);

private:
    // 处理队列
    QQueue<ExtractedFunction> m_processQueue;
    
    // 已完成列表（用于断点续传）
    QSet<QString> m_processedFunctions;
    
    // 失败列表（用于重试）
    QMap<QString, int> m_failedFunctions; // functionName -> retryCount
    
    // 状态管理
    BatchProcessState m_state;
    BatchProcessConfig m_config;
    
    // 统计信息
    int m_successCount;
    int m_failedCount;
    int m_skippedCount;
    
    // 处理下一个函数
    void processNext();
    
    // 保存处理状态（断点续传）
    void saveProcessState();
    
    // 加载处理状态
    void loadProcessState();
};
```

#### 3.2.3 处理流程图

```
                    ┌─────────────────┐
                    │  接收函数列表    │
                    └────────┬────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │  初始化处理队列  │◄─────────────────┐
                    └────────┬────────┘                  │
                              │                          │
                              ▼                          │
                    ┌─────────────────┐                  │
                    │  检查是否暂停？  │                  │
                    └────────┬────────┘                  │
                     是 │    │ 否                       │
                        │    ▼                          │
                        │  ┌─────────────────┐          │
                        │  │  队列是否为空？  │          │
                        │  └────────┬────────┘          │
                        │   是 │    │ 否               │
                        │      │    ▼                  │
                        │      │  ┌─────────────────┐  │
                        │      │  │  取出下一个函数  │  │
                        │      │  └────────┬────────┘  │
                        │      │           │           │
                        │      │           ▼           │
                        │      │  ┌─────────────────┐  │
                        │      │  │ 检查是否已存在？ │  │
                        │      │  └────────┬────────┘  │
                        │      │   是 │    │ 否       │
                        │      │      │    ▼          │
                        │      │      │  ┌─────────┐  │
                        │      │      │  │ 跳过    │──┼──┐
                        │      │      │  └─────────┘  │  │
                        │      │      │               │  │
                        │      │      └───────────────┼──┤
                        │      │                      │  │
                        │      │           ┌──────────┘  │
                        │      │           ▼             │
                        │      │  ┌─────────────────┐    │
                        │      │  │  发送AI请求     │    │
                        │      │  └────────┬────────┘    │
                        │      │           │             │
                        │      │           ▼             │
                        │      │  ┌─────────────────┐    │
                        │      │  │  请求是否成功？  │    │
                        │      │  └────────┬────────┘    │
                        │      │   否 │    │ 是         │
                        │      │      │    ▼            │
                        │      │      │  ┌───────────┐  │
                        │      │      │  │ 存储数据库 │  │
                        │      │      │  └─────┬─────┘  │
                        │      │      │        │        │
                        │      │      │        ▼        │
                        │      │      │  ┌───────────┐  │
                        │      │      │  │ 更新进度  │  │
                        │      │      │  └─────┬─────┘  │
                        │      │      │        │        │
                        │      │      ▼        │        │
                        │      │  ┌───────────┐│        │
                        │      │  │ 重试逻辑  ││        │
                        │      │  └─────┬─────┘│        │
                        │      │        │      │        │
                        │      └────────┼──────┼────────┤
                        │               │      │        │
                        │               ▼      ▼        │
                        │         ┌─────────────────┐   │
                        │         │  等待间隔时间   │───┼──┘
                        │         └─────────────────┘   │
                        │                               │
                        ▼                               │
                  ┌───────────┐                         │
                  │  等待恢复 │─────────────────────────┘
                  └───────────┘
```

---

### 3.3 AI请求发送机制优化

#### 3.3.1 增强的AIServiceManager

```cpp
/**
 * @brief AI分析请求结构
 */
struct AIAnalysisRequest {
    QString requestId;                  ///< 请求唯一标识
    ExtractedFunction function;         ///< 待分析的函数
    int priority;                       ///< 优先级
    QDateTime createTime;               ///< 创建时间
};

/**
 * @brief AI分析响应结构
 */
struct AIAnalysisResponse {
    QString requestId;                  ///< 请求ID
    bool success;                       ///< 是否成功
    QString functionName;               ///< 函数名称
    QString functionDescription;        ///< 函数描述
    QString errorMessage;               ///< 错误信息
    int retryCount;                     ///< 重试次数
    qint64 responseTime;                ///< 响应时间（毫秒）
};

// AIServiceManager 新增方法
class AIServiceManager : public QObject {
    // ... 现有代码 ...

public:
    /**
     * @brief 分析单个函数（异步）
     * @param func 要分析的函数
     * @param requestId 请求ID
     */
    void analyzeFunction(const ExtractedFunction& func, const QString& requestId);
    
    /**
     * @brief 批量分析函数（队列模式）
     * @param functions 函数列表
     */
    void analyzeFunctions(const QVector<ExtractedFunction>& functions);
    
    /**
     * @brief 获取请求队列状态
     */
    RequestQueueStatus getQueueStatus() const;
    
    /**
     * @brief 设置速率限制
     * @param requestsPerMinute 每分钟请求数
     */
    void setRateLimit(int requestsPerMinute);

signals:
    /**
     * @brief 单个函数分析完成信号
     * @param response 分析响应
     */
    void functionAnalysisComplete(const AIAnalysisResponse& response);
    
    /**
     * @brief 队列状态变化信号
     */
    void queueStatusChanged(const RequestQueueStatus& status);

private:
    // 请求队列
    QQueue<AIAnalysisRequest> m_requestQueue;
    
    // 活跃请求
    QMap<QString, QNetworkReply*> m_activeRequests;
    
    // 速率限制器
    RateLimiter m_rateLimiter;
    
    // 处理队列
    void processQueue();
    
    // 构建函数分析提示词
    QString buildFunctionPrompt(const ExtractedFunction& func) const;
};
```

#### 3.3.2 优化的提示词设计

```cpp
QString AIServiceManager::buildFunctionPrompt(const ExtractedFunction& func) const {
    QString prompt = QString(
        "请分析以下%1函数，提取函数详细信息。\n\n"
        "函数名称: %2\n"
        "函数签名: %3\n\n"
        "函数代码:\n"
        "```%4\n"
        "%5\n"
        "```\n\n"
        "请以JSON格式返回，格式如下：\n"
        "{\n"
        "  \"function_name\": \"函数名\",\n"
        "  \"function_description\": \"函数的详细功能说明，包括：\\n"
        "    1. 功能概述\\n"
        "    2. 参数说明\\n"
        "    3. 返回值说明\\n"
        "    4. 使用示例\\n"
        "    5. 注意事项\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"参数名\", \"type\": \"参数类型\", \"description\": \"参数说明\"}\n"
        "  ],\n"
        "  \"return_type\": \"返回值类型\",\n"
        "  \"return_description\": \"返回值说明\",\n"
        "  \"flowchart\": \"使用mermaid flowchart语法绘制的函数运行流程图\",\n"
        "  \"sequence_diagram\": \"使用mermaid sequenceDiagram语法绘制的函数调用时序图\",\n"
        "  \"structure_diagram\": \"使用mermaid graph语法绘制的函数结构关系图\"\n"
        "}\n\n"
        "要求：\n"
        "1. 描述应基于代码实际逻辑，准确反映函数功能\n"
        "2. 所有mermaid图表语法必须正确\n"
        "3. 返回的JSON必须是有效格式"
    ).arg(func.language)
     .arg(func.name)
     .arg(func.signature)
     .arg(func.language)
     .arg(func.body);
    
    return prompt;
}
```

---

### 3.4 数据库存储方案优化

#### 3.4.1 扩展数据模型

```cpp
/**
 * @brief 增强的函数数据结构
 */
struct FunctionData {
    int id;                             ///< 函数ID
    QString key;                        ///< 函数名称（唯一标识）
    QString value;                      ///< 函数介绍（Markdown格式）
    QDateTime createTime;               ///< 创建时间
    
    // 新增字段
    QString signature;                  ///< 函数签名
    QString returnType;                 ///< 返回类型
    QString parameters;                 ///< 参数列表（JSON格式）
    QString filePath;                   ///< 源文件路径
    int startLine;                      ///< 起始行号
    int endLine;                        ///< 结束行号
    QString language;                   ///< 编程语言
    QString flowchart;                  ///< 流程图
    QString sequenceDiagram;            ///< 时序图
    QString structureDiagram;           ///< 结构图
    QString aiModel;                    ///< 分析使用的AI模型
    QDateTime analyzeTime;              ///< 分析时间
};
```

#### 3.4.2 数据库表结构升级

```sql
-- 升级后的functions表结构
CREATE TABLE IF NOT EXISTS functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    key TEXT NOT NULL UNIQUE,
    value TEXT NOT NULL,
    signature TEXT,
    return_type TEXT,
    parameters TEXT,
    file_path TEXT,
    start_line INTEGER,
    end_line INTEGER,
    language TEXT,
    flowchart TEXT,
    sequence_diagram TEXT,
    structure_diagram TEXT,
    ai_model TEXT,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    analyze_time DATETIME
);

-- 创建索引以提高查询性能
CREATE INDEX IF NOT EXISTS idx_functions_key ON functions(key);
CREATE INDEX IF NOT EXISTS idx_functions_file_path ON functions(file_path);
CREATE INDEX IF NOT EXISTS idx_functions_language ON functions(language);

-- 创建处理状态表（用于断点续传）
CREATE TABLE IF NOT EXISTS process_state (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_path TEXT NOT NULL,
    function_name TEXT NOT NULL,
    status TEXT NOT NULL,  -- 'pending', 'processing', 'completed', 'failed'
    retry_count INTEGER DEFAULT 0,
    error_message TEXT,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(file_path, function_name)
);
```

#### 3.4.3 DatabaseManager 新增方法

```cpp
class DatabaseManager {
public:
    // ... 现有方法 ...
    
    /**
     * @brief 批量添加函数
     * @param functions 函数数据列表
     * @return 成功添加的数量
     */
    int addFunctionsBatch(const QVector<FunctionData>& functions);
    
    /**
     * @brief 更新或插入函数（存在则更新，不存在则插入）
     * @param func 函数数据
     * @return 操作是否成功
     */
    bool upsertFunction(const FunctionData& func);
    
    /**
     * @brief 保存处理状态
     * @param filePath 文件路径
     * @param functionName 函数名
     * @param status 状态
     * @param errorMessage 错误信息
     */
    bool saveProcessState(const QString& filePath, const QString& functionName,
                          const QString& status, const QString& errorMessage = "");
    
    /**
     * @brief 获取文件的处理状态
     * @param filePath 文件路径
     * @return 处理状态列表
     */
    QVector<ProcessStateRecord> getProcessState(const QString& filePath);
    
    /**
     * @brief 清除文件的处理状态
     * @param filePath 文件路径
     */
    bool clearProcessState(const QString& filePath);
    
    /**
     * @brief 获取文件中已处理的函数列表
     * @param filePath 文件路径
     * @return 已处理的函数名集合
     */
    QSet<QString> getProcessedFunctions(const QString& filePath);
};
```

---

### 3.5 错误处理流程

#### 3.5.1 错误类型定义

```cpp
/**
 * @brief 错误类型枚举
 */
enum class ProcessErrorType {
    None,                   ///< 无错误
    FileNotFound,           ///< 文件不存在
    FileReadError,          ///< 文件读取错误
    ParseError,             ///< 解析错误
    AIConfigError,          ///< AI配置错误
    AIRequestError,         ///< AI请求错误
    AIRateLimitError,       ///< AI速率限制错误
    AITimeoutError,         ///< AI超时错误
    AIResponseError,        ///< AI响应错误
    DatabaseError,          ///< 数据库错误
    DuplicateError,         ///< 重复错误
    UnknownError            ///< 未知错误
};

/**
 * @brief 错误信息结构
 */
struct ProcessError {
    ProcessErrorType type;              ///< 错误类型
    QString message;                    ///< 错误消息
    QString functionName;               ///< 相关函数名
    int retryCount;                     ///< 重试次数
    bool recoverable;                   ///< 是否可恢复
};
```

#### 3.5.2 错误处理策略

```cpp
/**
 * @brief 错误处理动作
 */
enum class ErrorAction {
    Retry,          ///< 重试
    Skip,           ///< 跳过
    Abort,          ///< 中止
    Pause,          ///< 暂停等待用户确认
    SaveAndContinue ///< 保存状态并继续下一个
};

/**
 * @brief 错误处理器
 */
class ErrorHandler {
public:
    /**
     * @brief 处理错误
     * @param error 错误信息
     * @return 处理后的动作
     */
    ErrorAction handleError(const ProcessError& error);
    
    /**
     * @brief 获取重试延迟时间（指数退避）
     * @param retryCount 当前重试次数
     * @return 延迟时间（毫秒）
     */
    int getRetryDelay(int retryCount) const {
        // 指数退避：1s, 2s, 4s, 8s, ...
        return qMin(1000 * (1 << retryCount), 60000);
    }

private:
    QMap<ProcessErrorType, ErrorAction> m_errorActions;
};
```

---

### 3.6 性能优化措施

#### 3.6.1 并发控制

```cpp
/**
 * @brief 并发控制器
 */
class ConcurrencyController {
public:
    /**
     * @brief 尝试获取处理许可
     * @return 是否获得许可
     */
    bool tryAcquire();
    
    /**
     * @brief 释放处理许可
     */
    void release();
    
    /**
     * @brief 等待获取许可
     * @param timeout 超时时间
     */
    bool waitForAcquire(int timeout = -1);

private:
    QSemaphore m_semaphore;
    QMutex m_mutex;
    int m_maxConcurrent;
};
```

#### 3.6.2 速率限制器

```cpp
/**
 * @brief 速率限制器（令牌桶算法）
 */
class RateLimiter {
public:
    RateLimiter(int requestsPerMinute);
    
    /**
     * @brief 等待直到可以发送请求
     */
    void waitForToken();
    
    /**
     * @brief 检查是否可以立即发送请求
     */
    bool canSendImmediately() const;

private:
    QMutex m_mutex;
    QWaitCondition m_condition;
    int m_tokens;
    int m_maxTokens;
    qint64 m_lastRefillTime;
    int m_refillRate;  // tokens per second
};
```

#### 3.6.3 缓存机制

```cpp
/**
 * @brief 分析结果缓存
 */
class AnalysisCache {
public:
    /**
     * @brief 获取缓存的分析结果
     * @param functionSignature 函数签名
     * @return 缓存的结果，如果不存在返回空
     */
    std::optional<FunctionData> get(const QString& functionSignature);
    
    /**
     * @brief 保存分析结果到缓存
     */
    void put(const QString& functionSignature, const FunctionData& data);
    
    /**
     * @brief 清除缓存
     */
    void clear();

private:
    QCache<QString, FunctionData> m_cache;
    QMutex m_mutex;
};
```

---

## 四、集成方案

### 4.1 更新后的 FunctionalityWidget

```cpp
class FunctionalityWidget : public QWidget {
    Q_OBJECT

public:
    explicit FunctionalityWidget(QWidget *parent = nullptr);

private slots:
    // 文件选择
    void onFileSelectClicked();
    
    // 开始解析
    void onParseButtonClicked();
    
    // 暂停/恢复处理
    void onPauseResumeClicked();
    
    // 取消处理
    void onCancelClicked();
    
    // 批量处理进度
    void onBatchProgress(int current, int total, const QString& functionName);
    
    // 单个函数处理完成
    void onFunctionProcessed(const ExtractedFunction& func, bool success, const QString& message);
    
    // 批量处理完成
    void onBatchCompleted(int successCount, int failedCount, int skippedCount);

private:
    // UI组件
    QLabel* m_fileInfoLabel;           ///< 文件信息标签
    QLabel* m_functionCountLabel;      ///< 函数数量标签
    QProgressBar* m_progressBar;       ///< 进度条
    QLabel* m_statusLabel;             ///< 状态标签
    QTextEdit* m_logView;              ///< 日志视图
    QPushButton* m_pauseResumeButton;  ///< 暂停/恢复按钮
    QPushButton* m_cancelButton;       ///< 取消按钮
    
    // 统计信息
    QLabel* m_successCountLabel;       ///< 成功计数
    QLabel* m_failedCountLabel;        ///< 失败计数
    QLabel* m_skippedCountLabel;       ///< 跳过计数
    
    // 当前文件信息
    QString m_currentFilePath;
    ExtractionResult m_extractionResult;
};
```

### 4.2 处理流程时序图

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│Functionality │    │FunctionParser│    │BatchProcess  │    │AIService     │    │Database      │
│   Widget     │    │              │    │  Manager     │    │  Manager     │    │  Manager     │
└──────┬───────┘    └──────┬───────┘    └──────┬───────┘    └──────┬───────┘    └──────┬───────┘
       │                   │                   │                   │                   │
       │  选择文件          │                   │                   │                   │
       │──────────────────►│                   │                   │                   │
       │                   │                   │                   │                   │
       │  extractFunctions │                   │                   │                   │
       │──────────────────►│                   │                   │                   │
       │                   │                   │                   │                   │
       │  ExtractionResult │                   │                   │                   │
       │◄──────────────────│                   │                   │                   │
       │                   │                   │                   │                   │
       │  显示函数数量      │                   │                   │                   │
       │                   │                   │                   │                   │
       │  点击开始解析      │                   │                   │                   │
       │──────────────────────────────────────►│                   │                   │
       │                   │                   │                   │                   │
       │                   │                   │ startBatchProcessing                  │
       │                   │                   │──────────────────►│                   │
       │                   │                   │                   │                   │
       │                   │                   │                   │ analyzeFunction   │
       │                   │                   │                   │──────────────────►│
       │                   │                   │                   │                   │
       │                   │                   │                   │◄──────────────────│
       │                   │                   │                   │  AI响应           │
       │                   │                   │                   │                   │
       │                   │                   │◄──────────────────│                   │
       │                   │                   │ functionAnalysisComplete              │
       │                   │                   │                   │                   │
       │                   │                   │──────────────────────────────────────►│
       │                   │                   │                   │  addFunction      │
       │                   │                   │                   │                   │
       │◄─────────────────────────────────────│                   │                   │
       │  batchProgress    │                   │                   │                   │
       │                   │                   │                   │                   │
       │  ... (重复处理每个函数)               │                   │                   │
       │                   │                   │                   │                   │
       │◄─────────────────────────────────────│                   │                   │
       │  batchCompleted   │                   │                   │                   │
       │                   │                   │                   │                   │
       │  显示统计结果      │                   │                   │                   │
       │                   │                   │                   │                   │
       ▼                   ▼                   ▼                   ▼                   ▼
```

---

## 五、配置与扩展

### 5.1 配置文件结构

```json
{
    "batchProcessing": {
        "maxConcurrentRequests": 1,
        "requestTimeout": 60000,
        "maxRetryCount": 3,
        "retryDelay": 2000,
        "requestInterval": 1000,
        "skipExisting": true,
        "enableCheckpoint": true
    },
    "functionParser": {
        "supportedLanguages": ["cpp", "python", "java", "javascript", "typescript"],
        "minFunctionLines": 3,
        "maxFunctionLines": 500
    },
    "cache": {
        "enabled": true,
        "maxSize": 100
    }
}
```

### 5.2 扩展性设计

系统采用策略模式设计，便于扩展：

1. **新增语言支持**：只需实现新的解析器并注册到 `FunctionParser`
2. **新增AI服务**：继承 `AIProvider` 基类实现新接口
3. **新增存储后端**：继承 `StorageBackend` 基类实现新存储

---

## 六、实施建议

### 6.1 实施优先级

| 优先级 | 模块 | 说明 |
|--------|------|------|
| P0 | FunctionParser | 核心功能，必须首先实现 |
| P0 | BatchProcessManager | 核心功能，协调处理流程 |
| P1 | AIServiceManager增强 | 请求队列和速率限制 |
| P1 | DatabaseManager扩展 | 批量操作和状态管理 |
| P2 | UI更新 | 进度展示和交互优化 |
| P2 | 错误处理 | 完善异常处理机制 |
| P3 | 性能优化 | 缓存和并发控制 |

### 6.2 测试要点

1. **单元测试**：各解析器的函数提取准确性
2. **集成测试**：批量处理流程的完整性
3. **性能测试**：大文件（1000+函数）的处理效率
4. **异常测试**：网络故障、AI服务不可用等场景

---

## 七、文件结构

新增文件结构如下：

```
src/
├── core/
│   ├── parser/
│   │   ├── functionparser.h          # 函数解析器头文件
│   │   └── FunctionParser.cpp        # 函数解析器实现
│   ├── batch/
│   │   ├── batchprocessmanager.h     # 批量处理管理器头文件
│   │   └── BatchProcessManager.cpp   # 批量处理管理器实现
│   ├── models/
│   │   ├── functiondata.h            # 函数数据模型（已存在，需扩展）
│   │   ├── extractedfunction.h       # 提取函数数据模型（新增）
│   │   └── batchconfig.h             # 批量处理配置模型（新增）
│   ├── database/
│   │   ├── databasemanager.h         # 数据库管理器（已存在，需扩展）
│   │   └── DatabaseManager.cpp       # 数据库管理器实现（需扩展）
│   └── ai/
│       ├── aiservicemanager.h        # AI服务管理器（已存在，需扩展）
│       └── AIServiceManager.cpp      # AI服务管理器实现（需扩展）
└── ui/
    └── mainwindow/
        ├── FunctionalityWidget.h     # 功能组件（已存在，需更新）
        └── FunctionalityWidget.cpp   # 功能组件实现（需更新）
```
