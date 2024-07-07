cmake_minimum_required(VERSION 3.20)

project(monolog
	VERSION 1.0.0
	DESCRIPTION "Logging library"
	HOMEPAGE_URL https://github.com/erengy/monolog
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(monolog INTERFACE)

target_include_directories(monolog INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(monolog INTERFACE src/monolog.cpp)
