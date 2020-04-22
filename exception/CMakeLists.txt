include_directories(include)
add_subdirectory(src/WIBException)

install(DIRECTORY  include  DESTINATION ${product}/${version}/ )
