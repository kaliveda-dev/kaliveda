macro( create_config_header )

	set(__WITHOUT_TGBUTTON_SETENABLED)
	if(ROOT_VERSION VERSION_LESS 5.02.00)
		set(__WITHOUT_TGBUTTON_SETENABLED yes)
	endif(ROOT_VERSION VERSION_LESS 5.02.00)
	
	set(__WITHOUT_TMACRO)
	if(ROOT_VERSION VERSION_LESS 5.04.00)
		set(__WITHOUT_TMACRO yes)
	endif(ROOT_VERSION VERSION_LESS 5.04.00)

	set(__WITHOUT_TSYSTEM_FINDFILE)
	if(ROOT_VERSION VERSION_LESS 5.12.00)
		set(__WITHOUT_TSYSTEM_FINDFILE yes)
	endif(ROOT_VERSION VERSION_LESS 5.12.00)
	
	set(__WITH_KVSTRING_ISDIGIT)
	if(ROOT_VERSION VERSION_LESS 5.13.06)
		set(__WITH_KVSTRING_ISDIGIT yes)
	endif(ROOT_VERSION VERSION_LESS 5.13.06)
	
	set(__WITH_KVSTRING_ISFLOAT)
	if(ROOT_VERSION VERSION_LESS 5.13.06)
		set(__WITH_KVSTRING_ISFLOAT yes)
	endif(ROOT_VERSION VERSION_LESS 5.13.06)
   
	set(__WITH_KVSTRING_ATOI)
	if(ROOT_VERSION VERSION_LESS 5.08.00)
		set(__WITH_KVSTRING_ATOI yes)
	endif(ROOT_VERSION VERSION_LESS 5.08.00)
   
	set(__WITH_KVSTRING_ATOF)
	if(ROOT_VERSION VERSION_LESS 5.08.00)
		set(__WITH_KVSTRING_ATOF yes)
	endif(ROOT_VERSION VERSION_LESS 5.08.00)
   
	set(__WITH_KVSTRING_ISWHITESPACE)
	if(ROOT_VERSION VERSION_LESS 5.08.00)
		set(__WITH_KVSTRING_ISWHITESPACE yes)
	endif(ROOT_VERSION VERSION_LESS 5.08.00)
   
	set(__WITHOUT_TNETSYSTEM_CTOR_BOOL_T)
	if(ROOT_VERSION VERSION_LESS 5.10.00)
		set(__WITHOUT_TNETSYSTEM_CTOR_BOOL_T yes)
	endif(ROOT_VERSION VERSION_LESS 5.10.00)
   
	set(__WITHOUT_TGNUMBERENTRY_SETNUMSTYLE)
	if(ROOT_VERSION VERSION_LESS 5.11.02)
		set(__WITHOUT_TGNUMBERENTRY_SETNUMSTYLE yes)
	endif(ROOT_VERSION VERSION_LESS 5.11.02)
   
	set(__WITHOUT_TGNUMBERENTRY_SETNUMATTR)
	if(ROOT_VERSION VERSION_LESS 5.11.02)
		set(__WITHOUT_TGNUMBERENTRY_SETNUMATTR yes)
	endif(ROOT_VERSION VERSION_LESS 5.11.02)
   
	set(__WITHOUT_TGCOMBOBOX_REMOVEALL)
	if(ROOT_VERSION VERSION_LESS 5.12.00)
		set(__WITHOUT_TGCOMBOBOX_REMOVEALL yes)
	endif(ROOT_VERSION VERSION_LESS 5.12.00)
   
	set(__WITHOUT_TGCOMBOBOX_SELECT_BOOL_T)
	if(ROOT_VERSION VERSION_LESS 5.12.00)
		set(__WITHOUT_TGCOMBOBOX_SELECT_BOOL_T yes)
	endif(ROOT_VERSION VERSION_LESS 5.12.00)
   
	set(__WITHOUT_THTML_SETPRODUCTNAME)
	if(ROOT_VERSION VERSION_LESS 5.17.00)
		set(__WITHOUT_THTML_SETPRODUCTNAME yes)
	endif(ROOT_VERSION VERSION_LESS 5.17.00)
   
	set(__WITH_OLD_TREFARRAY)
	if(ROOT_VERSION VERSION_LESS 5.20.00)
		set(__WITH_OLD_TREFARRAY yes)
	endif(ROOT_VERSION VERSION_LESS 5.20.00)
   
	set(__WITH_NEW_TCOLLECTION_PRINT)
	if(ROOT_VERSION VERSION_GREATER 5.20.00)
		set(__WITH_NEW_TCOLLECTION_PRINT yes)
	endif(ROOT_VERSION VERSION_GREATER 5.20.00)
   
	set(__WITH_TCOLLECTION_PRINT_WILDCARD)
	if((ROOT_VERSION VERSION_LESS 5.20.00) OR (ROOT_VERSION VERSION_EQUAL 5.20.00))
		set(__WITH_TCOLLECTION_PRINT_WILDCARD yes)
	endif((ROOT_VERSION VERSION_LESS 5.20.00) OR (ROOT_VERSION VERSION_EQUAL 5.20.00))
   
	set(__WITHOUT_TCA_CONSTRUCTED_AT)
	if(ROOT_VERSION VERSION_LESS 5.32.00)
		set(__WITHOUT_TCA_CONSTRUCTED_AT yes)
	endif(ROOT_VERSION VERSION_LESS 5.32.00)
   
	set(__WITH_KVSTRING_ITOA)
	if(ROOT_VERSION VERSION_LESS 5.33.02)
		set(__WITH_KVSTRING_ITOA yes)
   endif(ROOT_VERSION VERSION_LESS 5.33.02)

   set(__WITH_TITER_BUG)
   if(ROOT_VERSION VERSION_LESS 5.34.28)
      set(__WITH_TITER_BUG yes)
   endif(ROOT_VERSION VERSION_LESS 5.34.28)

	#--- get infos on number of processors on machine
	include(ProcessorCount)
	ProcessorCount(N)
	if(NOT N EQUAL 0)
	    set(WITH_MULTICORE_CPU ${N})
	endif()

	configure_file(
		${CMAKE_SOURCE_DIR}/KVConfig.h.in
		${CMAKE_BINARY_DIR}/KVConfig.h
	)
	install(FILES ${CMAKE_BINARY_DIR}/KVConfig.h
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT headers
	)
	
endmacro( create_config_header )