cmake_minimum_required(VERSION 3.0.0)
project(BoostDateTime VERSION 0.1.0)

add_executable(date date.cpp)

target_link_libraries(date ${Boost_LIBRARIES})