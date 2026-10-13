cmake_minimum_required(VERSION 3.13)

project(chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)  # 设置target的CXX_STANDARD默认值
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(    # 设置target的COMPILE_OPTIONS默认值
    -Wall       # 对printf、未使用的变量有警告
#    -Wextra     # 对未使用的函数参数有警告
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/client)
include_directories(${PROJECT_SOURCE_DIR}/thridparty)

add_subdirectory(src)