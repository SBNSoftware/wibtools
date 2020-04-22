include_directories(include)
add_subdirectory(src)

install(DIRECTORY  include  DESTINATION ${product}/${version}/ )
