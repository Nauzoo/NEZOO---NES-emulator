add_library(UI_template STATIC UI_template.hpp)
add_library(registers_editor.hpp STATIC registers_editor.hpp)

target_include_directories(UI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# target_link_libraries(NESoo_GUI PUBLIC 
#    
# )
