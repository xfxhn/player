


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)
#get_target_property(INCLUDE_DIR Qt${QT_VERSION_MAJOR}::Widgets INTERFACE_INCLUDE_DIRECTORIES)
#get_target_property(INCLUDE_DIR1 Qt${QT_VERSION_MAJOR}::Gui INTERFACE_INCLUDE_DIRECTORIES)

add_library(openglWidget OBJECT openglWidget.cpp videoWidget.cpp)

#[[
这里qt会Built target openglWidget_autogen，ui_xxx，如果引用了某个外部头文件，要告诉cmake这个头文件的位置
这里按理说应该不用告诉他位置，因为openglWidget.cpp把ui_xxx.h的内容引入到这个文件了(预编译展开)，然后编译openglWidget.cpp
这个目标的时候这个文件的当前目录存在videoWidget.h这个头文件，因为#include"xx" 首先在当前目录查找该文件
不用指定目录，我试了不同编译器，有的编译器需要指定，有的不需要
]]
target_include_directories(openglWidget
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src/demuxThread
        )
target_link_libraries(openglWidget
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        )

#qt_add_resources(
#        openglWidget "shader"       #生成的qrc文件的文件名
#        PREFIX "/shader"   #设置的是生成的qrc的前缀，方便程序按照这个前缀引用
#        #[[BASE 是一个路径前缀，表示文件别名的根点。例如，如果 BASE 是 "assets" 并且 FILES 是 "assets/images/logo.png" ，
#        那么该文件的别名是 "images/logo.png" ]]
#        BASE "../../shader"
#        FILES
#        ${PROJECT_SOURCE_DIR}/shader/video.vert
#        ${PROJECT_SOURCE_DIR}/shader/video.frag
#)
#qt_add_resources(
#        openglWidget "image"       #生成的qrc文件的文件名
#        PREFIX "/image"   #设置的是生成的qrc的前缀，方便程序按照这个前缀引用
#        #[[BASE 是一个路径前缀，表示文件别名的根点。例如，如果 BASE 是 "assets" 并且 FILES 是 "assets/images/logo.png" ，
#        那么该文件的别名是 "images/logo.png" ]]
#        BASE "../../resource"
#        FILES
#        ${PROJECT_SOURCE_DIR}/resource/icon1.png
#)