#cmake的最低版本要求
cmake_minimum_required(VERSION 3.0)
#工程名称
project(chat)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#配置可执行文件生成的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)

#配置头文件搜索路径

include_directories(${PROJECT_SOURCE_DIR}/include/)

include_directories(${PROJECT_SOURCE_DIR}/include/server/)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db/)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model/)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis/)
#include_directories(${PROJECT_SOURCE_DIR}/include/client)

#加载子目录
add_subdirectory(src)


