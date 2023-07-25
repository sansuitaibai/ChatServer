
#搜索当前目录下所有源文件
aux_source_directory(. SRC_LIST)
#file(GLOB_RECURSE SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


#根据源文件指定生成可执行文件
add_executable(ChatClient ${SRC_LIST} )


#指定连接时依赖时需要的库文件
target_link_libraries(ChatClient pthread)