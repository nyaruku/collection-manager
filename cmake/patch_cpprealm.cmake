# Patch cpprealm sources for GCC 16 compatibility.
# Called from FetchContent PATCH_COMMAND — runs in the cpprealm source directory.

# 1. Remove -pedantic -Werror from Realm's own build flags
file(READ "CMakeLists.txt" _content)
string(REPLACE "-Wall -Wextra -pedantic -Werror" "-Wall -Wextra -Wno-changes-meaning" _content "${_content}")
file(WRITE "CMakeLists.txt" "${_content}")

# 2. Add #include <cstdint> to every bridge header that uses uint*_t types
set(_headers
    include/cpprealm/internal/bridge/utils.hpp
    include/cpprealm/internal/bridge/realm.hpp
    include/cpprealm/internal/bridge/uuid.hpp
    include/cpprealm/internal/bridge/binary.hpp
    include/cpprealm/internal/bridge/mixed.hpp
    include/cpprealm/internal/bridge/object_id.hpp
)

foreach(_h IN LISTS _headers)
    if(EXISTS "${_h}")
        file(READ "${_h}" _src)
        if(NOT _src MATCHES "#include <cstdint>")
            file(WRITE "${_h}" "#include <cstdint>\n${_src}")
            message(STATUS "patch_cpprealm: added <cstdint> to ${_h}")
        endif()
    endif()
endforeach()

