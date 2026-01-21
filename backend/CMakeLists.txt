file(GLOB BACKEND_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms/*.cpp"
)

add_executable(railway_navigator ${BACKEND_SOURCES})

target_include_directories(railway_navigator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

option(BUILD_TESTS "Build backend tests" OFF)

if(BUILD_TESTS)
    set(BACKEND_LIB_SOURCES ${BACKEND_SOURCES})
    list(REMOVE_ITEM BACKEND_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

    add_library(backend_lib ${BACKEND_LIB_SOURCES})
    target_include_directories(backend_lib PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    add_executable(test_dfs tests/test_dfs.cpp)
    target_link_libraries(test_dfs PRIVATE backend_lib)

    add_executable(test_dijkstra tests/test_dijkstra.cpp)
    target_link_libraries(test_dijkstra PRIVATE backend_lib)
endif()
