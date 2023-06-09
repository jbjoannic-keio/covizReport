cmake_minimum_required(VERSION 3.0.0)
project(computerVision)

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3")

# Check C++14 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++14.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

find_package(OpenCV 3.0 QUIET)

if(NOT OpenCV_FOUND)
   find_package(OpenCV 2.4.3 QUIET)

   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV > 2.4.3 not found.")
   endif()
endif()

find_package(Eigen3 3.1.0 REQUIRED NO_MODULE)
find_package(Pangolin REQUIRED)

include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_SOURCE_DIR}/include
   ${EIGEN3_INCLUDE_DIR}
   ${Pangolin_INCLUDE_DIRS}
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED
   src/utils.cpp
)

target_link_libraries(${PROJECT_NAME}
   ${OpenCV_LIBS}
   ${EIGEN3_LIBS}
   ${Pangolin_LIBRARIES}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/exec)
add_executable(rep3 exec/rep3.cpp)
target_link_libraries(rep3 ${PROJECT_NAME})

add_executable(rep4 exec/rep4.cpp)
target_link_libraries(rep4 ${PROJECT_NAME})