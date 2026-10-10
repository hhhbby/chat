aux_source_directory(. SRC_LIST)

aux_source_directory(db/ DBSRC_LIST)

add_executable(server ${SRC_LIST} ${DBSRC_LIST})

target_link_libraries(server PRIVATE
        muduo_net muduo_base Threads::Threads)