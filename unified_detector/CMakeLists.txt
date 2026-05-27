cmake_minimum_required(VERSION 2.8)
set(LIB_NAME "ImageSegmenter")
project(${LIB_NAME})
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(BUILD_x86_TESTER TRUE)

add_definitions(-DBUILD_x86)
add_definitions(-DTESTING)
add_definitions( -DDLINTERFACELIBLOCATION_SYSTEM="libDLInterface.so")
add_definitions( -DLIBBUILDMODE="default")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/third_party/DLInterface/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/json)
# include_directories(${CMAKE_SOURCE_DIR}/third_party/snap_libs/include)

include_directories(${CMAKE_SOURCE_DIR}/include/image_segmentation)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(DLInterface SHARED IMPORTED)
set_property(TARGET DLInterface PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/third_party/DLInterface/libs/x86_64/libDLInterface.so")

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*/*.cpp" "${CMAKE_SOURCE_DIR}/src/*/*/*.cpp")

IF(BUILD_x86_TESTER)
	message("Set to build test exectuable")
    add_executable(${LIB_NAME} ${SOURCES} test/test_batch.cpp)
ELSE()
	message("Set to build .so library")
	add_library(${LIB_NAME} SHARED ${SOURCES})	
ENDIF(BUILD_x86_TESTER)

target_link_libraries(${LIB_NAME} ${OpenCV_LIBS} ${CMAKE_DL_LIBS})

string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(Green   "${Esc}[32m")

IF(BUILD_x86_TESTER)
    install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin/x86_64)
    message("${Green}Build Completed | Output directory: ./bin/x86_64${ColorReset}")
ELSE()
    install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/libs/x86_64)
    message("${Green}Build Completed | Output directory: ./libs/x86_64${ColorReset}")
ENDIF(BUILD_x86_TESTER)
