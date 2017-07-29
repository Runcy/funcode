
project(CompareBullGame)

set(comp_game_src_path
	"${CMAKE_HOME_DIRECTORY}/CompareBullGame/"
)

set(comp_bull_out_path
	"${CMAKE_HOME_DIRECTORY}/Output/CompareBullGame/"
)

option(BUILD_COMPBULL_ALLOC "build compare bull alloc server" TRUE)
if (BUILD_COMPBULL_ALLOC)
	# add alloc server
	set(alloc_src_path
		"${comp_game_src_path}/AllocServer"
	)
	add_subdirectory(${alloc_src_path})
endif()

option(BUILD_COMPBULL_GAME "build compare bull game server" TRUE)
if (BUILD_COMPBULL_GAME)
	# add bull server
	set(comp_src_path
		"${comp_game_src_path}/GameServer/trunk/src"
	)
	add_subdirectory(${comp_src_path})
endif()

