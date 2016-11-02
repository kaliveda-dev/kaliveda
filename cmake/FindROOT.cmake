# - Finds ROOT instalation
# This module sets up ROOT information
# It defines:
# ROOT_FOUND          If the ROOT is found
# ROOT_INCLUDE_DIR    PATH to the include directory
# ROOT_INCLUDE_DIRS   PATH to the include directories (not cached)
# ROOT_LIBRARIES      Most common libraries
# ROOT_<name>_LIBRARY Full path to the library <name>
# ROOT_LIBRARY_DIR    PATH to the library directory
#
# Updated by K. Smith (ksmith37@nd.edu) to properly handle
#  dependencies in ROOT_GENERATE_DICTIONARY

include(FindPackageMessage)
find_package_message(ROOT "Using KaliVeda-provided FindROOT.cmake" "[${CMAKE_CURRENT_SOURCE_DIR}]")

find_program(ROOT_CONFIG_EXECUTABLE root-config
  PATHS $ENV{ROOTSYS}/bin)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE ROOTSYS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE ROOT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_LIBRARY_DIRS ${ROOT_LIBRARY_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --bindir
    OUTPUT_VARIABLE ROOT_BINARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cc
    OUTPUT_VARIABLE ROOT_C_COMPILER
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cxx
    OUTPUT_VARIABLE ROOT_CXX_COMPILER
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(ROOT_LIBRARIES)
foreach(_cpt Core Cint ${ROOT_FIND_COMPONENTS})
  find_library(ROOT_${_cpt}_LIBRARY ${_cpt} HINTS ${ROOT_LIBRARY_DIR})
  if(ROOT_${_cpt}_LIBRARY)
    mark_as_advanced(ROOT_${_cpt}_LIBRARY)
    list(APPEND ROOT_LIBRARIES ${ROOT_${_cpt}_LIBRARY})
    list(REMOVE_ITEM ROOT_FIND_COMPONENTS ${_cpt})
  endif()
endforeach()
list(REMOVE_DUPLICATES ROOT_LIBRARIES)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libs
    OUTPUT_VARIABLE ROOT_CORE_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCHALL [-]l[A-Z][0-9A-Za-z]* corelibs ${ROOT_CORE_LIBS})
set(ROOT_CORE_LIBRARIES)
foreach(lib ${corelibs})
   string(REGEX REPLACE ^[-l] "" llib ${lib})
   list(APPEND ROOT_CORE_LIBRARIES ${ROOT_${llib}_LIBRARY})
endforeach()

#----------------------------------------------------------------------------
# Locate the tools
set(ROOT_ALL_TOOLS genreflex genmap root rootcint)
foreach(_cpt ${ROOT_ALL_TOOLS})
  if(NOT ROOT_${_cpt}_CMD)
    find_program(ROOT_${_cpt}_CMD ${_cpt} HINTS ${ROOT_BINARY_DIR})
    if(ROOT_${_cpt}_CMD)
      mark_as_advanced(ROOT_${_cpt}_CMD)
      list(REMOVE_ITEM ROOT_FIND_COMPONENTS ${_cpt})
    endif()
  endif()
endforeach()

set(ROOT_DEFINITIONS ${ROOT_CXX_FLAGS})
separate_arguments(ROOT_DEFINITIONS)
set(ROOT_EXECUTABLE ${ROOT_root_CMD})

#--set all ROOT flags corresponding to enabled features
execute_process(
  COMMAND ${ROOT_CONFIG_EXECUTABLE} --features
  OUTPUT_VARIABLE _root_options
  OUTPUT_STRIP_TRAILING_WHITESPACE)
  separate_arguments(_root_options)
foreach(_opt ${_root_options})
  set(ROOT_${_opt}_FOUND TRUE)
  set(${_opt} TRUE)
endforeach()
#--unset 'soversion' if enabled - it will cause problems
set(soversion FALSE)

#---set ROOT module include path depending on major version
if(${ROOT_VERSION} VERSION_LESS 6)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/root5)
#	set(ROOT_USE_FILE ${PROJECT_SOURCE_DIR}/cmake/root5/ROOTUseFile.cmake)
else()
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/root6)
#	set(ROOT_USE_FILE ${PROJECT_SOURCE_DIR}/cmake/root6/ROOTUseFile.cmake)
   #---for ROOT6 option 'cxx11' should always be set, but sometimes it is not
   #   in the root-config --features list (???)
   set(ROOT_cxx11_FOUND TRUE)
   set(cxx11 TRUE)
endif()
set(ROOT_USE_FILE ${CMAKE_CURRENT_LIST_DIR}/ROOTUseFile.cmake)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	ROOT
	REQUIRED_VARS ROOTSYS ROOT_CONFIG_EXECUTABLE ROOT_VERSION ROOT_INCLUDE_DIR ROOT_LIBRARIES ROOT_LIBRARY_DIR
	ROOT_BINARY_DIR
	VERSION_VAR ROOT_VERSION
	HANDLE_COMPONENTS
)

mark_as_advanced(ROOT_CONFIG_EXECUTABLE)

#--check we are using same compiler as ROOT for v6 (requires clang)
if(${ROOT_VERSION} VERSION_GREATER 6)
get_filename_component(_compiler_name ${CMAKE_CXX_COMPILER} NAME)
if(NOT ${_compiler_name} STREQUAL ${ROOT_CXX_COMPILER})
   message(WARNING
"
        === WARNING: POTENTIAL COMPILER INCOMPATIBILITY ===
It looks like your default C++ compiler (=${_compiler_name}) is not
the same as that used to compile ROOT (=${ROOT_CXX_COMPILER}).
If so, re-run cmake TWICE with the following additional definitions:
cmake -DCMAKE_C_COMPILER=${ROOT_C_COMPILER} -DCMAKE_CXX_COMPILER=${ROOT_CXX_COMPILER} ...
(the first time cmake will fail; the second should be OK: if not, delete build directory and start again)
"
)
endif()
endif(${ROOT_VERSION} VERSION_GREATER 6)

