# set(target gui)

# add_library(${target} STATIC)
# target_include_directories(${target} PUBLIC)
# target_sources(${target} PUBLIC)
include_directories(.)

add_subdirectory(driver)
add_subdirectory(lvgl)
add_subdirectory(porting)

link_libraries(driver lvgl porting)