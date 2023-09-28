
add_subdirectory(src/helpers)
add_subdirectory(src/helpers/StatusDisplay)
add_subdirectory(src/tool)
add_subdirectory(src/mbb_device)
add_subdirectory(src/wib_device)

install_other(SUBDIRNAME config LIST "config/mfCommonDefs.mk")
install_other(SUBDIRNAME include GLOB "include/helpers/StatusDisplay/*.hh")
