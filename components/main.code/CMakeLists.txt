set(target demo)

add_executable(${target} demo.cpp)

target_link_libraries(${target} driver porting lvgl)
