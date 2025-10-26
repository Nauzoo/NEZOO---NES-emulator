add_library(nzBUS STATIC nzBUS.c)

target_include_directories(nzBUS PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nzBUS PUBLIC project_headers)