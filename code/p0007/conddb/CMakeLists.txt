#

option(MU3E_CONDDB "Build mu3eCondDB" OFF)
if(NOT MU3E_CONDDB)
    return()
endif()

# SOURCES
file(GLOB_RECURSE SOURCES
    src/*.cc src/*.hh src/*.cpp src/*.h src/*.hpp src/*.inc
)

# LIBRARY
add_library(mu3eCondDB SHARED ${SOURCES})
set_target_properties(mu3eCondDB PROPERTIES
    EXPORT_NAME conddb
    OUTPUT_NAME mu3e_conddb
)
add_library(mu3e::conddb ALIAS mu3eCondDB)

find_package(bsoncxx)

IF( bsoncxx_FOUND )
  find_package(mongocxx REQUIRED HINTS ~/.local/mongo)

  target_link_libraries(mu3eCondDB PUBLIC
    #mongo::bson_shared mongo::mongoc_shared
    #mongo::bsoncxx_shared mongo::mongocxx_shared
    mongo::bsoncxx_static mongo::mongocxx_static
  )
  target_include_directories(mu3eCondDB PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
ELSE()
  # note: the installation below does not  work. "-DMU3E_CONDDB_MONGO_DIR=/data/user/langenegger/mongo/install" does work
  SET(MU3E_CONDDB_MONGO_DIR /data/experiment/mu3e/code/mongo-cxx-driver-install CACHE STRING "bla")

  target_link_libraries(mu3eCondDB PUBLIC
    ${MU3E_CONDDB_MONGO_DIR}/lib64/libbsoncxx.so
    ${MU3E_CONDDB_MONGO_DIR}/lib64/libmongoc-1.0.so
    ${MU3E_CONDDB_MONGO_DIR}/lib64/libmongocxx.so
    /usr/lib64/libcurl.so
  )
  target_include_directories(mu3eCondDB PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${MU3E_CONDDB_MONGO_DIR}/include/mongocxx/v_noabi
    ${MU3E_CONDDB_MONGO_DIR}/include/bsoncxx/v_noabi
  )
ENDIF()

# TEST
add_subdirectory(test)

# INSTALL
include(GNUInstallDirs)
# headers
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/mu3e/conddb/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mu3e/conddb/
    FILES_MATCHING PATTERN *.h PATTERN *.hpp PATTERN *.inc
)
# targets
install(TARGETS mu3eCondDB EXPORT mu3e-conddb-targets)
install(EXPORT mu3e-conddb-targets
    NAMESPACE mu3e::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mu3e
)
