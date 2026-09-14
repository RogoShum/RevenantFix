cmake_minimum_required(VERSION 3.21)

project(RevenantFix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(RevenantFix SHARED
  src/core.cpp
  src/log.cpp
  src/buddy_fix.cpp
  src/reward_fix.cpp
  src/reconnect_fix.cpp
  src/network_tools.cpp
  src/dllmain.cpp
)

set_target_properties(RevenantFix PROPERTIES
  UNITY_BUILD ON
  UNITY_BUILD_MODE BATCH
  UNITY_BUILD_BATCH_SIZE 16
)

target_compile_definitions(RevenantFix PRIVATE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
)

if(MSVC)
  target_compile_options(RevenantFix PRIVATE /W4 /EHsc /permissive- /Gy /Gw)
  target_link_options(RevenantFix PRIVATE /OPT:REF /OPT:ICF)
endif()
