#

# SOURCES
file(GLOB_RECURSE SOURCES
    src/*.h src/*.hpp src/*.cpp
)

# LIBRARY
add_library(mu3eUtilLib SHARED ${SOURCES})
add_library(mu3e::util ALIAS mu3eUtilLib)
set_target_properties(mu3eUtilLib PROPERTIES
    OUTPUT_NAME mu3e_util
    EXPORT_NAME util
)

target_compile_options(mu3eUtilLib PUBLIC
    "-mfpmath=sse" "-msse2"
    "-Wformat=2"
    "-Wold-style-cast"
)

option(MU3E_UTIL_ASAN "add `-fsanitize=address` compile/link options" OFF)
if(MU3E_UTIL_ASAN AND CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "[mu3e/util] add `-fsanitize=address` compile/link options")
    target_compile_options(mu3eUtilLib PUBLIC
        -fsanitize=address,undefined
    )
    target_link_options(mu3eUtilLib PUBLIC
        #-fno-omit-frame-pointer
        -fsanitize=address,undefined
    )
endif()

find_package(fmt QUIET)
if(NOT fmt_FOUND)
    message(STATUS "[mu3e/util] fmt NOT FOUND -> FetchContent(fmt ...)")
    FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
    )
    set(FMT_INSTALL TRUE)
    FetchContent_MakeAvailable(fmt)
    set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
if(DEFINED ENV{BOOST_DIR})
    set(ENV{Boost_DIR} $ENV{BOOST_DIR})
endif()
find_package(Boost 1.54 REQUIRED COMPONENTS filesystem program_options system)
find_package(ROOT 6 REQUIRED)
find_package(Geant4 10.7.0 REQUIRED)

message(STATUS "[mu3e/util] Boost_VERSION: ${Boost_VERSION}")
message(STATUS "[mu3e/util] ROOT_VERSION: ${ROOT_VERSION}")
message(STATUS "[mu3e/util] Geant4_VERSION: ${Geant4_VERSION}")

separate_arguments(ROOT_COMPILE_OPTS UNIX_COMMAND "${ROOT_CXX_FLAGS}")
if(FALSE
    # require c++17
    OR (${CMAKE_CXX_STANDARD} EQUAL 17 AND "-std=c++17" IN_LIST ROOT_COMPILE_OPTS)
    OR (${CMAKE_CXX_STANDARD} EQUAL 17 AND "-std=c++1z" IN_LIST ROOT_COMPILE_OPTS)
    # or c++14
    OR (${CMAKE_CXX_STANDARD} EQUAL 14 AND "-std=c++14" IN_LIST ROOT_COMPILE_OPTS)
)
else()
    message(SEND_ERROR "[mu3e/util] CMAKE_CXX_STANDARD (${CMAKE_CXX_STANDARD}) does not match ROOT_CXX_FLAGS (${ROOT_CXX_FLAGS})")
endif()

target_link_libraries(mu3eUtilLib PUBLIC
    fmt
    ${Boost_LIBRARIES}
    ${ROOT_LIBRARIES}
    ${Geant4_LIBRARIES}
    dl pthread
)
target_include_directories(mu3eUtilLib SYSTEM PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${ROOT_INCLUDE_DIRS}
    ${Geant4_INCLUDE_DIRS}
)
target_include_directories(mu3eUtilLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(mu3eUtilLib PUBLIC
    "MU3E_GEANT4_VERSION_MAJOR=${Geant4_VERSION_MAJOR}"
    "MU3E_GEANT4_VERSION_MINOR=${Geant4_VERSION_MINOR}"
    "MU3E_GEANT4_VERSION_PATCH=${Geant4_VERSION_PATCH}"
)

# TEST
add_subdirectory(test)

#
add_executable(mu3e-root-diff root-diff.cpp)
target_link_libraries(mu3e-root-diff mu3e::util)



# INSTALL
include(GNUInstallDirs)
# headers
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mu3e/util/
    FILES_MATCHING PATTERN *.h PATTERN *.hpp
)
# targets
install(TARGETS mu3eUtilLib EXPORT mu3e-util-targets)
install(EXPORT mu3e-util-targets
    NAMESPACE mu3e::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mu3e
)
