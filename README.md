# 函数数据库管理系统

一个跨平台的函数数据库管理系统，使用 C++ 和 Qt5 开发，支持 macOS 和 Windows 操作系统。

## 功能特性

- 左侧函数列表，支持点击选择
- 右侧 Markdown 格式的函数介绍展示
- 增加和删除函数功能
- SQLite 数据库持久化存储
- 完整的日志系统
- 跨平台支持（macOS 和 Windows）

## 技术栈

- C++17
- Qt5 (Core, Widgets, Sql)
- SQLite3
- CMake 构建系统

## 项目结构

```
.
├── CMakeLists.txt      # CMake 构建配置
├── README.md           # 项目说明
└── src/                # 源代码目录
    ├── main.cpp        # 主程序入口
    ├── mainwindow.h    # 主窗口头文件
    ├── mainwindow.cpp  # 主窗口实现
    ├── addfunctiondialog.h    # 增加对话框头文件
    ├── addfunctiondialog.cpp  # 增加对话框实现
    ├── databasemanager.h      # 数据库管理头文件
    ├── databasemanager.cpp    # 数据库管理实现
    ├── logger.h        # 日志系统头文件
    └── logger.cpp      # 日志系统实现
```

## 编译指南

### 前置要求

- CMake 3.16 或更高版本
- Qt5 开发库 (Core, Widgets, Sql 模块)
- C++17 兼容的编译器 (GCC, Clang, MSVC)

### macOS 编译

```bash
mkdir build
cd build
cmake ..
make
```

### Windows 编译

使用 Visual Studio 开发者命令提示符：

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## 使用说明

1. 启动程序后，会在应用数据目录自动创建数据库和日志文件
2. 点击"增加"按钮添加新函数
3. 在左侧列表选择函数查看详情
4. 选择函数后点击"删除"按钮移除函数

## 数据库表结构

```sql
CREATE TABLE functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    key TEXT NOT NULL UNIQUE,
    value TEXT NOT NULL,
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);
```

## 许可证

MIT License
