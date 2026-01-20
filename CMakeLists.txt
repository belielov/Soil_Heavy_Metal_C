cmake_minimum_required(VERSION 3.10)
project(SoilHMsInversion)

set(CMAKE_CXX_STANDARD 17)

# 1. 设置头文件路径
# 让编译器能找到 "json.hpp" (当前目录) 和 <xgboost/c_api.h> (include目录)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. 设置库文件搜索路径 (当前目录)
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 3. 生成可执行文件
add_executable(HMs_Predict main.cpp)

# 4. 链接库
# 针对 Windows 环境的特殊处理
if(MSVC)
    # 如果你是用 Visual Studio 编译器，你必须有一个 .lib 文件
    # 如果只有 dll，MSVC 无法直接链接。建议去 GitHub 下载 libxgboost.lib
    target_link_libraries(HMs_Predict xgboost) 
else()
    # 如果你是用 MinGW (GCC)，可以直接尝试链接 dll
    # 这里文件名要匹配你的文件名，去掉后缀
    target_link_libraries(HMs_Predict xgboost)
endif()