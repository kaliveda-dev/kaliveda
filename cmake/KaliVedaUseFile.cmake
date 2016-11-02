#---include this file in external project to setup environment
#---needed to use KaliVeda libraries

#---set path to header files
include_directories(${KALIVEDA_INCLUDE_DIR})

#---to be able to use modules for building with ROOT 5 or 6
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_LIST_DIR})

