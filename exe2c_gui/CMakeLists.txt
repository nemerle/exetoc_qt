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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

QT5_WRAP_UI(gui_UI_H ${gui_UI_FILES})

QT5_WRAP_CPP(gui_MOC_SRC ${gui_MOC_HEADERS})

ADD_EXECUTABLE(exetoc ${gui_SOURCES} ${gui_MOC_SRC} ${gui_UI_H})
TARGET_LINK_LIBRARIES(exetoc exe2c_lib ${Boost_LIBRARIES} ${REQ_LLVM_LIBRARIES})

qt5_use_modules(exetoc Widgets)

SET_TARGET_PROPERTIES(exetoc
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/BIN"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/BIN"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/BIN"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/BIN" )
