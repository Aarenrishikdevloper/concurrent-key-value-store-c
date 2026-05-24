cmake_minimum_required(VERSION 3.28)
project(key_value_store C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-ggdb3 -Wall)

set(SHARED_SOURCES
        kvstore.h kvstore.c
        queue.h queue.c
        stats.h stats.c
        worker.h worker.c
)

add_executable(dbserver
        lab3.h
        dbserver.c
        ${SHARED_SOURCES}
)

add_executable(dbclient
        dbclient.c
)

# Try find_package first
find_package(ZLIB)
if(ZLIB_FOUND)
    target_link_libraries(dbserver PRIVATE ZLIB::ZLIB)
else()
    # Fallback: link -lz directly
    target_link_libraries(dbserver PRIVATE z)
endif()

if(APPLE)
    execute_process(
            COMMAND brew --prefix argp-standalone
            OUTPUT_VARIABLE ARGP_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if(ARGP_PREFIX)
        target_include_directories(dbserver PRIVATE ${ARGP_PREFIX}/include)
        target_link_libraries(dbserver PRIVATE ${ARGP_PREFIX}/lib/libargp.a)
        target_include_directories(dbclient PRIVATE ${ARGP_PREFIX}/include)
        target_link_libraries(dbclient PRIVATE ${ARGP_PREFIX}/lib/libargp.a)
    endif()
endif()

set_property(DIRECTORY PROPERTY ADDITIONAL_CLEAN_FILES "data.[0-9]*")