cmake_minimum_required(VERSION 3.9)

set(USER_HOME $ENV{HOME})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../dll)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

project(
        Project.Nar.Program
        VERSION 100
        DESCRIPTION "Nar.Program"
        LANGUAGES C
)

add_library(Nar.Program SHARED
        include/Nar.Program.h
        _package.c
        _package.h
        cmd.c
        program.c
        sub.c
        task.c
        lifecycle.c
)

target_include_directories(Nar.Program PRIVATE ~/.nar/include)
set_target_properties(Nar.Program PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(Nar.Program)

file(COPY include DESTINATION ${USER_HOME}/.nar)