cmake_minimum_required(VERSION 3.0.0)
project(BoostTimer VERSION 0.1.0)


find_package(Boost 1.67.0 REQUIRED COMPONENTS
    program_options
    filesystem
    system
    thread
    date_time
    regex
)


add_subdirectory(timer)
add_subdirectory(date_timer)