include_directories(include)
include_directories(../exception/include)

add_subdirectory(src)

install_other(SUBDIRNAME include GLOB "include/*.hh" "include/*.h")
install_other(SUBDIRNAME tables GLOB "tables/*.*")
install_other(SUBDIRNAME config GLOB "config/*.txt")
