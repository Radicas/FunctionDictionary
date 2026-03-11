# CodeAtlas

<div align="center">

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/CodeAtlas)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey.svg)](https://github.com/yourusername/CodeAtlas)
[![C++](https://img.shields.io/badge/C++-17-orange.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.0%2B-brightgreen.svg)](https://www.qt.io/)

**智能化代码理解与知识管理工具**

[功能特性](#功能特性) • [快速开始](#快速开始) • [使用指南](#使用指南) • [API参考](#api参考) • [贡献指南](#贡献指南)

</div>

---

## 项目简介

**CodeAtlas** 是一个跨平台的智能化代码理解和知识管理工具，旨在帮助开发者快速理解开源项目、降低上手难度、积累代码知识资产。通过AI技术自动分析代码函数，生成易懂的说明文档，并将知识持久化存储，形成个人知识库。

### 核心价值

- 🚀 **快速理解**：通过AI自动分析代码，生成易懂的说明文档
- 📊 **可视化呈现**：自动生成架构图、流程图、关系图
- 💾 **知识沉淀**：将理解的知识持久化存储，形成个人知识库
- 🔍 **智能检索**：快速查找和复用已有知识

## 功能特性

### 核心功能

- ✅ **函数数据库管理**：SQLite数据库的CRUD操作，支持批量处理
- ✅ **AI智能分析**：集成AI服务，自动分析代码函数并生成文档
- ✅ **多语言解析**：支持C/C++、Python、Java、JavaScript等多种编程语言
- ✅ **Markdown渲染**：使用WebEngine渲染Markdown格式的函数说明
- ✅ **批量处理**：任务队列管理、进度追踪、错误处理、断点续传
- ✅ **主题管理**：支持Modern、Industrial、Cyberpunk等多种主题
- ✅ **跨平台支持**：支持macOS、Windows、Linux三大平台
- ✅ **C API接口**：提供C语言API，支持Python等其他语言调用

### 技术亮点

- 🏗️ **分层架构**：清晰的分层设计，遵循SOLID原则
- 🔌 **插件化设计**：高度模块化，易于扩展
- 📝 **完善日志**：跨平台彩色日志输出，支持文件日志和分级管理
- 🎨 **现代UI**：基于Qt6的现代图形用户界面
- ⚡ **异步处理**：高效的异步处理机制，提升用户体验

## 技术栈

### 核心技术

- **编程语言**：C++17
- **GUI框架**：Qt6 (Core, Widgets, Sql, Network, Concurrent, WebEngineWidgets)
- **数据库**：SQLite3
- **构建系统**：CMake 3.16+
- **渲染引擎**：QWebEngineView + Marked.js + Highlight.js

### 支持平台

| 平台 | 编译器 | 状态 |
|------|--------|------|
| macOS | Clang / GCC | ✅ 支持 |
| Windows | MSVC / MinGW | ✅ 支持 |
| Linux | GCC / Clang | ✅ 支持 |

## 快速开始

### 前置要求

- **CMake** 3.16 或更高版本
- **Qt6** 6.0 或更高版本（需要以下模块）
  - Core
  - Widgets
  - Sql
  - Network
  - Concurrent
  - WebEngineWidgets
  - Positioning
  - Quick
  - Qml
  - WebChannel
- **C++17兼容编译器**
  - GCC 7+
  - Clang 5+
  - MSVC 2017+

### 安装依赖

#### macOS

```bash
# 使用 Homebrew 安装 Qt6
brew install qt@6

# 设置 Qt6 路径（如果需要）
export CMAKE_PREFIX_PATH="/usr/local/opt/qt@6"
```

#### Windows

1. 从 [Qt官网](https://www.qt.io/download) 下载并安装Qt6
2. 安装 Visual Studio 2019 或更高版本（推荐）或 MinGW
3. 设置环境变量 `CMAKE_PREFIX_PATH` 指向Qt6安装目录

#### Linux (Ubuntu/Debian)

```bash
# 安装 Qt6 和构建工具
sudo apt-get update
sudo apt-get install qt6-base-dev qt6-webengine-dev cmake build-essential
```

### 编译项目

#### macOS / Linux

```bash
# 克隆仓库
git clone https://github.com/yourusername/CodeAtlas.git
cd CodeAtlas

# 创建构建目录
mkdir build
cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 运行
./FunctionDB
```

#### Windows (MSVC)

```powershell
# 克隆仓库
git clone https://github.com/yourusername/CodeAtlas.git
cd CodeAtlas

# 创建构建目录
mkdir build
cd build

# 配置项目（使用 Visual Studio 2019）
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.0.0/msvc2019_64"

# 编译
cmake --build . --config Release

# 运行
.\Release\FunctionDB.exe
```

#### Windows (MinGW)

```powershell
# 使用 MinGW 编译
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.0.0/mingw81_64"
mingw32-make
```

详细的Windows构建指南请参考 [MSVC构建指南](docs/MSVC构建指南.md)。

### 运行程序

编译成功后，可执行文件位于：

- **macOS**: `build/FunctionDB.app`
- **Windows**: `build/Release/FunctionDB.exe`
- **Linux**: `build/FunctionDB`

首次运行时，程序会在应用数据目录自动创建数据库和日志文件：

- **macOS**: `~/Library/Application Support/CodeAtlas/`
- **Windows**: `%APPDATA%/CodeAtlas/`
- **Linux**: `~/.local/share/CodeAtlas/`

## 使用指南

### 基本操作

1. **启动程序**
   ```bash
   ./FunctionDB
   ```

2. **配置AI服务**
   - 点击菜单 `设置` → `AI配置`
   - 选择AI服务提供商
   - 输入API密钥和基础URL
   - 选择模型ID

3. **添加函数**
   - 点击工具栏 `增加` 按钮
   - 输入函数名称和说明
   - 点击确定保存

4. **查看函数**
   - 在左侧函数列表中选择函数
   - 右侧显示Markdown格式的详细说明

5. **删除函数**
   - 选择要删除的函数
   - 点击工具栏 `删除` 按钮
   - 确认删除操作

### 高级功能

#### 批量分析

1. 点击菜单 `文件` → `批量分析`
2. 选择要分析的源代码文件或目录
3. 配置解析选项
4. 点击开始分析
5. 查看进度和结果

#### 主题切换

点击菜单 `视图` → `主题`，选择喜欢的主题：
- Modern（现代风格）
- Industrial（工业风格）
- Cyberpunk（赛博朋克风格）

### 数据模型

#### FunctionData 结构

```cpp
struct FunctionData {
    int id;                     // 函数ID
    QString key;                // 函数名称（唯一标识）
    QString value;              // 函数介绍（Markdown格式）
    QDateTime createTime;       // 创建时间
    QString signature;          // 函数签名
    QString returnType;         // 返回类型
    QString parameters;         // 参数列表（JSON格式）
    QString filePath;           // 源文件路径
    int startLine;              // 起始行号
    int endLine;                // 结束行号
    QString language;           // 编程语言
    QString flowchart;          // 流程图
    QString sequenceDiagram;    // 时序图
    QString structureDiagram;   // 结构图
    QString aiModel;            // 分析使用的AI模型
    QDateTime analyzeTime;      // 分析时间
};
```

## API参考

### C API接口

CodeAtlas提供C语言API，支持跨语言调用：

```c
#include "api/function_dict_c_api.h"

// 创建函数数据库句柄
FunctionDictHandle fd_create();

// 销毁句柄
void fd_destroy(FunctionDictHandle handle);

// 添加函数
int fd_add_function(FunctionDictHandle handle, 
                    const char* key, 
                    const char* value);

// 获取函数
const char* fd_get_function(FunctionDictHandle handle, 
                            const char* key);

// 删除函数
int fd_delete_function(FunctionDictHandle handle, 
                       const char* key);

// 获取所有函数数量
int fd_get_count(FunctionDictHandle handle);
```

详细API文档请参考 [API文档](docs/API.md)。

### Python绑定示例

```python
from ctypes import *

# 加载动态库
lib = cdll.LoadLibrary('./libfunctiondict.so')

# 创建句柄
handle = lib.fd_create()

# 添加函数
lib.fd_add_function(handle, b'my_function', b'This is my function')

# 获取函数
result = lib.fd_get_function(handle, b'my_function')
print(result.decode('utf-8'))

# 销毁句柄
lib.fd_destroy(handle)
```

## 项目架构

### 分层架构

```
┌─────────────────────────────────────────────────────┐
│              Presentation Layer (UI)                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │  MainWindow  │  │  Dialogs     │  │  Widgets │  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
├─────────────────────────────────────────────────────┤
│            Application Layer (Services)             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │ ParseService │  │BatchProcess  │  │AIService │  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
├─────────────────────────────────────────────────────┤
│              Domain Layer (Core Logic)              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │ CodeParser   │  │AIServiceMgr  │  │Database  │  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
├─────────────────────────────────────────────────────┤
│          Infrastructure Layer (Foundation)          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  │
│  │ Logger       │  │ThemeManager  │  │VersionMgr│  │
│  └──────────────┘  └──────────────┘  └──────────┘  │
└─────────────────────────────────────────────────────┘
```

### 目录结构

```
CodeAtlas/
├── src/
│   ├── api/                    # C语言API接口
│   ├── app/                    # 应用程序入口
│   ├── common/                 # 公共模块
│   │   ├── logger/            # 日志系统
│   │   ├── theme/             # 主题管理
│   │   └── utils/             # 工具类
│   ├── core/                   # 核心业务逻辑
│   │   ├── ai/                # AI服务管理
│   │   ├── batch/             # 批量处理
│   │   ├── database/          # 数据库管理
│   │   ├── interfaces/        # 接口定义
│   │   ├── models/            # 数据模型
│   │   ├── parser/            # 代码解析器
│   │   └── services/          # 业务服务
│   └── ui/                     # 用户界面
│       ├── dialogs/           # 对话框
│       ├── mainwindow/        # 主窗口
│       └── markdown/          # Markdown渲染
├── docs/                       # 文档
│   ├── 技术架构设计.md
│   ├── 开发路线图.md
│   ├── VERSIONING.md
│   └── MSVC构建指南.md
├── scripts/                    # 构建脚本
└── CMakeLists.txt             # CMake配置
```

详细的架构设计请参考 [技术架构设计](docs/技术架构设计.md)。

## 开发路线图

### 当前版本 v1.0.0

- ✅ 基础数据库管理
- ✅ AI配置管理
- ✅ Markdown渲染展示
- ✅ 单文件函数提取和分析
- ✅ 跨平台支持
- ✅ 主题管理

### 下一版本 v1.5.0

- 📁 项目级批量分析
- 🔍 自动扫描代码文件
- 🤖 批量AI分析
- 📊 进度可视化
- ⚡ 性能优化

### 未来版本 v2.0.0

- 🏗️ 可视化图表生成
- 🔗 模块关系图
- 📊 流程图和时序图
- 🎯 依赖关系分析

### 长期规划 v2.5.0+

- 🧠 智能知识管理
- 🔍 智能搜索引擎
- 💡 知识图谱构建
- 🤝 团队协作功能

详细的开发路线图请参考 [开发路线图](docs/开发路线图.md)。

## 贡献指南

我们欢迎所有形式的贡献！无论是报告Bug、提出新功能建议、改进文档还是提交代码。

### 如何贡献

1. **Fork 本仓库**
   ```bash
   git clone https://github.com/yourusername/CodeAtlas.git
   ```

2. **创建特性分支**
   ```bash
   git checkout -b feature/amazing-feature
   ```

3. **进行修改**
   - 遵循 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
   - 添加必要的单元测试
   - 更新相关文档

4. **提交更改**
   ```bash
   git commit -m 'Add some amazing feature'
   ```

5. **推送到分支**
   ```bash
   git push origin feature/amazing-feature
   ```

6. **创建 Pull Request**

### 代码规范

- 遵循 Google C++ Style Guide
- 使用 Doxygen 风格的中文注释
- 源文件名称统一使用小写字母
- 禁止使用相对路径引入头文件
- 所有公共接口都需要添加文档注释

### 提交信息规范

使用语义化提交信息：

- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建/工具相关

示例：
```
feat(ai): 添加批量分析功能
fix(database): 修复数据库连接泄漏问题
docs(readme): 更新安装指南
```

### 开发环境设置

1. 安装必要的开发工具
2. 配置代码格式化工具（推荐 clang-format）
3. 启用预提交钩子（可选）

## 常见问题

### 编译问题

**Q: 找不到Qt6模块？**

A: 确保已正确设置 `CMAKE_PREFIX_PATH` 环境变量指向Qt6安装目录。

**Q: WebEngineWidgets模块找不到？**

A: WebEngineWidgets在Qt 6.10.2+作为插件分发，需要先安装依赖模块（Positioning, Quick, Qml, WebChannel）。

### 运行问题

**Q: 程序启动时提示数据库错误？**

A: 检查应用数据目录是否有写入权限，程序会自动创建数据库文件。

**Q: AI分析失败？**

A: 检查AI配置是否正确，确保API密钥有效且网络连接正常。

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

```
MIT License

Copyright (c) 2026 CodeAtlas Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## 联系方式

- **项目主页**: [https://github.com/yourusername/CodeAtlas](https://github.com/yourusername/CodeAtlas)
- **问题反馈**: [GitHub Issues](https://github.com/yourusername/CodeAtlas/issues)
- **功能建议**: [GitHub Discussions](https://github.com/yourusername/CodeAtlas/discussions)
- **邮件**: support@example.com

## 致谢

感谢以下开源项目：

- [Qt](https://www.qt.io/) - 跨平台应用框架
- [SQLite](https://www.sqlite.org/) - 嵌入式数据库
- [Marked.js](https://marked.js.org/) - Markdown解析器
- [Highlight.js](https://highlightjs.org/) - 代码高亮库

---

<div align="center">

**如果这个项目对您有帮助，请给一个 ⭐️ Star！**

Made with ❤️ by CodeAtlas Team

</div>
