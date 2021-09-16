include_directories(include)
add_subdirectory(src)

install_other(SUBDIRNAME include GLOB "include/*.hh")
