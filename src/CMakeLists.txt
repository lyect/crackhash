cmake_minimum_required(VERSION 3.11)
project(lab1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.84.0)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(manager)
add_subdirectory(worker)

target_include_directories(client_message PRIVATE ./)
target_include_directories(client_message PRIVATE ./tp/)

target_include_directories(common PRIVATE ./)
target_include_directories(common PRIVATE ./tp/)

target_include_directories(manager PRIVATE ./)
target_include_directories(manager PRIVATE ./tp/)
target_include_directories(manager_message PRIVATE ./)
target_include_directories(manager_message PRIVATE ./tp/)

target_include_directories(worker PRIVATE ./)
target_include_directories(worker PRIVATE ./tp/)
target_include_directories(worker_message PRIVATE ./)
target_include_directories(worker_message PRIVATE ./tp/)