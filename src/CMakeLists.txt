set(TARGET polatory)

file(GLOB_RECURSE SOURCES
    "*.cpp"
    "${PROJECT_SOURCE_DIR}/third_party/ScalFMM/src/*.cpp"
)
add_library(${TARGET} STATIC ${SOURCES})

if(UNIX)
    target_compile_options(${TARGET} PUBLIC -Wall -Wextra -Werror)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET} PUBLIC -O2)
    endif()
elseif(MSVC)
    # C4702(unreachable code) is disabled as it is reported by link.exe in third-party code.
    target_compile_options(${TARGET} PUBLIC /W4 /WX /wd4702 /experimental:external /external:W0)
    target_compile_definitions(${TARGET} PUBLIC
        -D_ENABLE_EXTENDED_ALIGNED_STORAGE
    )
endif()

target_compile_definitions(${TARGET} PUBLIC
    -DEIGEN_DONT_PARALLELIZE
    -DEIGEN_MPL2_ONLY
    -DEIGEN_USE_MKL_ALL
    -DGLOG_NO_ABBREVIATED_SEVERITIES
    -DNOMINMAX
)

if(USE_PREBUILT_BOOST)
    target_compile_definitions(${TARGET} PUBLIC
        -DBOOST_ALL_DYN_LINK
        -DBOOST_ALL_NO_LIB
    )
endif()

set(THIRD_PARTY_PUBLIC_INCLUDE_DIRECTORIES
    ${Boost_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${MKL_INCLUDE_DIR}
    ${VCPKG_INCLUDE_DIR}
)
set(THIRD_PARTY_PRIVATE_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/third_party/ScalFMM/include
)

target_include_directories(${TARGET} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(${TARGET} SYSTEM PUBLIC
    ${THIRD_PARTY_PUBLIC_INCLUDE_DIRECTORIES}
)
target_include_directories(${TARGET} SYSTEM PRIVATE
    ${THIRD_PARTY_PRIVATE_INCLUDE_DIRECTORIES}
)

if(MSVC)
    foreach(DIR ${THIRD_PARTY_PUBLIC_INCLUDE_DIRECTORIES})
        target_compile_options(${TARGET} PUBLIC
            "SHELL:/external:I \"${DIR}\""
        )
    endforeach()
    foreach(DIR ${THIRD_PARTY_PRIVATE_INCLUDE_DIRECTORIES})
        target_compile_options(${TARGET} PRIVATE
            "SHELL:/external:I \"${DIR}\""
        )
    endforeach()
endif()

target_link_libraries(${TARGET} PUBLIC
    OpenMP::OpenMP_CXX
)

target_link_libraries(${TARGET} INTERFACE
    ${Boost_LIBRARIES}
    ceres
    double-conversion::double-conversion
    glog::glog
)

if(UNIX)
    target_link_libraries(${TARGET} INTERFACE
        flann::flann_cpp_s
        "-Wl,--start-group"
        ${MKL_LIB_DIR}/libmkl_intel_lp64.a ${MKL_LIB_DIR}/libmkl_sequential.a ${MKL_LIB_DIR}/libmkl_core.a pthread m dl
        "-Wl,--end-group"
    )
elseif(MSVC)
    target_link_libraries(${TARGET} INTERFACE
        flann::flann_cpp
        ${MKL_LIB_DIR}/mkl_intel_lp64_dll.lib ${MKL_LIB_DIR}/mkl_sequential_dll.lib ${MKL_LIB_DIR}/mkl_core_dll.lib
    )
endif()

if(MSVC)
    polatory_get_prebuilt_boost_dlls(BOOST_DLLS)
    list(APPEND POLATORY_DLLS ${BOOST_DLLS})
    set(POLATORY_DLLS ${POLATORY_DLLS} PARENT_SCOPE)
endif()

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/polatory DESTINATION include)
