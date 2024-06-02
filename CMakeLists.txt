cmake_minimum_required(VERSION 3.16)
project(cholesky-hh)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(Hedgehog_INCLUDE_DIR lib/hedgehog/)

include_directories(lib/)

# Need to defined variable Hedgehog_INCLUDE_DIR to the hedgehog path hedgehog
find_package(Hedgehog REQUIRED)

# OpenBLAS
find_package(OpenBLAS QUIET)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${Hedgehog_CXX_FLAGS})

if (NOT MSVC)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -Wextra -Werror -Wuninitialized -pedantic -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wuninitialized -pedantic -g")
endif (NOT MSVC)

# files
set(cholesky_hh_files
		src/main.cc
		src/state/decompose_state.h
		src/state/decompose_state_manager.h
		src/graph/cholesky_decomposition_graph.h
		src/data/matrix_data.h
		src/data/matrix_block_data.h
		src/data/block_types.h
		src/task/split_matrix_task.h
		src/utils.h
		src/task/compute_diagonal_block_task.h
		src/task/compute_column_block_task.h
		src/task/update_submatrix_block_task.h
		src/state/update_submatrix_state.h
		src/data/triple_block_data.h
		src/state/update_submatrix_state_manager.h
    src/config.cc src/config.h
)

# executable
add_executable(cholesky-hh ${cholesky_hh_files})
target_link_libraries(cholesky-hh openblas lapack)
