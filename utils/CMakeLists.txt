project(utils LANGUAGES CXX)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(recover_sem recover_sem.cpp)
target_link_libraries(recover_sem pthread rt)