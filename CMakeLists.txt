PROJECT(ExeToC_Qt)
#SET(CMAKE_BUILD_TYPE Debug)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)


SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
IF(CMAKE_CXX_COMPILER MATCHES cl)
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
	SET(wxWidgets_CONFIGURATION msw)
	ADD_DEFINITIONS(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS )
ENDIF ()
##	Boost related
SET(Boost_USE_STATIC_LIBS   ON)
SET(Boost_USE_MULTITHREADED ON)
SET(BOOST_ROOT "" CACHE PATH "")
SET (Boost_COMPILER "-vc100")
FIND_PACKAGE(Boost COMPONENTS filesystem system REQUIRED)
#########################################################

##  Qt related
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui REQUIRED)

INCLUDE(${QT_USE_FILE})
#########################################################
ADD_DEFINITIONS(-D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS)

find_package(LLVM)

if( NOT LLVM_FOUND )
  message(FATAL_ERROR "LLVM package can't be found. Set CMAKE_PREFIX_PATH variable to LLVM's installation prefix.")
endif()

include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )

#########################################################
INCLUDE_DIRECTORIES(
	.
	exe2c
	exe2c/include
	${Boost_INCLUDE_DIR}
	${LLVM_BLD_ROOT}/include
	${LLVM_ROOT}/include
)

LINK_DIRECTORIES(
${Boost_LIBRARY_DIRS}
${LLVM_BLD_ROOT}/lib
)
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES jit native)

ADD_SUBDIRECTORY(exe2c)
ADD_SUBDIRECTORY(exe2c_gui)
