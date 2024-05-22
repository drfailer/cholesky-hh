cmake_minimum_required(VERSION 3.16)
project(cholesky-hh)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

set(Hedgehog_INCLUDE_DIR ./lib/AtomicHedgehog/)

# Need to defined variable Hedgehog_INCLUDE_DIR to the hedgehog path hedgehog
find_package(Hedgehog REQUIRED)

# OpenBLAS
find_package(OpenBLAS QUIET)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${Hedgehog_CXX_FLAGS})

if (NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic -g")
endif (NOT MSVC)

# files
set(cholesky_hh_files
    src/main.cc
    )

# executable
add_executable(cholesky-hh ${cholesky_hh_files})
