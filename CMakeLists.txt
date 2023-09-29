cmake_minimum_required(VERSION 3.8.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Hairsplitter)
#add_executable(${PROJECT_NAME} "main.cpp")

# Local header files here ONLY
set(TARGET_H
    input_output.h
    robin_hood.h
    read.h
    sequence.h
    tools.h
    Partition.h
   )

# Local source files here
set(TARGET_SRC
    edlib/src/edlib.cpp
    input_output.cpp
    read.cpp
    tools.cpp
    sequence.cpp
    Partition.cpp
    )

file (GLOB CONVERT_SOURCE2 "gfa2fa.cpp")
add_executable(gfa2fa ${CONVERT_SOURCE2})

file (GLOB SOURCE_CREATE_NEW_CONTIG "create_new_contigs.cpp" "input_output.cpp" "robin_hood.h" "tools.cpp" "read.cpp" "sequence.cpp" "Partition.cpp")
add_executable(create_new_contigs ${SOURCE_CREATE_NEW_CONTIG} edlib/src/edlib.cpp)
target_include_directories(create_new_contigs PRIVATE edlib/include)
target_compile_options (create_new_contigs PRIVATE -g)
target_compile_options (create_new_contigs PRIVATE -lm)
target_compile_options (create_new_contigs PRIVATE -fopenmp)
target_compile_options (create_new_contigs PRIVATE -std=c++17)
target_compile_options (create_new_contigs PRIVATE -O3)
target_compile_options (create_new_contigs PRIVATE -march=x86-64)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    # target_link_libraries(Hairsplitter PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(create_new_contigs PRIVATE OpenMP::OpenMP_CXX)
endif()

#for OpenMP: https://answers.ros.org/question/64231/error-in-rosmake-rgbdslam_freiburg-undefined-reference-to-gomp/

