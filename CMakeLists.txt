set(CMAKE_FOLDER "${CMAKE_FOLDER}/samples")

gem_set_up_workspace("${CMAKE_CURRENT_LIST_DIR}/workspace"
	PACK_TARGET ASSETS
)

add_subdirectory(Components)
add_subdirectory(GodRays)
add_subdirectory(GUI)
add_subdirectory(Lighting)
add_subdirectory(Particles)
add_subdirectory(Shadows)
add_subdirectory(SimpleScene)
add_subdirectory(Sounds)
add_subdirectory(Sprites)
add_subdirectory(Text)
