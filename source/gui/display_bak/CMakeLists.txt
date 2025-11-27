# file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE SOURCES      ###递归搜索子目录
# file(GLOB SOURCES 
    "src/*.c"
    "src/*.cc"
    "src/*.cpp"
    "src/*.cxx"
)

add_library(display STATIC ${SOURCES})

target_include_directories(display SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(lv_100ask_xz_ai SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/lvgl/libfreetype/include/freetype2)
target_include_directories(display SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/lvgl/freetype2/include)

target_link_libraries(display  pthread)

