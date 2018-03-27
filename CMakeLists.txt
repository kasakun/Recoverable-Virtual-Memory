cmake_minimum_required(VERSION 3.9)
project(Recovery-Virtual-Memory)


add_executable(Recovery-Virtual-Memory
#                abort.cpp
#                basic.cpp
#                truncate.cpp
                multi-abort.cpp
                rvm.cpp
        )

