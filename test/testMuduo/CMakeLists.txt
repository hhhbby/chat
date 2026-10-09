add_executable(server
    muduo_server.cpp
)

target_compile_features(server PRIVATE cxx_std_17)

target_link_libraries(server PRIVATE
    muduo_net
    muduo_base
)