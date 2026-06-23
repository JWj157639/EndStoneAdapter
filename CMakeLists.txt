cmake_minimum_required(VERSION 3.15)

project(huhobot CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 接收用户传入参数，设置默认值
set(HUHOBOT_SERVER_URL "ws://127.0.0.1:8888" CACHE STRING "WebSocket server URL")

# 生成配置文件
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

include(FetchContent)
FetchContent_Declare(
        endstone
        GIT_REPOSITORY https://github.com/EndstoneMC/endstone.git
        GIT_TAG "v0.11.0"
)
FetchContent_MakeAvailable(endstone)

file(GLOB_RECURSE SOURCES
        "src/*.cpp"
        "src/**/*.cpp"
        "include/quiz/WsConnectionManager.cpp"
)

# 排除废弃的旧文件
list(FILTER SOURCES EXCLUDE REGEX ".*HuhobotClient\\.cpp$")

file(GLOB_RECURSE WEBSOCKETFILES_SOURCES
        "include/quiz/websocketfiles/src/*.cpp"
)

list(FILTER WEBSOCKETFILES_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# 加入重构后的新客户端实现
list(APPEND SOURCES "src/HuhobotClientNew.cpp")
list(APPEND SOURCES ${WEBSOCKETFILES_SOURCES})
endstone_add_plugin(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/quiz/websocketfiles/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/H>
        $<INSTALL_INTERFACE:include>
)

# 测试配置
enable_testing()

# 测试可执行文件（独立编译，不依赖 endstone 插件系统）
add_executable(tests
        tests/test_main.cpp
        src/TextUtils.cpp
        src/ConfigManager.cpp
        src/tools.cpp
)

target_include_directories(tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/H
        ${CMAKE_CURRENT_SOURCE_DIR}/include/quiz/websocketfiles/include
)

# 测试需要 nlohmann/json
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

target_link_libraries(tests PRIVATE nlohmann_json::nlohmann_json)

# 注册测试
add_test(NAME EndStoneAdapterTests COMMAND tests)