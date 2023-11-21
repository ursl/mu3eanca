#

# SOURCES
file(GLOB_RECURSE SOURCES
    src/*.cpp src/*.h src/*.hpp src/*.inc
)

# LIBRARY
add_library(mu3eRecLib SHARED ${SOURCES})
set_target_properties(mu3eRecLib PROPERTIES
    EXPORT_NAME rec
    OUTPUT_NAME mu3e_rec
)
add_library(mu3e::rec ALIAS mu3eRecLib)

target_link_libraries(mu3eRecLib PUBLIC watson mu3eCondDB mu3e::tools)
target_include_directories(mu3eRecLib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# EXECUTABLE
add_executable(mu3eRec trirec.cpp)
set_target_properties(mu3eRec PROPERTIES OUTPUT_NAME mu3eTrirec)
target_link_libraries(mu3eRec PRIVATE mu3e::rec mu3eCondDB)
target_include_directories(mu3eRec PRIVATE src mu3eCondDB)
install(TARGETS mu3eRec)

if(MU3E_DISPLAY)
    target_link_libraries(mu3eRec PRIVATE mu3e::display)
endif()

# TEST
add_subdirectory(test)



# INSTALL
include(GNUInstallDirs)
# headers
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/mu3e/rec/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mu3e/rec/
    FILES_MATCHING PATTERN *.h PATTERN *.hpp PATTERN *.inc
)
# targets
install(TARGETS mu3eRecLib EXPORT mu3e-rec-targets)
install(EXPORT mu3e-rec-targets
    NAMESPACE mu3e::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mu3e
)
