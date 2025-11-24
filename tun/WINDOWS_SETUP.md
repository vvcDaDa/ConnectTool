# Windows Wintun 设置指南

本项目在 Windows 上使用 Wintun 驱动来实现高性能的 TUN 虚拟网络接口。

## 前置要求

1. **Windows 7 或更高版本**（推荐 Windows 10/11）
2. **管理员权限**
3. **Visual Studio 2017 或更高版本**（或 MinGW-w64）

## 获取 Wintun

你有两种方式获取 Wintun：

### 方式 1: 使用项目中的 Wintun 源码

项目已包含 Wintun 源码在 `third_party/wintun`，但需要编译 DLL：

```bash
cd third_party/wintun
# 按照 Wintun 的编译说明编译
```

### 方式 2: 下载预编译的 DLL（推荐）

1. 访问 https://www.wintun.net/
2. 下载最新版本的 Wintun（例如 `wintun-0.14.1.zip`）
3. 解压后找到对应架构的 DLL：
   - `wintun/bin/amd64/wintun.dll` (64-bit)
   - `wintun/bin/x86/wintun.dll` (32-bit)
   - `wintun/bin/arm64/wintun.dll` (ARM64)
4. 将 `wintun.dll` 复制到以下位置之一：
   - 可执行文件所在目录
   - `third_party/wintun/bin/amd64/` (推荐)
   - Windows 系统目录（不推荐）

## 编译项目

```cmd
# 创建构建目录
mkdir build
cd build

# 配置 CMake (使用 Visual Studio)
cmake .. -G "Visual Studio 16 2019" -A x64

# 或使用 MinGW
cmake .. -G "MinGW Makefiles"

# 编译
cmake --build . --config Release

# 或在 Visual Studio 中打开 ConnectTool.sln
```

## 运行

### 方法 1: 以管理员身份运行

1. 找到编译生成的 `TunExample.exe` 或 `OnlineGameTool.exe`
2. 右键点击 -> **以管理员身份运行**

### 方法 2: 使用命令行（管理员）

```cmd
# 以管理员身份打开 PowerShell 或 CMD
cd build\Release

# 运行 TUN 示例
.\TunExample.exe

# 运行主程序
.\OnlineGameTool.exe
```

## 常见问题

### Q: 运行时提示 "Failed to load wintun.dll"
**A:** 确保 `wintun.dll` 在以下位置之一：
- 可执行文件同目录
- `third_party/wintun/bin/amd64/` 目录
- 添加到系统 PATH

### Q: 运行时提示 "Access Denied" 或权限错误
**A:** 必须以管理员身份运行程序。

### Q: 编译时提示找不到 Wintun 头文件
**A:** CMake 会自动包含 `third_party/wintun/api`，确保该目录存在。

### Q: 创建适配器失败
**A:** 检查：
1. 是否以管理员身份运行
2. Windows 防火墙/杀毒软件是否阻止
3. 是否有其他程序占用同名适配器

### Q: 如何查看创建的虚拟网卡？
**A:** 
```cmd
# 方法 1: 控制面板
控制面板 -> 网络和共享中心 -> 更改适配器设置

# 方法 2: 命令行
ipconfig /all

# 方法 3: PowerShell
Get-NetAdapter
```

### Q: 如何删除虚拟网卡？
**A:** 虚拟网卡会在程序退出时自动删除。如果残留：
```cmd
# 设备管理器 -> 网络适配器 -> 找到 Wintun 适配器 -> 右键卸载
```

## 测试 TUN 接口

运行 `TunExample.exe` 后：

```cmd
# 1. 查看接口配置
ipconfig

# 应该能看到类似输出：
# Ethernet adapter WintunTunnel:
#    IPv4 Address. . . . . . . . . . . : 10.8.0.1
#    Subnet Mask . . . . . . . . . . . : 255.255.255.0

# 2. Ping 测试
ping 10.8.0.1

# 3. 查看网络统计
netsh interface ipv4 show interfaces
```

## 卸载

Wintun 是无驱动签名的轻量级驱动，不需要特殊卸载：
1. 关闭所有使用 Wintun 的程序
2. 虚拟网卡会自动删除
3. 删除 `wintun.dll` 文件即可

## 许可证

Wintun 采用双重许可：
- GPLv2
- MIT (for inclusion in projects)

本项目的使用符合 MIT 许可。

## 参考资料

- Wintun 官网: https://www.wintun.net/
- Wintun GitHub: https://git.zx2c4.com/wintun/
- WireGuard (使用 Wintun 的知名项目): https://www.wireguard.com/
