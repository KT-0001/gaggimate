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
        -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\",\"UTF8ToString\",\"stringToUTF8\",\"lengthBytesUTF8\"]' \
        -s EXPORTED_FUNCTIONS='[\"_main\",\"_getCurrentScreenName\",\"_simulateButtonClick\",\"_malloc\",\"_free\"]' \
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/index.html \
        -s ASYNCIFY \
        -s EXIT_RUNTIME=0 \
        -s ASSERTIONS=1 \
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../data@/data \
        ")
endif()

# LVGL configuration
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)

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

# UI sources from display project
set(UI_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/display/ui/default/lvgl)
file(GLOB UI_C_SOURCES 
    ${UI_BASE_DIR}/ui.c
    ${UI_BASE_DIR}/ui_helpers.c
    ${UI_BASE_DIR}/components/*.c
    ${UI_BASE_DIR}/screens/*.c
    ${UI_BASE_DIR}/images/*.c
)
file(GLOB UI_CPP_SOURCES
    ${UI_BASE_DIR}/ui_theme_manager.cpp
    ${UI_BASE_DIR}/ui_themes.cpp
)
set(UI_SOURCES ${UI_C_SOURCES} ${UI_CPP_SOURCES})

# Simulator executable
set(SIMULATOR_SOURCES
    main.c
    mouse_cursor_icon.c
    ui_stubs.cpp
    test_helpers.cpp
    ${UI_SOURCES}
)

add_executable(lvgl_sim ${SIMULATOR_SOURCES})

target_include_directories(lvgl_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display/ui/default/lvgl
    ${LVGL_DIR}
)

target_link_libraries(lvgl_sim lvgl)
# Copy generated HTML as index.html for web server
add_custom_command(TARGET lvgl_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/lvgl_sim.html
    ${CMAKE_CURRENT_BINARY_DIR}/index.html
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/test_framework.js
    ${CMAKE_CURRENT_BINARY_DIR}/test_framework.js
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cases.js
    ${CMAKE_CURRENT_BINARY_DIR}/test_cases.js
)

