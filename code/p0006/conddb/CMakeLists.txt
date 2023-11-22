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

# arch linux: pacman -S mongo-c-driver
#ul find_package(bson-1.0 REQUIRED)
#ul find_package(mongoc-1.0 REQUIRED)
# ```
# cd "$HOME/.cache"
# git clone https://github.com/mongodb/mongo-cxx-driver
# mkdir cmake-build && cd cmake-build
# cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local/mongo -DBUILD_SHARED_AND_STATIC_LIBS=ON -DENABLE_TESTS=OFF ..
# make && make install
# ```
#find_package(bsoncxx REQUIRED CONFIGS ${MONGO}/install/lib64/cmake/libbson-1.0) 
#find_package(mongocxx REQUIRED CONFIGS ${MONGO}/install/lib64/cmake/libmongoc-1.0) 

target_link_libraries(mu3eCondDB PUBLIC
    /data/user/langenegger/mongo/install/lib64/libbsoncxx.so
    /data/user/langenegger/mongo/install/lib64/libmongoc-1.0.so
    /data/user/langenegger/mongo/install/lib64/libmongocxx.so
    /usr/lib64/libcurl.so
)
target_include_directories(mu3eCondDB PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    /data/user/langenegger/mongo/install/include/mongocxx/v_noabi
    /data/user/langenegger/mongo/install/include/bsoncxx/v_noabi
)

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
