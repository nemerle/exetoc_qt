SET(gui_UI_FILES
    exe2c_gui.ui
    functionviewwidget.ui
    functionlistdockwidget.ui
)
SET(gui_MOC_HEADERS
    exe2c_mainwindow.h
    functionviewwidget.h
    functionlistdockwidget.h
)
SET(gui_SOURCES
    exe2c_gui.cpp
    exe2c_mainwindow.cpp
    functionviewwidget.cpp
    functionlistdockwidget.cpp
    ${gui_MOC_HEADERS}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(gui_UI_H ${gui_UI_FILES})

QT4_WRAP_CPP(gui_MOC_SRC ${gui_MOC_HEADERS})

qt4_automoc(${gui_SOURCES})
ADD_EXECUTABLE(exetoc ${gui_SOURCES} ${gui_MOC_SRC} ${gui_UI_H})
TARGET_LINK_LIBRARIES(exetoc exe2c_lib ${QT_LIBRARIES} ${Boost_LIBRARIES})
ADD_DEPENDENCIES(exetoc exe2c_lib)
