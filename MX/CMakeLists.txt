cmake_minimum_required(VERSION 3.0)

project (MXCore C CXX)

option(USR_NP "Choose use PWM or NP LED" off)

if (USE_NP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_NP=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_NP=1")
endif(USE_NP)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/AUDIO)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/Hand)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/KEY)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/LED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/LED/CXXPort)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/LOOP)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/MUX)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/PARAM)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/PM)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/MX/SimpleLED)

file(GLOB_RECURSE MX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/MX/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(MXCore ${MX_SOURCE})

