cmake_minimum_required(VERSION 2.8)
project(exacam)

SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#find_package(libusb-1.0 REQUIRED)
#find_package(Boost COMPONENTS system serialization filesystem REQUIRED)

include(FindPkgConfig)
pkg_search_module(LIBUSB REQUIRED libusb-1.0)

include_directories(${LIBUSB_INCLUDE_DIRS})

file(GLOB EXACAM_SRC *.cpp)
SET(SOURCES ${SOURCES} ${EXACAM_SRC})
add_executable(exacam ${SOURCES})
target_link_libraries(exacam ${LIBUSB_LIBRARIES})

