cmake_minimum_required (VERSION 3.13.4)
PROJECT (POLARIS
    VERSION 4.13.02
    DESCRIPTION "POLARIS: POLArized RadIation Simulator, Copyright (C) 2018 Stefan Reissl"
    HOMEPAGE_URL "https://portia.astrophysik.uni-kiel.de/polaris/"
    LANGUAGES CXX
    )

SET (CMAKE_CXX_STANDARD 23)
SET (CXX_STANDARD_REQUIRED ON)

#begin Branch specific definitions
set (BUILD_TESTING OFF)
set (TEST_REWRITE ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
list (APPEND CMAKE_PREFIX_PATH
    "${CMAKE_SOURCE_DIR}/../bin/zlib/"
    "${CMAKE_SOURCE_DIR}/../bin/cfitsio/"
    "${CMAKE_SOURCE_DIR}/../bin/CCfits/")
#end

SET (PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR}/../)

OPTION(BUILD_SHARED_LIBS "Build Shared Libraries" ON)

SET (LIB_TYPE SHARED)
#SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MMD -MP -MF -ansi")
IF(NOT BUILD_SHARED_LIBS)
    SET (LIB_TYPE STATIC)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
ENDIF(NOT BUILD_SHARED_LIBS)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

FIND_PACKAGE (CCFITS REQUIRED)
FIND_PACKAGE (OpenMP REQUIRED)

set (RW_FILES 
    file_util/basic_parser.cpp
    file_util/parser/parsed_line.cpp
    file_util/parser/polaris_commands.cpp
    file_util/parser/polaris_parser.cpp
)

SET (SRC_FILES
    CellBasic.cpp CellCylindrical.cpp CellOcTree.cpp CellSpherical.cpp CellVoronoi.cpp
    CommandParser.cpp
    CrossSections.cpp
    Detector.cpp
    DustComponent.cpp DustMixture.cpp
    Faddeeva.cc
    GasMixture.cpp GasSpecies.cpp
    GridBasic.cpp GridCylindrical.cpp GridOcTree.cpp GridSpherical.cpp GridVoronoi.cpp
    MathFunctions.cpp MathInterp.cpp MathProbList.cpp MathSpline.cpp
    Matrix2D.cpp
    OPIATE.cpp
    Parameters.cpp
    Photon.cpp
    Pipeline.cpp
    RadiativeTransfer.cpp
    RandomGenerator.cpp
    RaytracingBasic.cpp RaytracingCartesian.cpp RaytracingHealPix.cpp RaytracingPolar.cpp RaytracingSlice.cpp
    SourceAGN.cpp SourceBackground.cpp SourceBasic.cpp SourceDust.cpp SourceGas.cpp SourceISRF.cpp SourceLaser.cpp SourceStar.cpp SourceStarField.cpp
    Stokes.cpp
    Synchrotron.cpp SyncParameters.cpp
    Vector3D.cpp
)

ADD_EXECUTABLE (polaris main.cpp ${SRC_FILES} ${RW_FILES})
TARGET_LINK_LIBRARIES (polaris PRIVATE OpenMP::OpenMP_CXX ${CCFITS_LIBRARY} ${CFITSIO_LIBRARY})

# CCFITS_INCLUDE_DIR must be set, if we only have relative paths
TARGET_INCLUDE_DIRECTORIES (polaris PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CFITSIO_INCLUDE_DIR} ${CCFITS_INCLUDE_DIR})
INSTALL (TARGETS polaris)

if (TEST_REWRITE)
    add_subdirectory (file_util/test/)
endif()

if(BUILD_TESTING)
    INCLUDE (CTest)
    ADD_EXECUTABLE (tests tests.cpp DustComponent_t.cpp MathFunctions_t.cpp RandomGenerator_t.cpp ${SRC_FILES})
    TARGET_LINK_LIBRARIES (tests PRIVATE OpenMP::OpenMP_CXX ${CCFITS_LIBRARY} ${CFITSIO_LIBRARY})
    TARGET_INCLUDE_DIRECTORIES (tests PRIVATE ../ext ${CMAKE_CURRENT_LIST_DIR} ${CFITSIO_INCLUDE_DIR})
    # allow user to run tests with `make test` or `ctest`
    INCLUDE(Catch.cmake)
    catch_discover_tests(tests)
endif()
