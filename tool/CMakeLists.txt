include_directories(include)

add_subdirectory(src/helpers)
add_subdirectory(src/helpers/StatusDisplay)
add_subdirectory(src/tool)
add_subdirectory(src/mbb_device)
add_subdirectory(src/wib_device)

file(GLOB StatusDisplayy_headers   "include/helpers/StatusDisplay/*.hh" )
install(FILES ${StatusDisplayy_headers}  DESTINATION ${product}/${version}/include)
install(DIRECTORY  config   DESTINATION ${product}/${version}/ )
