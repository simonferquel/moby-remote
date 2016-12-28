cmake_minimum_required(VERSION 2.6.2)
set(gtest_force_shared_crt ON)
set(gtest_catch_exceptions ON)
project(moby-remote)
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++14")
endif()


include_directories(include)


file(GLOB_RECURSE PUBLIC_INCLUDE_FILES
	"include/*.h")
	
file(GLOB_RECURSE COMMON_LIB_SOURCES
    "src/common-lib/*.cpp" "src/common-lib/*.h" "include/common.h" "include/protocol.h")
file(GLOB_RECURSE CLIENT_LIB_SOURCES
    "src/client-lib/*.cpp" "src/client-lib/*.h" "include/common.h" "include/protocol.h" "include/client.h")
file(GLOB_RECURSE SERVER_LIB_SOURCES
    "src/server-lib/*.cpp" "src/server-lib/*.h" "include/common.h" "include/protocol.h" "include/server.h")
file(GLOB_RECURSE SERVER_SOURCES
	"src/server/*.cpp" "src/server/*.h"  "include/common.h" "include/protocol.h" "include/server.h")


add_library(common ${COMMON_LIB_SOURCES})
add_library(clientlib ${CLIENT_LIB_SOURCES})
add_library(serverlib ${SERVER_LIB_SOURCES})
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server serverlib common)
if(UNIX)
	target_link_libraries(server serverlib common execinfo)
endif()


add_subdirectory(googletest)
enable_testing(true)
include_directories(${gtest_SOURCE_DIR}/include include)

add_executable(TransportTests src/Tests/TransportTests.cpp)
target_link_libraries(TransportTests gtest gtest_main common)
if(UNIX)
	target_link_libraries(TransportTests gtest gtest_main common execinfo)
endif()
add_test(TransportTests TransportTests)

add_executable(EndToEndTests src/Tests/EndToEnd.cpp)
target_link_libraries(EndToEndTests gtest gtest_main clientlib serverlib common)
if(UNIX)
	target_link_libraries(EndToEndTests gtest gtest_main clientlib serverlib common execinfo)
endif()
add_test(EndToEndTests EndToEndTests)


install(TARGETS common DESTINATION lib)
install(TARGETS clientlib DESTINATION lib)
install(TARGETS serverlib DESTINATION lib)
install(TARGETS server DESTINATION bin)
install(FILES ${PUBLIC_INCLUDE_FILES} DESTINATION include/moby-remote)