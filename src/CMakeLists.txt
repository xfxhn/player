

add_subdirectory(openglWidget)
add_subdirectory(demux)
add_subdirectory(decode)
add_subdirectory(resample)
add_subdirectory(audioPlay)
add_subdirectory(audioThread)
add_subdirectory(videoThread)
add_subdirectory(demuxThread)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_custom_target(gentest COMMAND ${CMAKE_COMMAND} -E echo "\"$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>\"")
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(player
            MANUAL_FINALIZATION
            main.cpp
            )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET cmakeqt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else ()
    add_executable(player main.cpp ${PROJECT_SOURCE_DIR}/rcc/shader.qrc)
endif ()


target_include_directories(player
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/openglWidget
        )

target_link_directories(player
        PRIVATE ${PROJECT_SOURCE_DIR}/lib
        )
target_link_libraries(player
        PRIVATE
        openglWidget
        demux
        decode
        resample
        audioPlay
        audioThread
        videoThread
        demuxThread
        avformat.lib
        avutil.lib
        avcodec.lib
        swresample.lib
        avfilter.lib
        Qt${QT_VERSION_MAJOR}::Widgets
        $<$<STREQUAL:${QT_VERSION_MAJOR},6>:OpenGLWidgets>
        )


set_target_properties(player PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        #[[当在 iOS 上构建时将此属性设置为TRUE可执行文件时，将创建为应用程序包]]
        MACOSX_BUNDLE TRUE

        #[[在 Windows 上链接的可执行文件将使用 WinMain() 入口点创建，而不仅仅是 main()]]
        WIN32_EXECUTABLE TRUE
        )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #LINK_FLAGS_DEBUG /SUBSYSTEM:CONSOLE
    target_link_options(player PRIVATE $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
endif ()

#if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
#    set_property(TARGET player PROPERTY WIN32_EXECUTABLE true)
#endif ()
if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(player)
endif ()
