# TUN 跨平台抽象层

这个模块提供了一个统一的 TUN 设备接口，支持 Linux、macOS 和 Windows。

## 设计理念

由于不同操作系统的网络接口实现差异巨大，我们采用了**抽象基类 + 平台特定实现**的设计模式：

- **Linux**: 使用 `/dev/net/tun` + `ioctl` 系统调用
- **macOS**: 使用 `utun` 内核控制接口
- **Windows**: 使用 Wintun 驱动（高性能，现代化）

## 文件结构

```
tun/
├── tun_interface.h      # 抽象基类定义
├── tun_linux.h/cpp      # Linux 实现
├── tun_macos.h/cpp      # macOS 实现
├── tun_windows.h/cpp    # Windows 实现（占位）
├── tun_factory.cpp      # 工厂函数（编译时选择平台）
└── README.md            # 本文件
```

## 使用示例

```cpp
#include "tun/tun_interface.h"

int main() {
    // 创建平台特定的 TUN 实例（自动选择）
    auto tun = tun::create_tun();
    
    // 打开 TUN 设备
    if (!tun->open("mytun", 1500)) {
        std::cerr << "Failed to open TUN: " << tun->get_last_error() << std::endl;
        return 1;
    }
    
    std::cout << "Created TUN device: " << tun->get_device_name() << std::endl;
    
    // 配置 IP 地址
    if (!tun->set_ip("10.0.0.1", "255.255.255.0")) {
        std::cerr << "Failed to set IP: " << tun->get_last_error() << std::endl;
        return 1;
    }
    
    // 启用设备
    if (!tun->set_up()) {
        std::cerr << "Failed to bring up device: " << tun->get_last_error() << std::endl;
        return 1;
    }
    
    // 设置为非阻塞模式
    tun->set_non_blocking(true);
    
    // 读取数据包
    uint8_t buffer[2048];
    while (true) {
        int n = tun->read(buffer, sizeof(buffer));
        if (n > 0) {
            std::cout << "Received packet: " << n << " bytes" << std::endl;
            // 处理数据包...
            
            // 回写数据包
            tun->write(buffer, n);
        } else if (n < 0) {
            std::cerr << "Read error: " << tun->get_last_error() << std::endl;
            break;
        }
        
        // 避免 CPU 占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return 0;
}
```

## 编译配置

在 `CMakeLists.txt` 中添加 TUN 模块：

```cmake
# TUN 模块源文件（根据平台选择）
set(TUN_SOURCES
    tun/tun_factory.cpp
)

if(UNIX AND NOT APPLE)
    # Linux
    list(APPEND TUN_SOURCES tun/tun_linux.cpp)
elseif(APPLE)
    # macOS
    list(APPEND TUN_SOURCES tun/tun_macos.cpp)
elseif(WIN32)
    # Windows
    list(APPEND TUN_SOURCES tun/tun_windows.cpp)
endif()

# 添加到你的目标
add_executable(your_app ${TUN_SOURCES} ...)
```

## 权限要求

### Linux
需要 root 权限或 `CAP_NET_ADMIN` 能力：
```bash
# 方法 1: 使用 root 运行
sudo ./your_app

# 方法 2: 授予能力（推荐）
sudo setcap cap_net_admin+ep ./your_app
```

### macOS
需要 root 权限：
```bash
sudo ./your_app
```

### Windows
需要管理员权限，并且需要编译 Wintun 驱动：
1. Wintun 源码已包含在 `third_party/wintun`
2. 编译项目时会自动查找 `wintun.dll`
3. 或者从 https://www.wintun.net/ 下载预编译的 `wintun.dll`
4. 将 `wintun.dll` 放在可执行文件旁边或 `third_party/wintun/bin/` 目录
5. 以管理员身份运行程序

## 平台差异说明

### Linux (`/dev/net/tun`)
- ✅ 完全实现
- ✅ 性能优秀
- ✅ 文档完善
- ⚠️ 需要 root 或 CAP_NET_ADMIN

### macOS (`utun`)
- ✅ 完全实现
- ⚠️ 需要 root 权限
- ⚠️ 包格式需要添加/移除 4 字节协议头
- ⚠️ 配置接口需要调用 `ifconfig` 命令

### Windows (Wintun)
- ✅ **完全实现**
- 使用 Wintun 高性能驱动
- 需要管理员权限
- 需要 `wintun.dll`（从 third_party/wintun 加载）
- ✅ 性能比老的 Tap-Windows 好得多

## 编译和运行

### 编译
```bash
# Linux/macOS
mkdir build && cd build
cmake ..
make

# 会生成两个可执行文件：
# - OnlineGameTool (主程序，包含 TUN 支持)
# - TunExample (TUN 示例程序)
```

### 运行 TUN 示例
```bash
# Linux
sudo ./TunExample
# 或授予能力
sudo setcap cap_net_admin+ep ./TunExample
./TunExample

# macOS
sudo ./TunExample

# Windows (需要管理员权限)
# 右键 -> 以管理员身份运行
TunExample.exe
```

### 测试
运行示例后，可以测试 TUN 接口：
```bash
# Ping TUN 接口
ping 10.8.0.1

# 查看接口信息
# Linux: ip addr show mytun0
# macOS: ifconfig utun3
# Windows: ipconfig
```

## 待办事项

- [x] 完善 Windows Wintun 实现
- [ ] 添加 IPv6 支持
- [ ] 添加路由表操作接口
- [ ] 添加单元测试
- [ ] 改进 macOS 配置方式（避免调用 system()）
- [ ] 添加异步 I/O 支持
- [ ] 优化错误处理和日志