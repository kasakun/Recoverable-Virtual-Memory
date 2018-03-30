cmake_minimum_required(VERSION 3.9)
project(Recovery-Virtual-Memory)


add_executable(Recovery-Virtual-Memory
#                abort.cpp
#                basic.cpp
#                truncate.cpp
#                multi.cpp
#                multi-abort.cpp
#                abortincommit.cpp
#                multi_trans_basic.cpp
                multi_trans_crash_basic.cpp
                rvm.cpp
        )

