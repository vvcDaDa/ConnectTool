cmake_minimum_required(VERSION 3.10)
project(ConnectTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Add ImGui
add_subdirectory(imgui)

# Add nanoid_cpp
add_subdirectory(nanoid_cpp)

# TUN module sources (platform-specific)
set(TUN_SOURCES
    tun/tun_factory.cpp
)

if(UNIX AND NOT APPLE)
    # Linux
    list(APPEND TUN_SOURCES tun/tun_linux.cpp)
    message(STATUS "Building TUN module for Linux")
elseif(APPLE)
    # macOS
    list(APPEND TUN_SOURCES tun/tun_macos.cpp)
    message(STATUS "Building TUN module for macOS")
elseif(WIN32)
    # Windows
    list(APPEND TUN_SOURCES tun/tun_windows.cpp)
    message(STATUS "Building TUN module for Windows (Wintun)")
endif()

# Add Online Game Tool executable
add_executable(OnlineGameTool 
    ConnectTool/online_game_tool.cpp 
    ConnectTool/steam/steam_message_handler.cpp 
    ConnectTool/steam/steam_networking_manager.cpp 
    ConnectTool/steam/steam_room_manager.cpp 
    ConnectTool/steam/steam_utils.cpp 
    ConnectTool/steam/steam_vpn_bridge.cpp
    ConnectTool/net/tcp_server.cpp 
    ConnectTool/net/multiplex_manager.cpp
    ${TUN_SOURCES}
)

# Set compile options for UTF-8 support
if(MSVC)
    target_compile_options(OnlineGameTool PRIVATE /utf-8)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/ConnectTool)
include_directories(${CMAKE_SOURCE_DIR}/ConnectTool/net)
include_directories(${CMAKE_SOURCE_DIR}/ConnectTool/steam)
include_directories(${CMAKE_SOURCE_DIR}/nanoid_cpp/inc)
include_directories(${CMAKE_SOURCE_DIR}/tun)

# Include Steamworks
include_directories(${CMAKE_SOURCE_DIR}/steamworks/public/steam)

# Include Wintun (Windows only)
if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/third_party/wintun/api)
endif()

# Link libraries
target_link_libraries(OnlineGameTool PRIVATE imgui nanoid ${CMAKE_SOURCE_DIR}/steamworks/redistributable_bin/win64/steam_api64.lib Boost::system ws2_32)

# Add TUN example executable (optional)
option(BUILD_TUN_EXAMPLE "Build TUN example program" ON)
if(BUILD_TUN_EXAMPLE)
    add_executable(TunExample tun/example_tun.cpp ${TUN_SOURCES})
    
    # Platform-specific libraries for TUN example
    if(WIN32)
        target_link_libraries(TunExample PRIVATE iphlpapi ws2_32 ole32)
    elseif(UNIX)
        target_link_libraries(TunExample PRIVATE pthread)
    endif()
    
    message(STATUS "TUN example will be built")
endif()