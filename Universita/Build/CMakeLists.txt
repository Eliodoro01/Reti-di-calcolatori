cmake_minimum_required(VERSION 3.22)
project(Universita C)

set(CMAKE_C_STANDARD 23)

add_executable(Studente
        ../Source/lib.h
        ../Source/Studente.c
        ../Implementation/Framework.c
        ../Implementation/Network.c
        ../Implementation/Studente_Helper.c
)

add_executable(Segreteria
        ../Source/lib.h
        ../Source/Segreteria.c
        ../Implementation/Framework.c
        ../Implementation/Network.c
        ../Implementation/Segreteria_Helper.c
)

add_executable(Server
        ../Source/lib.h
        ../Source/Server.c
        ../Implementation/Framework.c
        ../Implementation/Network.c
        ../Implementation/Server_Helper.c
)

find_package(SQLite3 REQUIRED)
target_link_libraries(Studente SQLite::SQLite3)
target_link_libraries(Segreteria SQLite::SQLite3)
target_link_libraries(Server SQLite::SQLite3)