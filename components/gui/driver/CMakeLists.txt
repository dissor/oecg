include_directories(.)

file(GLOB_RECURSE SOURCES *.c *.cpp)
add_library(driver STATIC ${SOURCES})
