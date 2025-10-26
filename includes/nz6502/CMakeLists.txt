add_library(nz6502 STATIC nz6502.c)

target_include_directories(nz6502 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nz6502 PUBLIC 
    nzBUS
    project_headers
)
