cmake_minimum_required(VERSION 3.10)


set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools "/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf")
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)


project(lvgl)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(lv_100ask_xz_ai)
add_subdirectory(lvgl)

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})

#add_subdirectory(lv_100ask_xz_ai)

# # 在 CMakeLists.txt 中添加
### include_directories(${CMAKE_CURRENT_LIST_DIR}/freetype-2/include)# 手动添加部分
include_directories(${PROJECT_SOURCE_DIR}/lvgl/freetype2/include)
# include_directories(${CMAKE_CURRENT_LIST_DIR}/freetype-2/include)# 手动添加部分

# include_directories(${PROJECT_SOURCE_DIR}/lvgl/libfreetype/include/freetype2)
# link_directories(${PROJECT_SOURCE_DIR}/lvgl/libfreetype/lib)
include_directories(${PROJECT_SOURCE_DIR}/lvgl/libpng/include)
link_directories(${PROJECT_SOURCE_DIR}/lvgl/libpng/lib)
include_directories(${PROJECT_SOURCE_DIR}/lvgl/libz/include)
link_directories(${PROJECT_SOURCE_DIR}/lvgl/libz/lib)

# find_package(Freetype REQUIRED)
# include_directories(${FREETYPE_INCLUDE_DIRS})


# # 或者使用 find_package
# find_package(Freetype REQUIRED)
# include_directories(${FREETYPE_INCLUDE_DIRS})


add_executable(main main.c mouse_cursor_icon.c)

include(${CMAKE_CURRENT_LIST_DIR}/lvgl/tests/FindLibDRM.cmake)
include_directories(${Libdrm_INCLUDE_DIRS})

find_package(SDL2)
find_package(SDL2_image)
include_directories(${CMAKE_SOURCE_DIR}/lv_100ask_xz_ai ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# target_link_libraries(main lv_100ask_xz_ai  lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${Libdrm_LIBRARIES} m pthread freetype png)
target_link_libraries(main lv_100ask_xz_ai  lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}  m pthread freetype png z)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/lv_xiaozhiui DEPENDS main)

