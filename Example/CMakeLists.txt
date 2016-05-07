cmake_minimum_required(VERSION 2.8)

project(BaluScriptExample)

file(GLOB_RECURSE BaluScriptExample_SRC
    "Source/*.h"
    "Source/*.cpp"
)

INCLUDE_DIRECTORIES("Include")


add_executable(BaluScriptExample
	${BaluScriptExample_SRC}
)

target_link_libraries ( BaluScriptExample
	BaluScript
) 
