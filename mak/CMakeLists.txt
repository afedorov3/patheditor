cmake_minimum_required(VERSION 3.15)
project(PathEditor VERSION 1.0 DESCRIPTION "Windows Path Environment Editor" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(PathEditor WIN32 
    ../src/DlgCtrl.hpp
    ../src/PathEditor.cpp
    ../src/PathEditor.manifest
    ../src/PathEditor.rc
    ../src/PathEditorDlg.cpp
    ../src/PathListCtrl.cpp
    ../src/PathReader.cpp
    ../src/Util.cpp
)

target_include_directories(PathEditor PRIVATE ../src)
target_compile_definitions(PathEditor PRIVATE UNICODE _UNICODE PICKFOLDER_FALLBACK)
target_compile_options(PathEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:  /W4  >
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:  -Wall -Wextra  >
)
