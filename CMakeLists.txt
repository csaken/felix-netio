cmake_minimum_required (VERSION 2.8.8)
project (libnetio)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O3")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -opt-prefetch -unroll-aggressive -march=native -mtune=native")
endif()

find_path(EVENT2 "event2/")
include_directories("." ${EVENT2})

find_library(LIBEVENT libevent.a)
find_library(LIBEVENT_PTHREAD libevent_pthreads.a)


file(GLOB NETIO_SOURCES src/netio.cpp src/serialization.cpp src/message.cpp src/backends.cpp src/posix/*.cpp src/libevent/*.cpp)
add_library(netio ${NETIO_SOURCES})
target_link_libraries(netio  	${LIBEVENT} ${LIBEVENT_PTHREAD})


file(GLOB NETIO_TEST_SOURCES test/*.cpp)
add_executable(netio_test ${NETIO_TEST_SOURCES})
target_link_libraries(netio_test  	netio boost_system rt pthread)

add_executable(netio_server src/main_netio_server.cpp)
target_link_libraries(netio_server 	netio boost_system rt pthread)

add_executable(netio_datagen src/main_netio_datagen.cpp)
target_link_libraries(netio_datagen	netio boost_system rt pthread) 

