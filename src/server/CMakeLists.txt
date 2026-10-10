aux_source_directory(. SRC_LIST)

aux_source_directory(db/ DB_SRC_LIST)
aux_source_directory(model/ MODEL_SRC_LIST)

add_executable(server ${SRC_LIST} ${DB_SRC_LIST} ${MODEL_SRC_LIST})

target_link_libraries(server PRIVATE
        muduo_net muduo_base mysqlclient Threads::Threads)