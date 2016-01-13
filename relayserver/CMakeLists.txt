cmake_minimum_required(VERSION 2.6)

project(relayserver)

set( COMMON_SOURCE_DIR ${relayserver_SOURCE_DIR}/../common )
set(CMAKE_C_COMPILER "g++")

include_directories( ${relayserver_SOURCE_DIR}/../common )

add_executable(relayserver Accept.cc ${COMMON_SOURCE_DIR}/Buffer.cc Timer.cc EventLoopThread.cc Thread.cc Socket.cc Poll.cc UDPConnection.cc TCPConnection.cc Connection.cc Authentication.cpp Channel.cc Session.cc SessionManager.cc Hash.cc ${relayserver_SOURCE_DIR}/../common/PackDef.cc ${COMMON_SOURCE_DIR}/ParsePdu.cc ${COMMON_SOURCE_DIR}/TraceLog.cpp ${COMMON_SOURCE_DIR}/Common.c ${COMMON_SOURCE_DIR}/LpConfig.c main.cc)



set(CMAKE_BUILE_TYPE DEBUG)

add_definitions(-Wall)
add_definitions(-std=c++0x)
add_definitions(-fpermissive)

target_link_libraries(relayserver pthread)
