cmake_minimum_required(VERSION 3.0)

project(pandaminer LANGUAGES C)

add_executable(pandaminer main.c)

install(TARGETS pandaminer RUNTIME DESTINATION bin)
