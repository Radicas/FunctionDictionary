#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
FunctionDictionary Windows 打包工具 (Python 版本)
根据 build.py 中的路径结构进行打包
"""

import os
import sys
import subprocess
import argparse
import shutil
import zipfile
from pathlib import Path
from datetime import datetime

# 定义颜色输出
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'  # No Color

    @classmethod
    def disable(cls):
        """禁用颜色输出"""
        cls.RED = ''
        cls.GREEN = ''
        cls.YELLOW = ''
        cls.BLUE = ''
        cls.CYAN = ''
        cls.NC = ''

# 检查是否在 Windows 平台
if os.name == 'nt':
    Colors.disable()

def find_qt_path():
    """查找 Qt 安装路径"""
    # 首先检查环境变量
    qt_path = os.environ.get('Qt6_DIR')
    if qt_path:
        print(f"[DEBUG] 从环境变量获取 Qt6_DIR: {qt_path}")
        # 尝试不同的路径层次
        for i in range(1, 5):
            test_path = Path(qt_path)
            for _ in range(i):
                test_path = test_path.parent
            if test_path.exists() and (test_path / 'bin' / 'windeployqt.exe').exists():
                print(f"[DEBUG] 找到 Qt 路径: {test_path}")
                return test_path
    
    # 检查默认安装路径
    default_paths = [
        r'C:\Qt\6.10.2\msvc2022_64',
        r'C:\Qt\6.9.0\msvc2022_64',
        r'C:\Qt\6.8.1\msvc2022_64',
        r'C:\Qt\6.8.0\msvc2022_64',
        r'C:\Qt\6.7.2\msvc2019_64',
        r'C:\Qt\6.7.1\msvc2019_64',
        r'C:\Qt\6.6.3\msvc2019_64',
        r'D:\Qt\6.10.2\msvc2022_64'
    ]
    
    for path in default_paths:
        qt_path = Path(path)
        if qt_path.exists() and (qt_path / 'bin' / 'windeployqt.exe').exists():
            print(f"[DEBUG] 从默认路径找到 Qt: {qt_path}")
            return qt_path
    
    print("[DEBUG] 未找到 Qt 安装")
    return None

def get_timestamp():
    """获取时间戳"""
    now = datetime.now()
    return now.strftime('%Y%m%d_%H%M%S')

def create_zip_archive(source_dir, output_file):
    """创建 ZIP 归档文件"""
    with zipfile.ZipFile(output_file, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, source_dir)
                zipf.write(file_path, arcname)

def main():
    """主函数"""
    # 解析命令行参数
    parser = argparse.ArgumentParser(
        description='FunctionDictionary Windows 打包工具',
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        '--config', '-c',
        choices=['Debug', 'Release'],
        default='Release',
        help='构建配置 (默认: Release)'
    )
    parser.add_argument(
        '--qt-path', '-q',
        help='Qt 安装路径'
    )
    parser.add_argument(
        '--output', '-o',
        help='输出目录 (默认: dist)'
    )
    
    args = parser.parse_args()
    
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print(f"{Colors.BLUE}  FunctionDictionary Windows Package{Colors.NC}")
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print()
    
    # 项目根目录
    script_path = Path(__file__)
    project_root = script_path.parent.parent
    
    # 构建配置
    build_config = args.config
    
    # 构建目录
    # 根据 build.py 中的路径结构，构建目录为 project_root / 'build'
    # 可执行文件路径需要根据实际情况调整
    build_dir = project_root / 'build'
    exe_file = build_dir / 'src' / 'app' / build_config / 'FunctionDB.exe'
    
    print(f"{Colors.YELLOW}构建配置:{Colors.NC} {build_config}")
    print(f"{Colors.YELLOW}可执行文件路径:{Colors.NC} {exe_file}")
    
    # 检查可执行文件是否存在
    if not exe_file.exists():
        print(f"{Colors.RED}错误: 可执行文件未找到: {exe_file}{Colors.NC}")
        print()
        print(f"{Colors.YELLOW}请先构建项目:{Colors.NC}")
        print(f"  python scripts/build.py --build-type {build_config}")
        return 1
    
    print(f"{Colors.GREEN}✅ 找到可执行文件: {exe_file}{Colors.NC}")
    
    # 查找 Qt 安装路径
    qt_path = args.qt_path
    if qt_path:
        qt_path = Path(qt_path)
        print(f"[DEBUG] 使用用户指定的 Qt 路径: {qt_path}")
    else:
        qt_path = find_qt_path()
    
    # 验证 Qt 路径
    valid_qt_path = None
    if qt_path and qt_path.exists():
        # 检查 windeployqt 是否存在
        windeployqt = qt_path / 'bin' / 'windeployqt.exe'
        if windeployqt.exists():
            valid_qt_path = qt_path
        else:
            # 尝试向上查找
            for i in range(1, 5):
                test_path = qt_path
                for _ in range(i):
                    test_path = test_path.parent
                if test_path.exists() and (test_path / 'bin' / 'windeployqt.exe').exists():
                    valid_qt_path = test_path
                    print(f"[DEBUG] 调整 Qt 路径为: {valid_qt_path}")
                    break
    
    if not valid_qt_path:
        print(f"{Colors.RED}错误: 未找到有效的 Qt 安装{Colors.NC}")
        print()
        print(f"{Colors.YELLOW}请使用以下命令指定正确的 Qt 路径:{Colors.NC}")
        print(f"  python scripts/package_windows.py -q 'C:/Qt/6.x.x/msvc2022_64'")
        return 1
    
    qt_path = valid_qt_path
    print(f"{Colors.GREEN}✅ Qt 路径: {qt_path}{Colors.NC}")
    
    # 检查 windeployqt
    windeployqt = qt_path / 'bin' / 'windeployqt.exe'
    if not windeployqt.exists():
        print(f"{Colors.RED}错误: windeployqt 未找到: {windeployqt}{Colors.NC}")
        return 1
    
    print(f"{Colors.GREEN}✅ windeployqt: {windeployqt}{Colors.NC}")
    
    # 添加 Qt bin 到 PATH
    os.environ['PATH'] = f"{qt_path / 'bin'};{os.environ.get('PATH', '')}"
    
    # 输出目录
    output_dir = args.output
    if output_dir:
        output_dir = Path(output_dir)
    else:
        output_dir = project_root / 'dist'
    
    # 创建输出目录
    if not output_dir.exists():
        output_dir.mkdir(parents=True)
    
    # 打包目录
    timestamp = get_timestamp()
    package_name = 'FunctionDictionary'
    package_dir = output_dir / f"{package_name}_{build_config}_{timestamp}"
    
    print(f"{Colors.YELLOW}创建打包目录: {package_dir}{Colors.NC}")
    if not package_dir.exists():
        package_dir.mkdir(parents=True)
    
    # 复制可执行文件
    print(f"{Colors.YELLOW}复制可执行文件...{Colors.NC}")
    try:
        shutil.copy2(exe_file, package_dir)
        print(f"{Colors.GREEN}✅ 可执行文件复制成功{Colors.NC}")
    except Exception as e:
        print(f"{Colors.RED}错误: 复制可执行文件失败: {e}{Colors.NC}")
        return 1
    
    # 运行 windeployqt
    print(f"{Colors.YELLOW}收集 Qt 依赖...{Colors.NC}")
    windeployqt_args = [
        str(windeployqt),
        '--no-translations',
        '--no-system-d3d-compiler',
        '--no-opengl-sw',
        str(package_dir / 'FunctionDB.exe')
    ]
    
    if build_config == 'Release':
        windeployqt_args.insert(1, '--release')
    else:
        windeployqt_args.insert(1, '--debug')
    
    try:
        result = subprocess.run(
            windeployqt_args,
            cwd=package_dir,
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='replace'
        )
        if result.returncode != 0:
            print(f"{Colors.YELLOW}警告: windeployqt 可能有问题，继续执行...{Colors.NC}")
            print(result.stdout)
            print(result.stderr)
        else:
            print(f"{Colors.GREEN}✅ Qt 依赖收集成功{Colors.NC}")
    except Exception as e:
        print(f"{Colors.YELLOW}警告: 运行 windeployqt 失败: {e}，继续执行...{Colors.NC}")
    
    # 创建启动脚本
    print(f"{Colors.YELLOW}创建启动脚本...{Colors.NC}")
    launch_bat = package_dir / 'Launch.bat'
    try:
        with open(launch_bat, 'w', encoding='utf-8') as f:
            f.write('@echo off\n')
            f.write('start "" "FunctionDB.exe"\n')
        print(f"{Colors.GREEN}✅ 启动脚本创建成功{Colors.NC}")
    except Exception as e:
        print(f"{Colors.RED}错误: 创建启动脚本失败: {e}{Colors.NC}")
        return 1
    
    # 创建 README 文件
    print(f"{Colors.YELLOW}创建 README 文件...{Colors.NC}")
    readme_txt = package_dir / 'README.txt'
    try:
        with open(readme_txt, 'w', encoding='utf-8') as f:
            f.write('========================================\n')
            f.write('  FunctionDictionary\n')
            f.write('========================================\n')
            f.write('\n')
            f.write('Version: 1.0.0\n')
            f.write(f'Build: {build_config}\n')
            f.write('\n')
            f.write('[How to use]\n')
            f.write('  Double-click "Launch.bat" or "FunctionDB.exe"\n')
            f.write('\n')
            f.write('[System Requirements]\n')
            f.write('  - Windows 10 or later (64-bit)\n')
            f.write('  - If the program fails to start, install VC++ Redistributable:\n')
            f.write('    https://aka.ms/vs/17/release/vc_redist.x64.exe\n')
            f.write('\n')
            f.write('[Data Location]\n')
            f.write('  Database and logs are stored in:\n')
            f.write('  %APPDATA%\\FunctionDictionary\\\n')
            f.write('\n')
            f.write('========================================\n')
        print(f"{Colors.GREEN}✅ README 文件创建成功{Colors.NC}")
    except Exception as e:
        print(f"{Colors.RED}错误: 创建 README 文件失败: {e}{Colors.NC}")
        return 1
    
    # 创建 ZIP 包
    print(f"{Colors.YELLOW}创建 ZIP 包...{Colors.NC}")
    zip_file = output_dir / f"{package_name}_{build_config}_{timestamp}.zip"
    try:
        create_zip_archive(package_dir, zip_file)
        print(f"{Colors.GREEN}✅ ZIP 包创建成功{Colors.NC}")
    except Exception as e:
        print(f"{Colors.RED}错误: 创建 ZIP 包失败: {e}{Colors.NC}")
        return 1
    
    # 获取文件大小
    zip_size = zip_file.stat().st_size
    zip_size_mb = zip_size / (1024 * 1024)
    
    print()
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print(f"{Colors.BLUE}  Package Complete!{Colors.NC}")
    print(f"{Colors.BLUE}========================================{Colors.NC}")
    print()
    print(f"{Colors.YELLOW}打包详情:{Colors.NC}")
    print(f"  构建配置: {build_config}")
    print(f"  打包目录: {package_dir}")
    print(f"  ZIP 文件: {zip_file}")
    print(f"  文件大小: {zip_size_mb:.2f} MB")
    print()
    print(f"{Colors.YELLOW}客户使用说明:{Colors.NC}")
    print(f"  1. 解压 {zip_file.name}")
    print(f"  2. 运行 'Launch.bat' 或 'FunctionDB.exe'")
    print()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
