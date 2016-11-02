#---Define the Standard macros for ROOT-----------------------------------------------------------
include(RootNewMacros)
if(NOT DEFINED ROOT_ARCHITECTURE)
  include(CheckCompiler)
endif()

#---Set Link and include directories--------------------------------------------------------------
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
include_directories(${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_DEFINITIONS})

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

link_directories(${ROOT_LIBRARY_DIRS})
