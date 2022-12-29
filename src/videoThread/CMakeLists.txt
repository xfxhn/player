

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(videoThread OBJECT videoThread.cpp)
target_include_directories(videoThread
        PRIVATE ${PROJECT_SOURCE_DIR}/src/decode
        PRIVATE ${PROJECT_SOURCE_DIR}/src/openglWidget
        PRIVATE ${PROJECT_SOURCE_DIR}/src/audioThread
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        )
target_link_libraries(videoThread PRIVATE Qt${QT_VERSION_MAJOR}::Core)