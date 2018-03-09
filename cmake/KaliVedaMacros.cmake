include(CMakeParseArguments)

set(KV_HTML_CONVERTER html_convert)

#---------------------------------------------------------------------------------------------------
#---KALIVEDA_SET_INCLUDE_DIRS(library MODULES mod1 mod2 ...)
#---------------------------------------------------------------------------------------------------
function(KALIVEDA_SET_INCLUDE_DIRS lib)

	CMAKE_PARSE_ARGUMENTS(ARG "" "" "MODULES" ${ARGN})
    
   foreach(mod ${ARG_MODULES})
		include_directories(${CMAKE_SOURCE_DIR}/${lib}/${mod})
   endforeach()

endfunction()

#---------------------------------------------------------------------------------------------------
#---CHANGE_LIST_TO_STRING(mystring list1 list2 list3)
#---------------------------------------------------------------------------------------------------
function(CHANGE_LIST_TO_STRING mystring)

	CMAKE_PARSE_ARGUMENTS(ARG "" "" "" ${ARGN})
	set(mylist ${ARG_UNPARSED_ARGUMENTS})
	set(stringy)
   foreach(l ${mylist})
		set(stringy "${stringy}${l} ")
   endforeach()
	set(${mystring} "${stringy}" PARENT_SCOPE)

endfunction()

#---------------------------------------------------------------------------------------------------
#---KALIVEDA_SET_MODULE_DEPS(<variable> library MODULES mod1 mod2 ...)
#---------------------------------------------------------------------------------------------------
function(KALIVEDA_SET_MODULE_DEPS variable)

	CMAKE_PARSE_ARGUMENTS(ARG "" "" "MODULES" ${ARGN})
    
	set(lib ${ARG_UNPARSED_ARGUMENTS})
	set(modlist)
   foreach(mod ${ARG_MODULES})
		set(modlist ${modlist} ${lib}${mod})
   endforeach()
	set(${variable} ${modlist} PARENT_SCOPE)

endfunction()

#---------------------------------------------------------------------------------------------------
#---BUILD_KALIVEDA_EXAMPLES_DIRECTORY()
#
# Compile every source file (*.cpp *.cxx *.C) in the current project's examples/ directory
# They can use classes from any module in the current project (they are all built
# before the examples), plus those from any project built before this one
#
function(BUILD_KALIVEDA_EXAMPLES_DIRECTORY dir)

    message(STATUS "   ...${KVSUBPROJECT} examples")

    #---get list of sources
    file(GLOB source_list ${dir}/*.[cC]*)

    #---set include directories
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/examples)
    #---get list of projects so far configured
    get_property(KALIVEDA_SUBPROJ_LIST GLOBAL PROPERTY KALIVEDA_SUBPROJ_LIST)
    #---set paths to find includes in all projects
    foreach(proj ${KALIVEDA_SUBPROJ_LIST})
        get_property(modlist GLOBAL PROPERTY ${proj}_MOD_LIST)
        KALIVEDA_SET_INCLUDE_DIRS(${proj} MODULES ${modlist})
    endforeach()
    #---get list of all libraries
    get_property(KALIVEDA_LIB_LIST GLOBAL PROPERTY KALIVEDA_LIB_LIST)

    foreach(source ${source_list})

        #---check for corresponding header file
        get_filename_component(sourcename ${source} NAME_WE)
        if(EXISTS ${dir}/${sourcename}.h)

            #---we have toto.cpp & toto.h => it is a class
            ADD_KALIVEDA_EXAMPLE_CLASS(${sourcename} SRC_FILE ${source} HDR_FILE ${dir}/${sourcename}.h)

        else()

            #---it is a simple function
            ADD_KALIVEDA_EXAMPLE_FUNCTION(${source})

        endif()
    endforeach()

endfunction()

#---------------------------------------------------------------------------------------------------
#---ADD_KALIVEDA_EXAMPLE_CLASS(classname)
#
# Compile class 'classname' in current directory
#
function(ADD_KALIVEDA_EXAMPLE_CLASS classname)

    CMAKE_PARSE_ARGUMENTS(ARG "" "SRC_FILE;HDR_FILE" "" ${ARGN})

    set_property(GLOBAL APPEND PROPERTY KALIVEDA_EXCLASS ${classname})
    set_property(GLOBAL APPEND PROPERTY ${KVSUBPROJECT}_EXCLASS ${classname})
    set_property(GLOBAL PROPERTY ${KVSUBPROJECT}_EXCLASS_${classname}_SRC ${ARG_SRC_FILE})

    message(STATUS "         ${classname} - [example class]")

    #---write LinkDef.h file
    set(linkdef ${CMAKE_CURRENT_BINARY_DIR}/examples/LinkDef.h)
    file(WRITE ${linkdef} "#ifdef __CINT__\n")
    file(APPEND ${linkdef} "#pragma link off all globals;\n")
    file(APPEND ${linkdef} "#pragma link off all classes;\n")
    file(APPEND ${linkdef} "#pragma link off all functions;\n")
    file(APPEND ${linkdef} "#pragma link C++ nestedclasses;\n")
    file(APPEND ${linkdef} "#pragma link C++ nestedtypedefs;\n")
    file(APPEND ${linkdef} "#pragma link C++ class ${classname}+;\n")
    file(APPEND ${linkdef} "#endif\n")

    #---generate dictionary etc.
    set(dictionary "G__${classname}")
    #---Get the list of include directories------------------
    get_directory_property(incdirs INCLUDE_DIRECTORIES)
    set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/examples)
    foreach( d ${incdirs})
        set(includedirs ${includedirs} -I${d})
    endforeach()
    list(REMOVE_DUPLICATES includedirs)
    #---Get the list of definitions---------------------------
    get_directory_property(defs COMPILE_DEFINITIONS)
    foreach( d ${defs})
        if((NOT d MATCHES "=") AND (NOT d MATCHES "^[$]<.*>$")) # avoid generator expressions
            set(definitions ${definitions} -D${d})
        endif()
    endforeach()
    if(${ROOT_VERSION} VERSION_LESS 6)
        #---call rootcint------------------------------------------
        add_custom_command(OUTPUT ${dictionary}.cxx ${dictionary}.h
                     COMMAND ${rootcint_cmd} -cint -f  ${dictionary}.cxx
                                          -c -p ${definitions} ${includedirs} ${ARG_HDR_FILE}
                     DEPENDS ${ARG_HDR_FILE} ${ROOTCINTDEP})
    else()
        #---call rootcling------------------------------------------
        add_custom_command(OUTPUT ${dictionary}.cxx
                     COMMAND rootcling -f  ${dictionary}.cxx ${definitions} ${includedirs} ${ARG_HDR_FILE}
                     DEPENDS ${ARG_HDR_FILE} ${ROOTCINTDEP})
    endif()

    #---build library
    add_library(${KVSUBPROJECT}${classname} SHARED ${ARG_SRC_FILE} ${dictionary}.cxx)
    target_link_libraries(${KVSUBPROJECT}${classname} ${KALIVEDA_LIB_LIST})
    
endfunction()

#---------------------------------------------------------------------------------------------------
#---ADD_KALIVEDA_EXAMPLE_FUNCTION(source)
#
# Compile source file 'source' in current directory
#
function(ADD_KALIVEDA_EXAMPLE_FUNCTION source)

    #--get filename without extension or path
    get_filename_component(sourcename ${source} NAME_WE)
    get_filename_component(sourcefile ${source} NAME)
    
    message(STATUS "         ${sourcefile} - [example]")
    set_property(GLOBAL APPEND PROPERTY KALIVEDA_EXFUNCS ${sourcename})
    set_property(GLOBAL APPEND PROPERTY ${KVSUBPROJECT}_EXFUNCS ${sourcename})
    set_property(GLOBAL PROPERTY ${KVSUBPROJECT}_EXFUNC_${sourcename}_FILE ${sourcefile})
    set_property(GLOBAL PROPERTY ${KVSUBPROJECT}_EXFUNC_${sourcename}_SRC ${source})

    add_library(${KVSUBPROJECT}${sourcename} SHARED ${source})
    target_link_libraries(${KVSUBPROJECT}${sourcename} ${KALIVEDA_LIB_LIST} ${ROOT_LIBRARIES})

endfunction()

#---------------------------------------------------------------------------------------------------
#---BUILD_KALIVEDA_MODULE(kvmod PARENT kvtopdir [KVMOD_DEPENDS kvmod1 kvmod2...] [EXTRA_LIBS lib1 lib2...] 
#             [DICT_EXCLUDE toto.h titi.h ...] [LIB_EXCLUDE Class1 Class2...])
#
#---Build the current module, i.e. a subdirectory in KVMultiDet/, KVIndra/, etc.
#   PARENT = name of top-level directory i.e. KVMultiDet
#   KVMOD_DEPENDS = list of modules in same top-level directory that this module depends on
#                   i.e. they must & will be built first
#   EXTRA_LIBS = extra libraries/targets this module depends on
#  To exclude some headers from dictionary generation: DICT_EXCLUDE toto.h titi.h ...
#  To exclude some classes from shared library: LIB_EXCLUDE Class1 Class2 ...
#
#  For each module we fill the following GLOBAL properties:
#        ${PARENT}_MOD_LIST : list of all modules in ${PARENT}
#        ${PARENT}_LIB_LIST : list of all library targets in ${PARENT}
#        KALIVEDA_LIB_LIST  : global list of all library targets
#---------------------------------------------------------------------------------------------------
include(GenerateRootLibrary)
function(BUILD_KALIVEDA_MODULE kvmod)

  message(STATUS "   ...module ${kvmod}")

  CMAKE_PARSE_ARGUMENTS(ARG "" "PARENT" "KVMOD_DEPENDS;DICT_EXCLUDE;LIB_EXCLUDE;EXTRA_LIBS" ${ARGN})

  set(libName ${ARG_PARENT}${kvmod})
  
  set_property(GLOBAL APPEND PROPERTY ${ARG_PARENT}_MOD_LIST ${kvmod})

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  KALIVEDA_SET_INCLUDE_DIRS(${ARG_PARENT} MODULES ${ARG_KVMOD_DEPENDS})
  KALIVEDA_SET_MODULE_DEPS(kvdeps ${ARG_PARENT} MODULES ${ARG_KVMOD_DEPENDS})

  #---generate library & rootmap
  GENERATE_ROOT_LIBRARY(${libName} LIB_EXCLUDE ${ARG_LIB_EXCLUDE} DICT_EXCLUDE ${ARG_DICT_EXCLUDE}
						DEPENDENCIES ${ROOT_CORE_LIBRARIES} ${kvdeps} ${ARG_EXTRA_LIBS})
  
  set_property(GLOBAL APPEND PROPERTY ${ARG_PARENT}_LIB_LIST ${libName})
  set_property(GLOBAL APPEND PROPERTY KALIVEDA_LIB_LIST ${libName})

endfunction()


#---------------------------------------------------------------------------------------------------
#---BUILD_KALIVEDA_SUBPROJECT([DATASETS ds1 ds2 ...])
#                             [IGNORE_DIRS dir1 dir2 ...])
#
#---CMakeLists.txt for a KaliVeda subproject i.e. KVMultiDet, KVIndra, ...
#---DATASETS = list of dataset subdirectories
#---IGNORE_DIRS = subdirectories to ignore
#
#---------------------------------------------------------------------------------------------------
function(BUILD_KALIVEDA_SUBPROJECT)

  CMAKE_PARSE_ARGUMENTS(ARG "" "" "DATASETS;IGNORE_DIRS" ${ARGN})
	
	#---get name of directory = name of subproject
	get_filename_component(KVSUBPROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	message(STATUS "Configuring ${KVSUBPROJECT}...")

	#---add to list of subprojects
	set_property(GLOBAL APPEND PROPERTY KALIVEDA_SUBPROJ_LIST ${KVSUBPROJECT})

	#---get list of modules (=subdirectories)
	file(GLOB module_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
	
	#---remove dataset dirs, etc.
	set(to_be_removed CMakeLists.txt etc data factory examples doc tools)
	list(REMOVE_ITEM module_list ${to_be_removed} ${ARG_DATASETS} ${ARG_IGNORE_DIRS})
	
	#---configure modules
	foreach(mod ${module_list})
		add_subdirectory(${mod})
	endforeach(mod)

	#---install configuration files
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/etc)
		file(GLOB kvrootrc RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/etc etc/*.rootrc)
  		set_property(GLOBAL APPEND PROPERTY KALIVEDA_CONF_FILES ${kvrootrc})
		install(DIRECTORY etc DESTINATION ${CMAKE_INSTALL_SYSCONFROOTDIR})
	endif()

	#---everything in data/ is installed in ${CMAKE_INSTALL_DATADIR}
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
		install(DIRECTORY data/ DESTINATION ${CMAKE_INSTALL_DATADIR})
	endif()

	#---everything in examples/ is installed in ${CMAKE_INSTALL_TUTDIR}/${KVSUBPROJECT}
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
		install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_TUTDIR}/${KVSUBPROJECT})
		BUILD_KALIVEDA_EXAMPLES_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/examples)
	endif()

	#---everything in factory/ is installed in ${CMAKE_INSTALL_TMPLDIR}
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/factory)
		install(DIRECTORY factory/ DESTINATION ${CMAKE_INSTALL_TMPLDIR})
	endif()

	#---install dataset directories in ${CMAKE_INSTALL_DATASETDIR}
	if(ARG_DATASETS)
		foreach(d ${ARG_DATASETS})
			install(DIRECTORY ${d} DESTINATION ${CMAKE_INSTALL_DATASETDIR})
			#---write Makefile for automatic database updating
			file(GLOB contents RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${d} ${d}/*)
			CHANGE_LIST_TO_STRING(st_contents ${contents})
			file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${d}_Makefile "$(KV_WORK_DIR)/db/${d}/DataBase.root : ${st_contents}\n")
			file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${d}_Makefile "	@echo Database needs update\n")
			install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${d}_Makefile DESTINATION ${CMAKE_INSTALL_DATASETDIR}/${d} RENAME Makefile)
		endforeach()
	endif()

endfunction()
