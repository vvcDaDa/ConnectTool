/**
 * @file tun.h
 * @brief TUN 跨平台抽象层 - 主头文件
 * 
 * 这是 TUN 模块的统一入口，包含此文件即可使用所有功能。
 * 
 * @example 基本用法
 * @code
 * #include "tun/tun.h"
 * 
 * int main() {
 *     auto tun = tun::create_tun();
 *     
 *     if (!tun->open("mytun")) {
 *         std::cerr << "Error: " << tun->get_last_error() << std::endl;
 *         return 1;
 *     }
 *     
 *     tun->set_ip("10.0.0.1", "255.255.255.0");
 *     tun->set_up();
 *     
 *     uint8_t buffer[2048];
 *     while (true) {
 *         int n = tun->read(buffer, sizeof(buffer));
 *         if (n > 0) {
 *             // 处理数据包
 *             process_packet(buffer, n);
 *         }
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 */

#pragma once

// 包含核心接口
#include "tun_interface.h"

/**
 * @namespace tun
 * @brief TUN 设备操作命名空间
 * 
 * 提供跨平台的 TUN 虚拟网络接口操作。
 * 支持 Linux (/dev/net/tun), macOS (utun), Windows (Wintun)
 */

// 版本信息
#define TUN_VERSION_MAJOR 1
#define TUN_VERSION_MINOR 0
#define TUN_VERSION_PATCH 0

#define TUN_VERSION_STRING "1.0.0"

namespace tun {

/**
 * @brief 获取 TUN 模块版本
 * @return 版本字符串（如 "1.0.0"）
 */
inline const char* get_version() {
    return TUN_VERSION_STRING;
}

/**
 * @brief 获取当前平台名称
 * @return 平台名称字符串
 */
inline const char* get_platform() {
#ifdef __linux__
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(_WIN32)
    return "Windows";
#else
    return "Unknown";
#endif
}

/**
 * @brief 检查当前平台是否支持 TUN
 * @return true 如果支持，false 否则
 */
inline bool is_platform_supported() {
#if defined(__linux__) || defined(__APPLE__) || defined(_WIN32)
    return true;
#else
    return false;
#endif
}

} // namespace tun

/**
 * @mainpage TUN 跨平台抽象层
 * 
 * @section intro_sec 简介
 * 
 * 这是一个完整的跨平台 TUN 虚拟网络接口抽象层，支持：
 * - Linux: 使用 /dev/net/tun
 * - macOS: 使用 utun 内核控制接口
 * - Windows: 使用 Wintun 高性能驱动
 * 
 * @section install_sec 安装
 * 
 * 只需将 tun 目录添加到你的项目，并在 CMakeLists.txt 中包含：
 * @code
 * add_subdirectory(tun)
 * target_link_libraries(your_app PRIVATE tun)
 * @endcode
 * 
 * @section usage_sec 使用
 * 
 * @code
 * #include "tun/tun.h"
 * 
 * auto tun = tun::create_tun();
 * tun->open("mytun");
 * tun->set_ip("10.0.0.1", "255.255.255.0");
 * tun->set_up();
 * 
 * uint8_t buffer[2048];
 * int n = tun->read(buffer, sizeof(buffer));
 * @endcode
 * 
 * @section license_sec 许可证
 * 
 * MIT License
 * 
 * @see tun::TunInterface
 * @see tun::create_tun()
 */
