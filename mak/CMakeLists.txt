cmake_minimum_required(VERSION 3.15)
project(PathEditor VERSION 1.0 DESCRIPTION "Windows Path Environment Editor" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(PathEditor WIN32 
    ../src/PathEditor.cpp
    ../src/PathEditorDlg.cpp
    ../src/PathListCtrl.cpp
    ../src/PathReader.cpp
    ../src/Util.cpp
    ../src/PathEditor.manifest
    ../src/PathEditor.rc
)

target_include_directories(PathEditor PRIVATE ../src)
target_compile_definitions(PathEditor PRIVATE UNICODE _UNICODE PICKFOLDER_FALLBACK)
target_compile_options(PathEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:  /W4  >
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:  -Wall -Wextra -Wno-unknown-pragmas  >
)
target_link_options(PathEditor PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:  -municode  >
    $<$<CXX_COMPILER_ID:GNU>:  -static-libgcc -static-libstdc++  >
)
target_link_libraries(PathEditor PRIVATE comctl32)

install(TARGETS PathEditor)
