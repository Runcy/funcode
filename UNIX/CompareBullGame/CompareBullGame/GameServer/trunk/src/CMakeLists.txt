#遍历当前目录及其子目录下的所有文件
file(GLOB_RECURSE project_headers "*.h")
file(GLOB_RECURSE project_cpps "*.cpp")

set(proj_name CompareBullGame)
set(src_path "./")
set(all_files ${project_headers} ${project_cpps})

#进行源代码分组
source_group_by_dir(all_files ${src_path})

#part-include
#common
include_directories(${base_inc_path})
include_directories(${comm_inc_path})
include_directories(${third_inc_path})
#self
include_directories(${comp_src_path})
include_directories(${comp_src_path}/process)
include_directories(${comp_src_path}/admin)
include_directories(${comp_src_path}/model)
#part-link
link_directories(${sdk_lib_path})
#part-product
add_executable(${proj_name} ${all_files})

set_target_properties(${proj_name} PROPERTIES FOLDER "CompareBull")
target_link_libraries(${proj_name} ${sdk_comm_libs} -Wl,-Bdynamic pthread)

#set output dir
set(game_output_dir "${comp_bull_out_path}/GameServer/compare/bin")
set_target_properties(${proj_name}
	PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${game_output_dir}
	ARCHIVE_OUTPUT_DIRECTORY ${lib_out_dir}
	LIBRARY_OUTPUT_DIRECTORY ${lib_out_dir}
)