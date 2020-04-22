include_directories(include)
include_directories(../exception/include)

add_subdirectory(src)

install(DIRECTORY  include  DESTINATION ${product}/${version}/ )
install(DIRECTORY  tables   DESTINATION ${product}/${version}/ )
install(DIRECTORY  config   DESTINATION ${product}/${version}/ )
