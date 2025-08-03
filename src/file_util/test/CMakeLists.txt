cmake_minimum_required (VERSION 3.13.4)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/heads/main.zip
)

FetchContent_MakeAvailable(benchmark)

set (RW_FILES
    ../basic_parser.cpp
    ../parser/polaris_commands.cpp
    ../parser/parsed_line.cpp
    ../parser/polaris_parser.cpp
)

set (BASE_FILES
    ../../Parameters.cpp
    ../../CommandParser.cpp
    ../../Vector3D.cpp
    ../../Matrix2D.cpp
    ../../MathFunctions.cpp
    ../../Faddeeva.cc
    ../../MathProbList.cpp
    ../../Stokes.cpp
)

# Test Command Parser

add_executable (test_polaris_parser
    test_command_parser.cpp

    ${BASE_FILES}
    ${RW_FILES}
)

target_compile_definitions (test_polaris_parser PRIVATE TEST_REWRITE)

target_link_libraries (test_polaris_parser
    OpenMP::OpenMP_CXX
    GTest::gmock
    GTest::gtest_main
)

# Benchmark Command Parser

add_executable (bm_polaris_parser
    ../benchmark/bm_command_parser.cpp

    ${BASE_FILES}
    ${RW_FILES}
)

target_link_libraries (bm_polaris_parser
    OpenMP::OpenMP_CXX
    benchmark::benchmark
)

# Test Dust Component Loader

set (DUST_FILES
    ../../DustComponent_1.cpp ../../DustComponent.cpp
    ../../CellBasic.cpp ../../CellCylindrical.cpp ../../CellSpherical.cpp ../../CellOcTree.cpp ../../CellVoronoi.cpp
    ../../GridBasic.cpp ../../GridCylindrical.cpp ../../GridSpherical.cpp ../../GridOcTree.cpp ../../GridVoronoi.cpp
    ../../CrossSections.cpp
    ../../MathInterp.cpp ../../MathSpline.cpp ../../MathProbList.cpp
    ../../Photon.cpp
    ../../RandomGenerator.cpp
)

add_executable (test_dust_component
    dust_component_parameter_parser.cpp

    ${BASE_FILES}
    ${DUST_FILES}
    ${RW_FILES}
)

target_compile_definitions (test_dust_component PRIVATE TEST_REWRITE)

target_link_libraries (test_dust_component
    ${CCFITS_LIBRARY}
    ${CFITSIO_LIBRARY}
    OpenMP::OpenMP_CXX
    GTest::gmock
    GTest::gtest_main
)

target_include_directories (test_dust_component PRIVATE ${CCFITS_INCLUDE_DIR} ${CFITSIO_INCLUDE_DIR})

# Benchmark Dust Component Loader

add_executable (bm_dust_component
    ../benchmark/dust_component_parameter_parser.cpp
    ${DUST_FILES}
    ${BASE_FILES}
    ${RW_FILES}
)

target_link_libraries (bm_dust_component
    ${CCFITS_LIBRARY}
    ${CFITSIO_LIBRARY}
    OpenMP::OpenMP_CXX
    benchmark::benchmark
)

target_include_directories (bm_dust_component PRIVATE ${CCFITS_INCLUDE_DIR} ${CFITSIO_INCLUDE_DIR})
