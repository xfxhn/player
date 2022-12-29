

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)


add_library(demuxThread OBJECT demuxThread.cpp)

target_include_directories(demuxThread
        PRIVATE ${PROJECT_SOURCE_DIR}/src/audioThread
        PRIVATE ${PROJECT_SOURCE_DIR}/src/videoThread
        PRIVATE ${PROJECT_SOURCE_DIR}/src/demux
        PRIVATE ${PROJECT_SOURCE_DIR}/src/openglWidget
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(demuxThread PRIVATE Qt${QT_VERSION_MAJOR}::Gui)



