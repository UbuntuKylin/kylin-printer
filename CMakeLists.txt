cmake_minimum_required(VERSION 3.5)
#Set Project Name
project(kylin-printer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets X11Extras Network DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets X11Extras Network DBus REQUIRED)
find_package(KF5WindowSystem REQUIRED)

#add include 
include_directories(${PROJECT_SOURCE_DIR}/printer_manager/)
include_directories(${PROJECT_SOURCE_DIR}/ui/)

file(GLOB printer_manager_SRC 
    "printer_manager/*.cpp"
    "printer_manager/*.h"
    )
  

file(GLOB ui_SRC 
    "ui/*.cpp"
    "ui/*.h"
    )

SET(ui_RESOURCE ${PROJECT_SOURCE_DIR}/ui/picture.qrc)

add_executable(kylin-printer
    main.cpp
    ${printer_manager_SRC}
    ${ui_SRC}
    ${ui_RESOURCE}
)

target_link_libraries(kylin-printer Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

#dependencies
target_link_libraries(kylin-printer "-lpthread -lz")
#cups
target_link_libraries(kylin-printer "-lcups")
#deb install
target_link_libraries(kylin-printer "-lQApt")
target_link_libraries(kylin-printer KF5::WindowSystem)

target_link_libraries(kylin-printer -lX11)

########### install ###############
install(TARGETS kylin-printer DESTINATION /usr/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/kylin-printer.desktop DESTINATION /etc/xdg/autostart/)
