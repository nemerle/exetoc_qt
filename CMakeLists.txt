PROJECT(ExeToC_Qt)
#SET(CMAKE_BUILD_TYPE Debug)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

cmake_policy(SET CMP0020 NEW)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})
IF(CMAKE_CXX_COMPILER MATCHES cl.exe)
    ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS )
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
ENDIF()

##	Boost related
SET(Boost_USE_STATIC_LIBS   ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost COMPONENTS filesystem system)
#########################################################

find_package(Qt5Widgets REQUIRED)
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
