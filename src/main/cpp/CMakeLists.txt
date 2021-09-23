cmake_minimum_required(VERSION 3.4.1)

set(SOURCES)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/giflib/openbsd-reallocarry.c)

add_library(native-lib SHARED ${SOURCES} )

set(LIBS)
list(APPEND LIBS
        jnigraphics
        log
        )

target_link_libraries(native-lib ${LIBS} )