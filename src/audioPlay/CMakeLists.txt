

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)


#get_target_property(INCLUDE_DIR Qt${QT_VERSION_MAJOR}::Widgets INTERFACE_INCLUDE_DIRECTORIES)
#get_target_property(INCLUDE_DIR1 Qt${QT_VERSION_MAJOR}::Multimedia INTERFACE_INCLUDE_DIRECTORIES)

add_library(audioPlay OBJECT audioPlay.cpp CAudioPlay.cpp)

target_include_directories(audioPlay
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        )

target_link_libraries(audioPlay
        PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia
        )



