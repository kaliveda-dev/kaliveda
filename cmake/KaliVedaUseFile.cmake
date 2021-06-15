#---include this file in external project to setup environment
#---needed to use KaliVeda libraries

#---set path to header files
include_directories(${KALIVEDA_INCLUDE_DIR})

#---to be able to use modules for building with ROOT 5 or 6
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_LIST_DIR})

#---do we need to enforce C++11 standard in order to be able to compile with KaliVeda?
if(KaliVeda_NEEDS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED yes)
    message(STATUS "KaliVeda: C++11 support is required and has been enabled")
endif(KaliVeda_NEEDS_CXX11)
