set(SRCS
    main.cpp
    Window.cpp
    Window.h

    Shaders/diffuse.fs
    Shaders/diffuse.vs
    Textures/voronoi.png
    thirdparty/glm
        thirdparty/tinygltf
    resources.qrc
        camera.cpp camera.h mainwindow.cpp mainwindow.h)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(demo-app ${SRCS})

target_link_libraries(demo-app
    PRIVATE
        Qt5::Widgets
        FGL::Base
        thirdparty::tinygltf
)