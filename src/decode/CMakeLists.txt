

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)
add_library(decode OBJECT decode.cpp)


target_include_directories(decode
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )