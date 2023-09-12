cmake_minimum_required(VERSION 3.6)

project(minalyzer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

# required packages
find_package(Boost 1.49 REQUIRED COMPONENTS thread system program_options)
find_package(Git 1.8 REQUIRED)

set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
find_package(Midas REQUIRED
    HINTS
        $ENV{MIDASSYS}
        ~/midas
        ~/online/modules/midas
        ${CMAKE_CURRENT_SOURCE_DIR}/../midas
    )
# To be implemented in midas/MidasConfig.cmake at some point
if(NOT EXISTS ${MIDAS_LIBRARY_DIRS})
    message(FATAL_ERROR "MIDAS not found.
    Please add your instance of midas to \"cmake -DCMAKE_PREFIX_PATH=PATHTOMIDAS ..\".
    Also make sure that you did make install after installing midas.
    If you have, consider deleting your build directory")
endif()
set(MIDASSYS ${MIDAS_LIBRARY_DIRS}/..)
message(STATUS "MIDASSYS = ${MIDASSYS}")
include(${MIDASSYS}/lib/midas-targets.cmake)

list (APPEND MIDAS_INCLUDE_DIRS
    ${MIDASSYS}/manalyzer
    ${MIDASSYS}/midasio
    ${MIDASSYS}/mvodb
    )
link_directories(${MIDAS_LIBRARY_DIRS})
include_directories(${MIDAS_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)

if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(ROOT REQUIRED COMPONENTS
    MathCore RIO Hist Tree Net Html RHTTP Gui XMLIO
)
message(STATUS "ROOT_LIBRARIES = ${ROOT_LIBRARIES}")

if(ROOT_FOUND)
   include_directories(${ROOT_INCLUDE_DIRS})
endif()
message(STATUS "ROOT_INCLUDE_DIRS = ${ROOT_INCLUDE_DIRS}")

if (NOT CMAKE_CXX_STANDARD)    # let top-level project overwrite c++ standard
   set(CMAKE_CXX_STANDARD 17)
endif()

find_package(Git 1.8 REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --long --dirty --abbrev=16 --all
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE ANALYZER_GIT_REF
)
message(STATUS "ANALYZER_GIT_REF: ${ANALYZER_GIT_REF}")

add_subdirectory(analyzer)