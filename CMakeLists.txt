cmake_minimum_required(VERSION 3.20)
project(collection-manager LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_POLICY_DEFAULT_CMP0177 NEW)

# Dependencies
include(FetchContent)
macro(fetch name repo tag)
    set(_local "${CMAKE_SOURCE_DIR}/third_party/${name}")
    if(EXISTS "${_local}/CMakeLists.txt" OR EXISTS "${_local}/include" OR EXISTS "${_local}/src")
        message(STATUS "Using local vendor: ${name}")
        FetchContent_Declare(${name} SOURCE_DIR "${_local}")
    else()
        message(STATUS "Fetching from upstream: ${name} @ ${tag}")
        FetchContent_Declare(${name} GIT_REPOSITORY ${repo} GIT_TAG ${tag})
    endif()
    FetchContent_MakeAvailable(${name})
endmacro()

fetch(Crow https://github.com/CrowCpp/Crow.git v1.3.2)
fetch(nlohmann_json https://github.com/nlohmann/json.git v3.12.0)
fetch(libsass https://github.com/sass/libsass.git 3.6.6)
fetch(cpprealm https://github.com/nyaruku/cpprealm.git patched)
find_package(ZLIB REQUIRED)

# libsass
file(GLOB LIBSASS_SOURCES
    "${libsass_SOURCE_DIR}/src/*.cpp"
    "${libsass_SOURCE_DIR}/src/*.c"
)
add_library(sass STATIC ${LIBSASS_SOURCES})
target_include_directories(sass PUBLIC "${libsass_SOURCE_DIR}/include")
target_compile_definitions(sass PRIVATE LIBSASS_VERSION="3.6.6")
set_target_properties(sass PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# Web assets
add_custom_target(copy_web_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/www/static"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/www" "${CMAKE_BINARY_DIR}/www"
    COMMENT "Copying web assets"
)

add_executable(${PROJECT_NAME} src/main.cpp)
add_dependencies(${PROJECT_NAME} copy_web_assets)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${cpprealm_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Crow::Crow
    ZLIB::ZLIB
    cpprealm
    nlohmann_json::nlohmann_json
    sass
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CROW_STATIC_DIRECTORY="www/static/"
    COLLECTION_MANAGER_VERSION="1.0.0"
    COLLECTION_MANAGER_VERSION_SIGNED="100"
    COLLECTION_MANAGER_HOST="127.0.0.1"
    COLLECTION_MANAGER_PORT=11727
)
