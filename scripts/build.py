#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
项目 - 构建工具 (Python 版本)
默认使用 MSVC 编译，未来支持 MingW
"""

import os
import sys
import subprocess
import argparse
import shutil
from pathlib import Path

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

def check_compiler_available(compiler):
    """检查编译器是否可用"""
    if compiler == 'msvc':
        # 1. 首先检查环境变量中的 cl.exe
        try:
            # 尝试运行 cl.exe 查看版本
            result = subprocess.run(
                ['cl.exe'],
                capture_output=True,
                text=True
            )
            # MSVC 的 cl.exe 在没有参数时会返回错误码，但会输出版本信息
            # 注意：版本信息可能在 stdout 或 stderr 中
            output = result.stdout + result.stderr
            if 'Microsoft (R) C/C++' in output:
                return True
        except FileNotFoundError:
            pass
        
        # 2. 检查默认安装路径
        default_paths = [
            r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC',
            r'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC',
            r'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC'
        ]
        
        for base_path in default_paths:
            if os.path.exists(base_path):
                # 查找最新版本的 MSVC
                try:
                    versions = os.listdir(base_path)
                    # 按版本号排序，取最新的
                    versions.sort(reverse=True)
                    for version in versions:
                        cl_path = os.path.join(base_path, version, 'bin', 'Hostx64', 'x64', 'cl.exe')
                        if os.path.exists(cl_path):
                            # 尝试运行找到的 cl.exe
                            result = subprocess.run(
                                [cl_path],
                                capture_output=True,
                                text=True
                            )
                            # 注意：版本信息可能在 stdout 或 stderr 中
                            output = result.stdout + result.stderr
                            if 'Microsoft (R) C/C++' in output:
                                return True
                except Exception:
                    pass
        
        return False
    
    elif compiler == 'mingw':
        # 1. 首先检查环境变量中的 g++
        try:
            result = subprocess.run(
                ['g++', '--version'],
                capture_output=True,
                text=True,
                check=True
            )
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass
        
        # 2. 检查默认安装路径
        default_paths = [
            r'C:\MinGW\bin',
            r'C:\msys64\mingw64\bin',
            r'C:\Program Files\MinGW\bin'
        ]
        
        for path in default_paths:
            gpp_path = os.path.join(path, 'g++.exe')
            if os.path.exists(gpp_path):
                try:
                    result = subprocess.run(
                        [gpp_path, '--version'],
                        capture_output=True,
                        text=True,
                        check=True
                    )
                    return True
                except (subprocess.CalledProcessError, FileNotFoundError):
                    pass
        
        return False
    
    return False

def find_msvc_path():
    """查找 MSVC 编译器路径"""
    # 检查默认安装路径
    default_paths = [
        r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC',
        r'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC',
        r'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC'
    ]
    
    for base_path in default_paths:
        if os.path.exists(base_path):
            # 查找最新版本的 MSVC
            try:
                versions = os.listdir(base_path)
                # 按版本号排序，取最新的
                versions.sort(reverse=True)
                for version in versions:
                    cl_path = os.path.join(base_path, version, 'bin', 'Hostx64', 'x64', 'cl.exe')
                    if os.path.exists(cl_path):
                        return os.path.dirname(cl_path)
            except Exception:
                pass
    return None

def find_mingw_path():
    """查找 MingW 编译器路径"""
    # 检查默认安装路径
    default_paths = [
        r'C:\MinGW\bin',
        r'C:\msys64\mingw64\bin',
        r'C:\Program Files\MinGW\bin'
    ]
    
    for path in default_paths:
        gpp_path = os.path.join(path, 'g++.exe')
        if os.path.exists(gpp_path):
            return path
    return None

def get_cmake_generator(compiler):
    """根据编译器获取 CMake 生成器"""
    if compiler == 'msvc':
        # 默认使用 Visual Studio 生成器（自动检测版本）
        return 'Visual Studio 17 2022'  # 可根据实际情况调整
    elif compiler == 'mingw':
        return 'MinGW Makefiles'
    return ''

def main():
    """主函数"""
    # 解析命令行参数
    parser = argparse.ArgumentParser(
        description='RedCGL 构建工具',
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        '--compiler', '-c',
        choices=['msvc', 'mingw'],
        default='msvc',
        help='选择编译器 (默认: msvc)'
    )
    parser.add_argument(
        '--build-type', '-b',
        choices=['Debug', 'Release'],
        default='Debug',
        help='构建类型 (默认: Debug)'
    )
    parser.add_argument(
        '--clean', '-C',
        action='store_true',
        help='清理构建目录'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='显示详细信息'
    )
    
    args = parser.parse_args()
    
    print(f"{Colors.BLUE}=== RedCGL 构建工具 ==={Colors.NC}")
    
    # 项目根目录
    script_path = Path(__file__)
    project_root = script_path.parent.parent
    build_dir = project_root / 'build'
    
    print(f"{Colors.YELLOW}项目根目录:{Colors.NC} {project_root}")
    print(f"{Colors.YELLOW}构建目录:{Colors.NC} {build_dir}")
    
    # 清理构建目录
    if args.clean:
        print(f"{Colors.YELLOW}清理构建目录...{Colors.NC}")
        if build_dir.exists():
            try:
                shutil.rmtree(build_dir)
                print(f"{Colors.GREEN}✅ 构建目录已清理{Colors.NC}")
            except Exception as e:
                print(f"{Colors.RED}错误: 清理构建目录失败: {e}{Colors.NC}")
                return 1
        else:
            print(f"{Colors.YELLOW}构建目录不存在，无需清理{Colors.NC}")
        # 清理后退出
        return 0
    
    # 检查编译器是否可用
    if not check_compiler_available(args.compiler):
        print(f"{Colors.RED}错误: {args.compiler.upper()} 编译器未找到{Colors.NC}")
        if args.compiler == 'msvc':
            print(f"{Colors.YELLOW}请确保已安装 Visual Studio 并在正确的环境中运行此脚本{Colors.NC}")
            print(f"{Colors.YELLOW}或在 Visual Studio 命令提示符中执行此脚本{Colors.NC}")
        elif args.compiler == 'mingw':
            print(f"{Colors.YELLOW}请确保已安装 MingW 并将其添加到系统 PATH 中{Colors.NC}")
        return 1
    
    # 查找编译器路径并添加到环境变量
    if args.compiler == 'msvc':
        msvc_path = find_msvc_path()
        if msvc_path:
            # 检查是否已经在环境变量中
            if msvc_path not in os.environ.get('PATH', ''):
                os.environ['PATH'] = f"{msvc_path};{os.environ.get('PATH', '')}"
                print(f"{Colors.GREEN}已将 MSVC 路径添加到环境变量: {msvc_path}{Colors.NC}")
    elif args.compiler == 'mingw':
        mingw_path = find_mingw_path()
        if mingw_path:
            # 检查是否已经在环境变量中
            if mingw_path not in os.environ.get('PATH', ''):
                os.environ['PATH'] = f"{mingw_path};{os.environ.get('PATH', '')}"
                print(f"{Colors.GREEN}已将 MingW 路径添加到环境变量: {mingw_path}{Colors.NC}")
    
    # 获取 CMake 生成器
    cmake_generator = get_cmake_generator(args.compiler)
    if not cmake_generator:
        print(f"{Colors.RED}错误: 不支持的编译器: {args.compiler}{Colors.NC}")
        return 1
    
    print(f"{Colors.CYAN}使用编译器:{Colors.NC} {args.compiler.upper()}")
    print(f"{Colors.CYAN}CMake 生成器:{Colors.NC} {cmake_generator}")
    print(f"{Colors.CYAN}构建类型:{Colors.NC} {args.build_type}")
    
    # 检查构建目录
    if build_dir.exists():
        print(f"{Colors.YELLOW}构建目录已存在，尝试增量编译...{Colors.NC}")
        # 尝试进行增量编译
        try:
            test_build_cmd = [
                'cmake',
                '--build', '.',
                '--config', args.build_type,
                '--parallel'
            ]
            # 执行增量编译并显示详细信息
            print(f"\n{Colors.YELLOW}开始增量构建...{Colors.NC}")
            result = subprocess.run(
                test_build_cmd,
                cwd=build_dir,
                capture_output=False,  # 直接显示输出
                text=True,
                encoding='utf-8',
                errors='replace',
                check=True
            )
            print(f"\n{Colors.GREEN}✅ 增量编译成功！{Colors.NC}")
            return 0
        except subprocess.CalledProcessError:
            print(f"{Colors.YELLOW}增量编译失败，删除构建目录并重新编译...{Colors.NC}")
            try:
                shutil.rmtree(build_dir)
                print(f"{Colors.GREEN}✅ 构建目录已删除{Colors.NC}")
            except Exception as e:
                print(f"{Colors.RED}错误: 删除构建目录失败: {e}{Colors.NC}")
                return 1
    
    # 创建构建目录
    print(f"{Colors.YELLOW}创建构建目录...{Colors.NC}")
    try:
        build_dir.mkdir(parents=True)
        print(f"{Colors.GREEN}✅ 构建目录已创建{Colors.NC}")
    except Exception as e:
        print(f"{Colors.RED}错误: 创建构建目录失败: {e}{Colors.NC}")
        return 1
    
    # 执行 CMake 配置
    print(f"\n{Colors.YELLOW}执行 CMake 配置...{Colors.NC}")
    cmake_config_cmd = [
        'cmake',
        '-G', cmake_generator,
        f'-DCMAKE_BUILD_TYPE={args.build_type}',
        str(project_root)
    ]
    
    if args.verbose:
        print(f"{Colors.CYAN}执行命令:{Colors.NC} {' '.join(cmake_config_cmd)}")
    
    try:
        result = subprocess.run(
            cmake_config_cmd,
            cwd=build_dir,
            capture_output=not args.verbose,
            text=True,
            encoding='utf-8',
            errors='replace',
            check=True
        )
        if args.verbose:
            print(result.stdout)
        print(f"{Colors.GREEN}✅ CMake 配置成功{Colors.NC}")
    except subprocess.CalledProcessError as e:
        print(f"{Colors.RED}错误: CMake 配置失败{Colors.NC}")
        if not args.verbose:
            if hasattr(e, 'stdout') and e.stdout:
                print(e.stdout)
            if hasattr(e, 'stderr') and e.stderr:
                print(e.stderr)
        return 1
    
    # 执行构建
    print(f"\n{Colors.YELLOW}开始构建项目...{Colors.NC}")
    cmake_build_cmd = [
        'cmake',
        '--build', '.',
        '--config', args.build_type,
        '--parallel'  # 使用并行构建
    ]
    
    if args.verbose:
        print(f"{Colors.CYAN}执行命令:{Colors.NC} {' '.join(cmake_build_cmd)}")
    
    # 总是显示构建过程的信息
    try:
        result = subprocess.run(
            cmake_build_cmd,
            cwd=build_dir,
            capture_output=False,  # 直接显示输出
            text=True,
            encoding='utf-8',
            errors='replace',
            check=True
        )
        print(f"\n{Colors.GREEN}✅ 项目构建成功！{Colors.NC}")
    except subprocess.CalledProcessError as e:
        print(f"{Colors.RED}错误: 项目构建失败{Colors.NC}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
