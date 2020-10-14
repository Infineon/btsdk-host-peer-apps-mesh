cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../../distribution)

add_library(lib_wicedmesh STATIC IMPORTED)
set_target_properties(lib_wicedmesh PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/wicedmesh/prebuild/${ANDROID_ABI}/libwicedmesh.a)

# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(hello_lib SHARED
            hello_lib.c)

target_include_directories(hello_lib PRIVATE
                           ${distribution_DIR}/wicedmesh/include)

target_link_libraries(hello_lib
                      android
                      lib_wicedmesh
                      log)
