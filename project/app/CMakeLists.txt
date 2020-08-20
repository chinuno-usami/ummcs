# User code, which contains the main() function and builds to an executable
#ultimate mecha musume core system

set(APP_NAME ummcs)

set(SOURCE_FILES
    main.cpp 
    rss.cpp
    storage.cpp
    rss_parser.cpp
    configure.cpp
    modules/module.cpp
    modules/nbnhhsh.cpp
    modules/core.cpp
    modules/feed.cpp
    modules/autoban.cpp
    modules/bilibili.cpp
    modules/saucenao.cpp
    ) # Add new source files into this list

find_package(ZLIB)
find_package(RocksDB CONFIG REQUIRED)

find_path(GUMBO_INCLUDE_DIR gumbo.h )
find_library(GUMBO_LIBRARY gumbo)
link_libraries(${GUMBO_LIBRARY} ${PROJECT_SOURCE_DIR}/feedparser/target/release/libfeedparser.a )

add_executable(${APP_NAME} ${SOURCE_FILES})

include_directories(. /usr/include/libxml2 ${GUMBO_INCLUDE_DIR})
target_compile_features(${APP_NAME} PRIVATE cxx_std_17) # Enforce C++17 or newer
set_target_properties(${APP_NAME} PROPERTIES CXX_EXTENSIONS off)

if (MSVC)
    # Force MSVC to use utf-8 encoding regardless of whether the BOM exists
    target_compile_options(${APP_NAME} PRIVATE "/utf-8")
endif ()

# Warnings and errors settings
# Use highest reasonable warning level, and treat warnings as errors
if (MSVC) # Visual Studio
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]") # If default /W3 presents
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # Change /W3 to /W4
    else ()
        target_compile_options(${APP_NAME} PRIVATE /W4) # Add /W4 directly
    endif ()
    target_compile_options(${APP_NAME} PRIVATE /WX) # Treat warnings as errors
    # Treat all header files specified by angle brackets to be system headers, and ignore all those warnings
    target_compile_options(${APP_NAME} PRIVATE 
        /experimental:external /external:W0 /external:anglebrackets)
else () # Not Visual Studio, assuming gcc or clang
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif ()

target_link_libraries(${APP_NAME} PRIVATE
    miraipp
    RocksDB::rocksdb RocksDB::rocksdb-shared
    ZLIB::ZLIB
    )
