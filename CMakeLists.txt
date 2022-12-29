cmake_minimum_required(VERSION 3.15)
project(player VERSION 0.1)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #[[msvc编译器要添加这两个编译选项]]
    #  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    add_compile_options("/Zc:__cplusplus" "/permissive-" "/utf-8")
    #    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_PREFIX_PATH D:/softwares/qt/5.15.2/msvc2019_64)
else ()
    set(CMAKE_PREFIX_PATH D:/softwares/qt/5.15.2/mingw81_64)
endif ()


#[[设置查找UI文件的路径 AUTOUIC处理]]
set(CMAKE_AUTOUIC_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/fromes")


# 有些项目会动态生成头文件，项目中需要引入它，因此需要将output目录也include进来
# 等同于INCLUDE_DIRECTORY(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

#[[在编译器编译之前，要先经过这三个处理器处理源文件，之后才放进cpp编译器里]]
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#[[
这里如果是msvc编译器只会设置/std:c++17,但不会设置特定于 msvc 的/Zc:__cplusplus标志,
因为 MSVC__cplusplus在所有未指定的情况下都明确设置为 199711L /Zc:cplusplus
https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
]]
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(src)

