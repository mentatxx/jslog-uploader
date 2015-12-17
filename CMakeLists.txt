cmake_minimum_required (VERSION 2.8)
project (Jslog-uploader)
include_directories("include/")

# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Search OpenSSL
find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL REQUIRED openssl)

if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

#Search CURL
find_package(CURL REQUIRED)
if( CURL_FOUND )
    include_directories(${CURL_INCLUDE_DIRS})
    message(STATUS "Using CURL ${CURL_VERSION}")
else()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

#Search YAML
find_package(YamlCpp REQUIRED)
if( YAMLCPP_FOUND )
    include_directories(${YAMLCPP_INCLUDE_DIRS})
    message(STATUS "Using YamlCpp ${YAMLCPP_VERSION}")
else()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

#Search json-c
pkg_search_module(JSONC REQUIRED json-c)

if( JSONC_FOUND )
    include_directories(${JSONC_INCLUDE_DIRS})
    message(STATUS "Using Json-C ${JSONC_VERSION}")
else()
    # Error; with REQUIRED, pkg_search_module() will throw an error by it's own
endif()

add_executable(Jslog-uploader src/main.cpp src/CLog.cpp src/CApplication.cpp)

target_link_libraries(Jslog-uploader ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} ${YAMLCPP_LIBRARY} ${JSONC_LIBRARIES})
