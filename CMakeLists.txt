cmake_minimum_required(VERSION 2.8)

add_executable(jsmin_test 
	jsmin.cpp
	jsmin.hpp
	jsmin_test.cpp)

file(COPY after.js before.js DESTINATION ${CMAKE_BINARY_DIR})