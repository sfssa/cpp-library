cmake_minimum_required(VERSION 3.0)
project(atpdxy)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# find_library(YAMLCPP libyaml-cpp.a)
# message("***", ${YAMLCPP})

set(LIB_SRC
    atpdxy/config/config.cpp
    atpdxy/fiber/fiber.cpp
    atpdxy/iomanager/iomanager.cpp
    atpdxy/log/log.cpp
    atpdxy/mutex/mutex.cpp
    atpdxy/scheduler/scheduler.cpp
    atpdxy/thread/thread.cpp
    atpdxy/timer/timer.cpp
    atpdxy/utils/utils.cpp
)

add_library(atpdxy SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(atpdxy) #__FILE__

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)

set (LIBS
    atpdxy
    pthread
    yaml-cpp)

message("***" ${LIBS})

add_executable(test_log test/test_log.cpp)
add_dependencies(test_log atpdxy)
force_redefine_file_macro_for_sources(test_log)
target_link_libraries(test_log ${LIBS})

add_executable(test_config test/test_config.cpp)
add_dependencies(test_config atpdxy)
force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread test/test_thread.cpp)
add_dependencies(test_thread atpdxy)
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIBS})

add_executable(test_utils test/test_utils.cpp)
add_dependencies(test_utils atpdxy)
force_redefine_file_macro_for_sources(test_utils)
target_link_libraries(test_utils ${LIBS})

add_executable(test_fiber test/test_fiber.cpp)
add_dependencies(test_fiber atpdxy)
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler test/test_scheduler.cpp)
add_dependencies(test_scheduler atpdxy)
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager test/test_iomanager.cpp)
add_dependencies(test_iomanager atpdxy)
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

