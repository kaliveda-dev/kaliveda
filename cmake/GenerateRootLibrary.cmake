include(CMakeParseArguments)

#---------------------------------------------------------------------------------------------------
#---INSTALL_CLASS_HEADERS(file1 file2 ... OPTIONS [options])
#---------------------------------------------------------------------------------------------------
function(INSTALL_CLASS_HEADERS)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "OPTIONS" ${ARGN})
  install(FILES ${ARG_UNPARSED_ARGUMENTS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                           COMPONENT dev
                           ${ARG_OPTIONS})
endfunction()


#---------------------------------------------------------------------------------------------------
#---GENERATE_ROOT_DICTIONARY(libname LINKDEF [name of LinkDef.h] HEADERS [toto.h titi.h ...] DEPENDENCIES [lib1 lib2])
#
#---Generate ROOT dictionary & rootmap.
#   This works with ROOT5 or ROOT6.
#   For ROOT6 we generate the .pcm file too.
#   rootmap and pcm will be installed in ${CMAKE_INSTALL_LIBDIR}
#
#---------------------------------------------------------------------------------------------------
function(GENERATE_ROOT_DICTIONARY libname)
  CMAKE_PARSE_ARGUMENTS(ARG "" "LINKDEF" "HEADERS;DEPENDENCIES" ${ARGN})

  if(${ROOT_VERSION} VERSION_LESS 6)
    ROOT_GENERATE_DICTIONARY(G__${libname} ${ARG_HEADERS} LINKDEF ${ARG_LINKDEF} OPTIONS -p)
    ROOT_GENERATE_ROOTMAP(${libname} LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_LINKDEF} DEPENDENCIES ${ARG_DEPENDENCIES})
    set_source_files_properties(${libname}.rootmap PROPERTIES GENERATED TRUE)
  else()
    ROOT_GENERATE_DICTIONARY(G__${libname} ${ARG_HEADERS} MODULE ${libname} LINKDEF ${ARG_LINKDEF} OPTIONS -noIncludePaths)    
    set_source_files_properties(${libname}_rdict.pcm ${libname}.rootmap PROPERTIES GENERATED TRUE)
  endif()
  set_source_files_properties(G__${libname}.cxx G__${libname}.h  PROPERTIES GENERATED TRUE)

endfunction()

#---------------------------------------------------------------------------------------------------
#---GENERATE_ROOT_LIBRARY(libname [DICT_EXCLUDE toto.h titi.h ...] [LIB_EXCLUDE Class1 Class2...] DEPENDENCIES lib1 lib2)
#
#---Generate ROOT dictionary & shared library from all .h & .cpp in current directory
#   Directory must also contain LinkDef.h file
#   This works with ROOT5 or ROOT6.
#   For ROOT5 we generate the rootmap. For ROOT6 we generate the .pcm
#
#  To exclude some headers from dictionary generation: DICT_EXCLUDE toto.h titi.h ...
#    - corresponding source file will still be compiled, and header will be installed
#  To exclude some classes from shared library: LIB_EXCLUDE Class1 Class2 ...
#    - corresponding source files will not be compiled
#    - corresponding header files will not be used in dictionary, and not installed
# 
#   All used headers will be installed in ${CMAKE_INSTALL_INCLUDEDIR}
#---------------------------------------------------------------------------------------------------
function(GENERATE_ROOT_LIBRARY libname)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "DICT_EXCLUDE;LIB_EXCLUDE;DEPENDENCIES" ${ARGN})
  
  #---get list of all headers in directory, except LinkDef.h
  set(rheaderfiles)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  file(GLOB files *.h)
  set(LINKDEF_FILE LinkDef.h)
  if(files)
    foreach(f ${files})
   	if(NOT f MATCHES LinkDef)
   	  set(rheaderfiles ${rheaderfiles} ${f})
   	endif()
    endforeach()
  endif()
  #--remove any headers which were given to argument DICT_EXCLUDE
  set(dict_headers ${rheaderfiles})
  if(ARG_DICT_EXCLUDE)
    foreach(head ${rheaderfiles})
      get_filename_component(no_dir_head ${head} NAME)
      foreach(exclH ${ARG_DICT_EXCLUDE})
        if(no_dir_head STREQUAL exclH)
          list(REMOVE_ITEM dict_headers ${head}) 
        endif()
      endforeach()
    endforeach()
  endif()
  #--all header files are installed, even if not used in dictionary generation
  set(inst_headers ${rheaderfiles})
  #--remove source path from header filenames to be given to dictionary generator
  set(dictgen_headers)
  foreach(head ${dict_headers})
    get_filename_component(no_dir_head ${head} NAME)
    set(dictgen_headers ${dictgen_headers} ${no_dir_head})
  endforeach()
  
  
  #---get list of all sources in directory
  #---for each class in LIB_EXCLUDE we remove the corresponding .cpp and .h
  #---from source & header file lists
  set(rsourcefiles)
  file(GLOB rsourcefiles *.cpp *.cxx)
  set(sourcefiles ${rsourcefiles})
  if(ARG_LIB_EXCLUDE)
    foreach(source ${rsourcefiles})
      get_filename_component(classname ${source} NAME_WE)
      get_filename_component(path_to_source ${source} PATH)
      foreach(exclSrc ${ARG_LIB_EXCLUDE})
        if(classname STREQUAL exclSrc)
          list(REMOVE_ITEM sourcefiles ${source})
	       list(REMOVE_ITEM dictgen_headers ${classname}.h)
	       list(REMOVE_ITEM inst_headers ${path_to_source}/${classname}.h)
        endif()
      endforeach()
    endforeach()
  endif()
  
  GENERATE_ROOT_DICTIONARY(${libname} LINKDEF ${LINKDEF_FILE} HEADERS ${dictgen_headers} DEPENDENCIES ${ARG_DEPENDENCIES})
  
  ROOT_LINKER_LIBRARY(${libname} ${sourcefiles} G__${libname}.cxx DEPENDENCIES ${ARG_DEPENDENCIES})
  
  INSTALL_CLASS_HEADERS(${inst_headers})
    
endfunction()
