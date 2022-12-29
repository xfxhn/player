
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
add_library(audioThread OBJECT audioThread.cpp)


target_include_directories(audioThread
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src/decode
        PRIVATE ${PROJECT_SOURCE_DIR}/src/audioPlay
        PRIVATE ${PROJECT_SOURCE_DIR}/src/resample
        )
target_link_libraries(audioThread PRIVATE Qt${QT_VERSION_MAJOR}::Core)
