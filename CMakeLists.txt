cmake_minimum_required(VERSION 3.20)
project(collection-manager LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_BUILD_PARALLEL_LEVEL 16)

include(FetchContent)

macro(fetch name repo tag)
    FetchContent_Declare(${name} GIT_REPOSITORY ${repo} GIT_TAG ${tag})
    FetchContent_MakeAvailable(${name})
endmacro()

fetch(Crow https://github.com/CrowCpp/Crow.git v1.3.2)
fetch(nlohmann_json https://github.com/nlohmann/json.git v3.12.0)

find_package(ZLIB REQUIRED)

set(CMAKE_POLICY_DEFAULT_CMP0177 NEW)
FetchContent_Declare(
    cpprealm
    GIT_REPOSITORY https://github.com/realm/realm-cpp.git
    GIT_TAG local-only
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_cpprealm.cmake
)
FetchContent_MakeAvailable(cpprealm)

add_custom_target(copy_web_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/www/static"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/www" "${CMAKE_BINARY_DIR}/www"
    COMMENT "Copying web assets"
)

add_executable(${PROJECT_NAME} src/main.cpp)
add_dependencies(${PROJECT_NAME} copy_web_assets)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Crow::Crow
    ZLIB::ZLIB
    cpprealm
    nlohmann_json::nlohmann_json
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${cpprealm_SOURCE_DIR}/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CROW_STATIC_DIRECTORY="www/static/"
    COLLECTION_MANAGER_VERSION="0.1.0"
    COLLECTION_MANAGER_VERSION_SIGNED="10"
    COLLECTION_MANAGER_HOST="127.0.0.1"
    COLLECTION_MANAGER_PORT=11727
)
