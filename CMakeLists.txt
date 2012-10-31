cmake_minimum_required(VERSION 2.8)
set (PROJECT BaluScript) 
ENABLE_TESTING()
#SET(CMAKE_BUILD_TYPE "Release")

project (${PROJECT}) 
set (HEADERS Include/BaluScript.h)

include_directories (include) 
include_directories (source) 

set (SOURCES
	./Examples/Test/test.cpp
	./Source/class.cpp
	./Source/common.cpp
	./Source/lexer.cpp
	./Source/notOptimizedProgram.cpp
	./Source/semanticAnalyzer.cpp
	./Source/syntaxAnalyzer.cpp
	./Source/virtualMachine.cpp) 
	
add_executable (${PROJECT} ${HEADERS} ${SOURCES})
add_test(${PROJECT} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT} --log_level=all --report_format=XML --report_level=detailed)
 