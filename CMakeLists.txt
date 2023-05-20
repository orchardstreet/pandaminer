cmake_minimum_required(VERSION 3.0)

add_compile_options(-O2)

project(pandaminer LANGUAGES C)

add_executable(pandaminer miner.c JSON.c readline_custom.c user_options.c http.c utils.c)

install(TARGETS pandaminer RUNTIME DESTINATION bin)
