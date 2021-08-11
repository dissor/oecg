file(GLOB_RECURSE SOURCES *.c)


set(target porting)
add_library(${target} STATIC ${SOURCES})
target_link_libraries(${target} driver lvgl)