cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
PROJECT(asiotoken)

option(BUILD_SHARED_LIBS "Build the shared library" OFF)
option(Boost_USE_STATIC_LIBS "Use Boost static version" ON)

set(MAJOR 1)
set(MINOR 6)
set(PATCH 0)

if(NOT CMAKE_BUILD_TYPE )
MESSAGE(STATUS "not define build type, set to release" )
set(CMAKE_BUILD_TYPE Release )
elseif(NOT (${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "Debug" ))
MESSAGE(SEND_ERROR "CMAKE_BUILD_TYPE must be either Release or Debug")
return()
endif()

set(BOOST_VER "1.60.0" CACHE STRING "boost version" )

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src ALL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/internal ALL_SRC)
file(GLOB ALL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/src/*.h )
set(SIO_INCLUDEDIR ${CMAKE_CURRENT_LIST_DIR})

add_library(asiotoken ${ALL_SRC})
target_include_directories(asiotoken PRIVATE ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/src
)

set_property(TARGET asiotoken PROPERTY CXX_STANDARD 11)
set_property(TARGET asiotoken PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(asiotoken PRIVATE ${Boost_LIBRARIES})
if(BUILD_SHARED_LIBS)
set_target_properties(asiotoken
        PROPERTIES
                SOVERSION ${MAJOR}
                VERSION ${MAJOR}.${MINOR}.${PATCH}
        )
endif()
list(APPEND TARGET_LIBRARIES asiotoken)



install(FILES ${ALL_HEADERS}
    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/include"
)

install(TARGETS ${TARGET_LIBRARIES}
    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/lib/${CMAKE_BUILD_TYPE}"
)

install(FILES ${Boost_LIBRARIES} 
    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/lib/${CMAKE_BUILD_TYPE}"
)
