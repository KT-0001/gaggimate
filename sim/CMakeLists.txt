cmake_minimum_required(VERSION 3.13)
project(lvgl_simulator C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Emscripten settings for web build
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
    # Emscripten compile flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s WASM=1")
    
    # Emscripten link flags
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s USE_SDL=2 \
        -s WASM=1 \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s TOTAL_MEMORY=67108864 \
        -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]' \
        -s EXPORTED_FUNCTIONS='[\"_main\"]' \
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/index.html \
        -s ASYNCIFY \
        -s EXIT_RUNTIME=0 \
        -s ASSERTIONS=1 \
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../data@/data \
        ")
endif()

# LVGL configuration
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf_simulator.h CACHE STRING "" FORCE)
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
add_compile_definitions(LV_CONF_PATH="${LV_CONF_PATH}")

# LVGL library
set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.pio/libdeps/display/lvgl)

# If LVGL is not in .pio, try to find it or clone it
if(NOT EXISTS ${LVGL_DIR})
    set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lvgl)
    if(NOT EXISTS ${LVGL_DIR})
        message(STATUS "LVGL not found. Please run: git clone --depth 1 --branch release/v8.4 https://github.com/lvgl/lvgl.git sim/lvgl")
        message(FATAL_ERROR "LVGL directory not found")
    endif()
endif()

file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/src/*.c)

add_library(lvgl STATIC ${LVGL_SOURCES})
target_include_directories(lvgl PUBLIC 
    ${LVGL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Simulator executable
set(SIMULATOR_SOURCES
    main.c
    mouse_cursor_icon.c
)

add_executable(lvgl_sim ${SIMULATOR_SOURCES})

target_include_directories(lvgl_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display
    ${LVGL_DIR}
)

target_link_libraries(lvgl_sim lvgl)

# Copy index.html to build directory for standalone use
if(NOT EMSCRIPTEN)
    add_custom_command(TARGET lvgl_sim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        ${CMAKE_CURRENT_BINARY_DIR}/index.html
    )
endif()
