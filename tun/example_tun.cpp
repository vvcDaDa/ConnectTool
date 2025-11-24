/**
 * @file example_tun.cpp
 * @brief TUN 设备使用示例
 * 
 * 这个程序演示如何使用跨平台的 TUN 接口：
 * 1. 创建 TUN 设备
 * 2. 配置 IP 地址
 * 3. 读写数据包
 */

#include "tun_interface.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

void print_packet_info(const uint8_t* buffer, int length) {
    if (length < 20) {
        std::cout << "Packet too short: " << length << " bytes" << std::endl;
        return;
    }
    
    // 解析 IP 头部（简化版）
    uint8_t version = (buffer[0] >> 4) & 0x0F;
    uint8_t protocol = buffer[9];
    
    char src_ip[16], dst_ip[16];
    snprintf(src_ip, sizeof(src_ip), "%d.%d.%d.%d", 
             buffer[12], buffer[13], buffer[14], buffer[15]);
    snprintf(dst_ip, sizeof(dst_ip), "%d.%d.%d.%d", 
             buffer[16], buffer[17], buffer[18], buffer[19]);
    
    const char* proto_name = "Unknown";
    if (protocol == 1) proto_name = "ICMP";
    else if (protocol == 6) proto_name = "TCP";
    else if (protocol == 17) proto_name = "UDP";
    
    std::cout << "  IPv" << (int)version << " packet: " 
              << src_ip << " -> " << dst_ip 
              << " [" << proto_name << "] "
              << length << " bytes" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== TUN Device Example ===" << std::endl;
    std::cout << "Platform: ";
    #ifdef __linux__
        std::cout << "Linux" << std::endl;
    #elif defined(__APPLE__)
        std::cout << "macOS" << std::endl;
    #elif defined(_WIN32)
        std::cout << "Windows" << std::endl;
    #else
        std::cout << "Unknown" << std::endl;
    #endif
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建 TUN 设备
    auto tun = tun::create_tun();
    if (!tun) {
        std::cerr << "Failed to create TUN device" << std::endl;
        return 1;
    }
    
    // 打开 TUN 设备
    std::cout << "\n[1] Opening TUN device..." << std::endl;
    if (!tun->open("mytun0", 1500)) {
        std::cerr << "Failed to open TUN device: " << tun->get_last_error() << std::endl;
        std::cerr << "\nNote: You may need administrator/root privileges:" << std::endl;
        #ifdef __linux__
            std::cerr << "  sudo ./example_tun" << std::endl;
            std::cerr << "  or: sudo setcap cap_net_admin+ep ./example_tun" << std::endl;
        #elif defined(__APPLE__)
            std::cerr << "  sudo ./example_tun" << std::endl;
        #elif defined(_WIN32)
            std::cerr << "  Run as Administrator" << std::endl;
        #endif
        return 1;
    }
    std::cout << "✓ TUN device opened: " << tun->get_device_name() << std::endl;
    
    // 配置 IP 地址
    std::cout << "\n[2] Configuring IP address..." << std::endl;
    std::string ip = "10.8.0.1";
    std::string netmask = "255.255.255.0";
    if (!tun->set_ip(ip, netmask)) {
        std::cerr << "Failed to set IP: " << tun->get_last_error() << std::endl;
        return 1;
    }
    std::cout << "✓ IP configured: " << ip << "/" << netmask << std::endl;
    
    // 启用设备
    std::cout << "\n[3] Bringing interface up..." << std::endl;
    if (!tun->set_up()) {
        std::cerr << "Failed to bring up device: " << tun->get_last_error() << std::endl;
        return 1;
    }
    std::cout << "✓ Interface is UP" << std::endl;
    
    // 设置为非阻塞模式
    std::cout << "\n[4] Setting non-blocking mode..." << std::endl;
    if (!tun->set_non_blocking(true)) {
        std::cerr << "Failed to set non-blocking: " << tun->get_last_error() << std::endl;
        return 1;
    }
    std::cout << "✓ Non-blocking mode enabled" << std::endl;
    
    // 打印使用说明
    std::cout << "\n=== TUN Device Ready ===" << std::endl;
    std::cout << "Device: " << tun->get_device_name() << std::endl;
    std::cout << "IP:     " << ip << std::endl;
    std::cout << "MTU:    " << tun->get_mtu() << std::endl;
    std::cout << "\nYou can now test the interface:" << std::endl;
    std::cout << "  ping " << ip << std::endl;
    std::cout << "  ping 10.8.0.2  (if you configure a peer)" << std::endl;
    std::cout << "\nPress Ctrl+C to stop...\n" << std::endl;
    
    // 主循环：读取并回显数据包
    uint8_t buffer[2048];
    int packet_count = 0;
    
    while (running) {
        // 读取数据包
        int n = tun->read(buffer, sizeof(buffer));
        
        if (n > 0) {
            packet_count++;
            std::cout << "[Packet #" << packet_count << "] Received " << n << " bytes:" << std::endl;
            print_packet_info(buffer, n);
            
            // 回显数据包（交换源和目的地址）
            if (n >= 20) {
                // 交换 IP 地址（简化版，仅适用于 IPv4）
                uint8_t temp[4];
                memcpy(temp, buffer + 12, 4);           // 保存源 IP
                memcpy(buffer + 12, buffer + 16, 4);    // 目的 -> 源
                memcpy(buffer + 16, temp, 4);           // 源 -> 目的
                
                // 重新计算校验和（这里简化处理，实际应该重算）
                buffer[10] = 0;
                buffer[11] = 0;
                
                // 发送回显
                int sent = tun->write(buffer, n);
                if (sent > 0) {
                    std::cout << "  ↳ Echoed back " << sent << " bytes" << std::endl;
                } else if (sent < 0) {
                    std::cerr << "  ↳ Write error: " << tun->get_last_error() << std::endl;
                }
            }
        } else if (n < 0) {
            std::cerr << "Read error: " << tun->get_last_error() << std::endl;
            break;
        }
        
        // 避免 CPU 占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\n=== Shutting Down ===" << std::endl;
    std::cout << "Total packets received: " << packet_count << std::endl;
    std::cout << "Closing TUN device..." << std::endl;
    
    tun->close();
    std::cout << "✓ Done" << std::endl;
    
    return 0;
}
