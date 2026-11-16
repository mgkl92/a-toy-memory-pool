cmake_minimum_required(VERSION 3.10)

project(MemoryPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_library(${PROJECT_NAME} STATIC
  src/ThreadCache.cpp
  src/CentralCache.cpp
  src/PageCache.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
