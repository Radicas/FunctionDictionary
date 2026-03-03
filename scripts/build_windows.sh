#!/bin/bash

###############################################################################
# @file build_windows.sh
# @brief Windows交叉编译脚本 - 在macOS上使用mingw-w64编译Windows可执行文件
# @author FunctionDictionary Team
# @date 2026-03-02
# @version 1.0.0
#
# @description 本脚本用于在macOS环境下通过mingw-w64工具链交叉编译生成
#              可在Windows系统运行的程序。需要预先安装mingw-w64和Windows版Qt5。
#
# @usage ./build_windows.sh [选项]
#        -h, --help     显示帮助信息
#        -c, --clean    清理编译目录后重新编译
#        -r, --release  使用Release模式编译（默认）
#        -d, --debug    使用Debug模式编译
###############################################################################

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_windows"
TOOLCHAIN_FILE="${PROJECT_ROOT}/cmake/mingw-w64-x86_64.cmake"

# 编译配置
BUILD_TYPE="Release"
CLEAN_BUILD=false

# MinGW-w64 工具链前缀
MINGW_PREFIX="x86_64-w64-mingw32"
MINGW_GCC="${MINGW_PREFIX}-g++"
MINGW_GPP="${MINGW_PREFIX}-g++"
MINGW_STRIP="${MINGW_PREFIX}-strip"

# Qt5 Windows路径配置
# 优先级：环境变量 > local目录 > 用户配置
QT_VERSION="5.15.2"
QT_MINGW_VERSION="mingw81_64"
QT5_LOCAL_PATH="${PROJECT_ROOT}/local/qt5-windows/${QT_VERSION}/${QT_MINGW_VERSION}"

if [ -n "${QT5_WINDOWS_PATH}" ]; then
    QT5_PATH="${QT5_WINDOWS_PATH}"
elif [ -d "${QT5_LOCAL_PATH}" ]; then
    QT5_PATH="${QT5_LOCAL_PATH}"
else
    QT5_PATH=""
fi

# 日志函数
log_info() {
    echo -e "${BLUE}[信息]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[成功]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[警告]${NC} $1"
}

log_error() {
    echo -e "${RED}[错误]${NC} $1"
}

# 显示帮助信息
show_help() {
    cat << EOF
Windows交叉编译脚本

用法: $0 [选项]

选项:
    -h, --help     显示此帮助信息
    -c, --clean    清理编译目录后重新编译
    -r, --release  使用Release模式编译（默认）
    -d, --debug    使用Debug模式编译

环境变量:
    QT5_WINDOWS_PATH   Windows版Qt5的安装路径
                       例如: /path/to/Qt/5.15.2/mingw81_64

示例:
    $0                          # Release模式编译
    $0 -d                       # Debug模式编译
    $0 -c                       # 清理后重新编译
    QT5_WINDOWS_PATH=/path/to/qt $0  # 指定Qt5路径

注意:
    1. 需要预先安装mingw-w64工具链
       macOS: brew install mingw-w64

    2. 需要Windows版本的Qt5库
       可以从Qt官方下载: https://download.qt.io/archive/qt/
       选择对应的MinGW版本，例如: qt-opensource-windows-x86-5.15.2.exe
       解压后将lib、include等目录放置到指定位置

    3. 编译后的可执行文件需要在Windows系统上测试运行
EOF
}

# 解析命令行参数
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -r|--release)
                BUILD_TYPE="Release"
                shift
                ;;
            -d|--debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            *)
                log_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# 检查mingw-w64工具链
check_mingw_toolchain() {
    log_info "检查mingw-w64工具链..."

    if ! command -v ${MINGW_GCC} &> /dev/null; then
        log_error "未找到mingw-w64工具链"
        log_info "请使用以下命令安装:"
        echo "  brew install mingw-w64"
        exit 1
    fi

    local gcc_version=$(${MINGW_GCC} --version | head -n1)
    log_success "找到mingw-w64: ${gcc_version}"
}

# 检查Qt5 Windows版本
check_qt5_windows() {
    log_info "检查Qt5 Windows版本..."

    if [ -z "${QT5_PATH}" ]; then
        log_error "未找到Qt5 Windows路径"
        log_info ""
        log_info "请使用以下方法之一配置Qt5:"
        echo ""
        echo "方法一：使用下载脚本自动下载（推荐）"
        echo "  ./scripts/download_qt5_windows.sh"
        echo ""
        echo "方法二：手动安装到local目录"
        echo "  安装路径: ${QT5_LOCAL_PATH}"
        echo "  详细说明: ./scripts/download_qt5_windows.sh"
        echo ""
        echo "方法三：设置环境变量"
        echo "  export QT5_WINDOWS_PATH=/path/to/Qt/${QT_VERSION}/${QT_MINGW_VERSION}"
        echo "  ./scripts/build_windows.sh"
        exit 1
    fi

    if [ ! -d "${QT5_PATH}" ]; then
        log_error "Qt5 Windows路径不存在: ${QT5_PATH}"
        exit 1
    fi

    local qt_cmake_path="${QT5_PATH}/lib/cmake/Qt5/Qt5Config.cmake"
    if [ ! -f "${qt_cmake_path}" ]; then
        log_error "Qt5配置文件不存在: ${qt_cmake_path}"
        log_info "请确保Qt5路径正确，且包含完整的库文件"
        exit 1
    fi

    if [ "${QT5_PATH}" = "${QT5_LOCAL_PATH}" ]; then
        log_success "找到Qt5 Windows版本（local目录）: ${QT5_PATH}"
    else
        log_success "找到Qt5 Windows版本（环境变量）: ${QT5_PATH}"
    fi
}

# 创建CMake工具链文件
create_toolchain_file() {
    log_info "创建CMake工具链文件..."

    local toolchain_dir="${PROJECT_ROOT}/cmake"
    mkdir -p "${toolchain_dir}"

    cat > "${TOOLCHAIN_FILE}" << 'EOF'
# CMake工具链文件 - mingw-w64交叉编译
# 目标系统: Windows x86_64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 编译器设置
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# 目标环境设置
set(CMAKE_FIND_ROOT_PATH /usr/local/opt/mingw-w64)

# 搜索路径设置
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows特定设置
set(WIN32 TRUE)
set(MINGW TRUE)

# 静态链接运行时库（可选）
# set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static" CACHE STRING "" FORCE)
EOF

    log_success "工具链文件已创建: ${TOOLCHAIN_FILE}"
}

# 清理编译目录
clean_build() {
    if [ "${CLEAN_BUILD}" = true ]; then
        log_info "清理编译目录..."
        rm -rf "${BUILD_DIR}"
        log_success "编译目录已清理"
    fi
}

# 配置CMake
configure_cmake() {
    log_info "配置CMake..."

    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"

    local cmake_args=(
        -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        -DCMAKE_PREFIX_PATH="${QT5_PATH}"
        -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}/install"
        -DQt5_DIR="${QT5_PATH}/lib/cmake/Qt5"
        -DQt5Core_DIR="${QT5_PATH}/lib/cmake/Qt5Core"
        -DQt5Widgets_DIR="${QT5_PATH}/lib/cmake/Qt5Widgets"
        -DQt5Sql_DIR="${QT5_PATH}/lib/cmake/Qt5Sql"
        -DQt5Network_DIR="${QT5_PATH}/lib/cmake/Qt5Network"
        -DQt5Concurrent_DIR="${QT5_PATH}/lib/cmake/Qt5Concurrent"
        -DQT_QMAKE_EXECUTABLE="${QT5_PATH}/bin/qmake"
    )

    cmake "${PROJECT_ROOT}" "${cmake_args[@]}"

    if [ $? -ne 0 ]; then
        log_error "CMake配置失败"
        exit 1
    fi

    log_success "CMake配置完成"
}

# 编译项目
build_project() {
    log_info "编译项目 (${BUILD_TYPE}模式)..."

    cd "${BUILD_DIR}"

    local cores=$(sysctl -n hw.ncpu)
    log_info "使用 ${cores} 个核心进行编译"

    cmake --build . --config "${BUILD_TYPE}" -- -j${cores}

    if [ $? -ne 0 ]; then
        log_error "编译失败"
        exit 1
    fi

    log_success "编译完成"
}

# 优化可执行文件
optimize_executable() {
    log_info "优化可执行文件..."

    local exe_file="${BUILD_DIR}/src/app/FunctionDB.exe"

    if [ ! -f "${exe_file}" ]; then
        log_warning "未找到可执行文件: ${exe_file}"
        return
    fi

    # 剥离调试符号（减小文件大小）
    if command -v ${MINGW_STRIP} &> /dev/null; then
        ${MINGW_STRIP} "${exe_file}"
        log_success "已剥离调试符号"
    fi

    # 显示文件信息
    local file_size=$(ls -lh "${exe_file}" | awk '{print $5}')
    log_success "可执行文件大小: ${file_size}"
    log_success "可执行文件路径: ${exe_file}"
}

# 收集依赖文件
collect_dependencies() {
    log_info "收集依赖文件..."

    local deploy_dir="${BUILD_DIR}/deploy"
    mkdir -p "${deploy_dir}"

    # 复制可执行文件
    if [ -f "${BUILD_DIR}/src/app/FunctionDB.exe" ]; then
        cp "${BUILD_DIR}/src/app/FunctionDB.exe" "${deploy_dir}/"
        log_success "已复制可执行文件到部署目录"
    fi

    # 提示用户关于Qt依赖
    log_warning "注意: 可执行文件需要Qt5的DLL文件才能在Windows上运行"
    log_info "必需的Qt5 DLL文件（位于${QT5_PATH}/bin/）:"
    echo "  - Qt5Core.dll"
    echo "  - Qt5Gui.dll"
    echo "  - Qt5Widgets.dll"
    echo "  - Qt5Sql.dll"
    echo "  - Qt5Network.dll"
    echo "  - Qt5Concurrent.dll"
    echo "  - platforms/qwindows.dll"
    echo ""
    log_info "建议使用windeployqt工具自动收集依赖:"
    echo "  在Windows系统上运行:"
    echo "  windeployqt --release FunctionDB.exe"

    log_success "部署目录: ${deploy_dir}"
}

# 显示编译摘要
show_summary() {
    echo ""
    echo "========================================"
    log_success "Windows交叉编译完成！"
    echo "========================================"
    echo ""
    log_info "编译信息:"
    echo "  编译类型: ${BUILD_TYPE}"
    echo "  编译目录: ${BUILD_DIR}"
    echo "  可执行文件: ${BUILD_DIR}/src/app/FunctionDB.exe"
    echo "  部署目录: ${BUILD_DIR}/deploy"
    echo ""
    log_warning "运行提示:"
    echo "  1. 将可执行文件复制到Windows系统"
    echo "  2. 确保包含必要的Qt5 DLL文件"
    echo "  3. 或在Windows上使用windeployqt工具收集依赖"
    echo ""
}

# 主函数
main() {
    echo ""
    echo "========================================"
    echo "  Windows交叉编译脚本"
    echo "========================================"
    echo ""

    parse_arguments "$@"

    check_mingw_toolchain
    check_qt5_windows
    create_toolchain_file
    clean_build
    configure_cmake
    build_project
    optimize_executable
    collect_dependencies
    show_summary
}

# 执行主函数
main "$@"
