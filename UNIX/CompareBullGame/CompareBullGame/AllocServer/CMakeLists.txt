
set(proj_name CompareBullAlloc)

set(src_path
	"../../Common/BullAllocServer/"
)

file(GLOB_RECURSE project_headers ${src_path}/*.h)
file(GLOB_RECURSE project_cpps ${src_path}/*.cpp)
set(all_files ${project_headers} ${project_cpps})
source_group_by_dir(all_files ${src_path})

include_directories(${base_inc_path})
include_directories(${sdk_inc_path}/comm/)
include_directories(${sdk_inc_path}/comm/Protocol/)
include_directories(${third_inc_path})

include_directories(${src_path})
include_directories(${src_path}/admin)
include_directories(${src_path}/process)
include_directories(${src_path}/gameprocess)
include_directories(${src_path}/backconnect)

link_directories(${sdk_lib_path})

add_definitions(-DGAME_TYPE=E_COMPALL_BULL_GAME_ID)

add_executable(${proj_name} ${all_files})

set_target_properties(${proj_name} PROPERTIES FOLDER "CompareBull")
target_link_libraries(${proj_name} ${sdk_comm_libs} -Wl,-Bdynamic pthread)

# set output dir
set(alloc_output_path "${comp_bull_out_path}/AllocServer/bin")

set_target_properties(${proj_name}
	PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${alloc_output_path}
	ARCHIVE_OUTPUT_DIRECTORY ${lib_out_dir}
	LIBRARY_OUTPUT_DIRECTORY ${lib_out_dir}
)