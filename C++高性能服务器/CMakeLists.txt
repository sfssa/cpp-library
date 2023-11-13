## 项目构建最低所需的cmake版本
#cmake_minimum_required(VERSION 3.0)
#
#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "$ENV(CXXFLAGS) -)
#
## 项目的名称
#project(atpdxy)
#
## 设置变量LIB_SRC，包含源文件log.cpp
#set(LIB_SRC
#    atpdxy/log/log.cpp
#)
#
## 用LIB_SRC中的源文件构建一个共享库，名为atpdxy
#add_library(atpdxy SHARED ${LIB_SRC})
#
## 设置目标的属性，将atpdxy目标C++表中设置为11，并要求编译器强制使用此标准
#set_target_properties(atpdxy PROPERTIES
#    CXX_STANDARD 11
#    CXX_STANDARD_REQUIRED ON
#)
## 设置目标的编译选项
## -rdynamic:生成一个包含所有符号信息的动态链接库
## -O3:启用最高级别的优化，提高程序的运行速度，但会增大编译时间
## -g:生成调试信息，用于调试
## -Wall:启用所有常见的警告
## -Wno-deprecated:禁止对过时的特性给出警告
## -Werror:将所有警告视为错误，也就是警告也会导致编译失败
## -Wno-unused-function:禁止对未使用的函数给出警告
#target_compile_options(atpdxy PRIVATE
#    -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function
#)
#
## 使用test_log.cpp构建一个可执行文件test
#add_executable(test test/test_log.cpp)
#
## 添加构建目标之间的依赖关系，test可执行文件依赖于atpdxy库
#add_dependencies(test atpdxy)
#
## 指定链接的库，test可执行文件链接到atpdxy库
#target_link_libraries(test atpdxy)
#
## 设置可执行文件和库的输出路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#

cmake_minimum_required(VERSION 2.8)
project(atpdxy)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(LIB_SRC
    atpdxy/log/log.cpp
    )
add_library(atpdxy SHARED ${LIB_SRC})
add_executable(test test/test_log.cpp)
add_dependencies(test atpdxy)
target_link_libraries(test atpdxy)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)