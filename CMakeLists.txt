cmake_minimum_required(VERSION 2.6.2)
set(gtest_force_shared_crt ON)
set(gtest_catch_exceptions ON)
project(moby-remote)
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++14")
	SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
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
file(GLOB_RECURSE RANDOMSEND_SOURCES
	"src/randomsend/*.cpp" "src/randomsend/*.h"  "include/common.h" "include/protocol.h" "include/server.h" "include/client.h")
file(GLOB_RECURSE RANDOMRECV_SOURCES
	"src/randomrecv/*.cpp" "src/randomrecv/*.h"  "include/common.h" "include/protocol.h" "include/server.h" "include/client.h")


add_library(common ${COMMON_LIB_SOURCES})
add_library(clientlib ${CLIENT_LIB_SOURCES})
add_library(serverlib ${SERVER_LIB_SOURCES})
add_executable(server ${SERVER_SOURCES})
add_executable(randomsend ${RANDOMSEND_SOURCES})
add_executable(randomrecv ${RANDOMRECV_SOURCES})
set_target_properties(common PROPERTIES COMPILE_PDB_NAME common )
set_target_properties(clientlib PROPERTIES COMPILE_PDB_NAME clientlib )
set_target_properties(serverlib PROPERTIES COMPILE_PDB_NAME serverlib )
target_link_libraries(server serverlib common)
target_link_libraries(randomsend serverlib clientlib common)
target_link_libraries(randomrecv serverlib clientlib common)


add_subdirectory(googletest)
enable_testing(true)
include_directories(${gtest_SOURCE_DIR}/include include)

add_executable(TransportTests src/Tests/TransportTests.cpp)
target_link_libraries(TransportTests gtest gtest_main common)
add_test(TransportTests TransportTests)

add_executable(EndToEndTests src/Tests/EndToEnd.cpp)
target_link_libraries(EndToEndTests gtest gtest_main clientlib serverlib common)
add_test(EndToEndTests EndToEndTests)


install(TARGETS common DESTINATION lib)
install(TARGETS clientlib DESTINATION lib)
install(TARGETS serverlib DESTINATION lib)
install(TARGETS server DESTINATION bin)
install(TARGETS randomsend DESTINATION bin)
install(TARGETS randomrecv DESTINATION bin)
install(FILES ${PUBLIC_INCLUDE_FILES} DESTINATION include/moby-remote)
IF(MSVC)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/common.pdb DESTINATION lib)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/clientlib.pdb DESTINATION lib)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/serverlib.pdb DESTINATION lib)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/server.pdb DESTINATION bin)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/randomsend.pdb DESTINATION bin)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/randomrecv.pdb DESTINATION bin)
ENDIF(MSVC)


