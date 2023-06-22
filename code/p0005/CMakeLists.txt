
project(mu3eRec CXX)

# OPTIONS
option(MU3E_TRIREC_DISPLAY "Build mu3eTrirec display" OFF)

# SOURCES
file(GLOB_RECURSE SOURCES
    src/*.cpp src/*.h PATTERN *.hpp PATTERN *.inc
)
if(NOT MU3E_TRIREC_DISPLAY)
    list(FILTER SOURCES EXCLUDE REGEX display/EventDisplay.cpp)
endif()

# LIBRARY
add_library(mu3eRecLib SHARED ${SOURCES})
set_target_properties(mu3eRecLib PROPERTIES
    EXPORT_NAME rec
    OUTPUT_NAME mu3e_rec
)
add_library(mu3e::rec ALIAS mu3eRecLib)

target_link_libraries(mu3eRecLib PUBLIC watson mu3e::tools /psi/home/langenegger/mu3e/mu3eanca/db0/cdb1/lib/libCDB.so /psi/home/langenegger/data/mongo/install/lib64/libbsoncxx.so /psi/home/langenegger/data/mongo/install/lib64/libmongoc-1.0.so /psi/home/langenegger/data/mongo/install/lib64/libmongocxx.so /usr/lib64/libcurl.so)
target_include_directories(mu3eRecLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    /psi/home/langenegger/mu3e/mu3eanca/db0/cdb1
)

if(MU3E_TRIREC_DISPLAY)
    target_compile_definitions(mu3eRecLib PUBLIC MU3E_TRIREC_DISPLAY)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
    target_link_libraries(mu3eRecLib PUBLIC
        ${GTKMM_LIBRARIES}
    )
    target_include_directories(mu3eRecLib SYSTEM PUBLIC
        ${GTKMM_INCLUDE_DIRS}
    )
endif()

# EXECUTABLE
add_executable(mu3eRec trirec.cpp)
set_target_properties(mu3eRec PROPERTIES OUTPUT_NAME mu3eTrirec)
target_link_libraries(mu3eRec PRIVATE mu3e::rec /psi/home/langenegger/mu3e/mu3eanca/db0/cdb1/lib/libCDB.so /psi/home/langenegger/data/mongo/install/lib64/libbsoncxx.so /psi/home/langenegger/data/mongo/install/lib64/libmongoc-1.0.so /psi/home/langenegger/data/mongo/install/lib64/libmongocxx.so /usr/lib64/libcurl.so)
target_include_directories(mu3eRec PRIVATE src /psi/home/langenegger/mu3e/mu3eanca/db0/cdb1)
install(TARGETS mu3eRec)

# TEST
add_subdirectory(test)



# INSTALL
include(GNUInstallDirs)
# headers
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mu3e/rec/
    FILES_MATCHING PATTERN *.h PATTERN *.hpp PATTERN *.inc
)
# targets
install(TARGETS mu3eRecLib EXPORT mu3e-rec-targets)
install(EXPORT mu3e-rec-targets
    NAMESPACE mu3e::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mu3e
)
