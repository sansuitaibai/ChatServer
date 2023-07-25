
#搜索当前目录下所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
#file(GLOB_RECURSE SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

#指定库的路径
link_directories(/usr/lib64/mysql/)
link_directories(/usr/local/lib/)

#根据源文件指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})


#指定连接时依赖时需要的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)