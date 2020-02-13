add_definitions(-DMESH_DATA_DIR=\"$ENV{HOME}\")

find_package(Qt5 COMPONENTS Gui Widgets OpenGL REQUIRED)

include_directories(
  include
)

set(MAINWINDOW "wtt_demo")

set(${MAINWINDOW}_SRC
    src/main.cpp
    src/mainwindow.cpp
    src/opengl_widget.cpp
    src/action_panel.cpp
    src/arcball_camera.cpp
    src/scene_object.cpp
    src/triangle_mesh_scene.cpp
    src/modal_widget.cpp
    src/message_box.cpp
    src/wtt_manager.cpp
    src/integer_setter.cpp
    src/input_prop.cpp
    src/threaded_gl_buffer_uploader.cpp
    src/control_panel.cpp
    src/glview_control_panel.cpp
)


qt5_wrap_cpp(${MAINWINDOW}_MOCS 
              include/mainwindow.hpp
              include/opengl_widget.hpp
              include/action_panel.hpp
              include/arcball_camera.hpp
              include/scene_object.hpp
              include/triangle_mesh_scene.hpp
              include/modal_widget.hpp
              include/message_box.hpp
              include/wtt_manager.hpp
              include/integer_setter.hpp
              include/input_prop.hpp
              include/threaded_gl_buffer_uploader.hpp
              include/control_panel.hpp
              include/glview_control_panel.hpp
            )

qt5_wrap_ui(${MAINWINDOW}_UIS
            uis/mainwindow.ui
            uis/action_panel.ui
            uis/messagebox.ui
            uis/integer_setter.ui
            uis/input_prop.ui
            uis/control_panel.ui
            uis/mesh_info_label.ui)

qt5_add_resources(${MAINWINDOW}_RESOURCES
                  resources/resources.qrc)

add_executable(${MAINWINDOW}
                ${${MAINWINDOW}_SRC}
                ${${MAINWINDOW}_UIS}
                ${${MAINWINDOW}_RESOURCES}
                ${${MAINWINDOW}_MOCS}
              )

target_include_directories(${MAINWINDOW}
              PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
              PUBLIC ${CGAL_INCLUDE_DIRS}
              PUBLIC include
              )

target_link_libraries(${MAINWINDOW}
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::OpenGL
                      ${CGAL_LIBRARY})

install(TARGETS ${MAINWINDOW}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})