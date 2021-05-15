list(APPEND physics2d_files
	"Game.cpp"
	"Game.h"
	"main.cpp"
	"Settings.cfg"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${physics2d_files})

add_executable(physics2d WIN32 ${physics2d_files})
add_dependencies(physics2d ASSETS)
sf_target_compile_warnings(physics2d)
sf_target_compile_warnings_as_errors(physics2d OPTIONAL)

target_include_directories(physics2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(physics2d PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(physics2d PUBLIC gemcutter)
