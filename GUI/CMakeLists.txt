list(APPEND gui_files
	"Game.cpp"
	"Game.h"
	"main.cpp"
	"Settings.cfg"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${gui_files})

add_executable(gui WIN32 ${gui_files})
add_dependencies(gui ASSETS)
sf_target_compile_warnings(gui)
sf_target_compile_warnings_as_errors(gui OPTIONAL)

target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(gui PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(gui PUBLIC gemcutter)
