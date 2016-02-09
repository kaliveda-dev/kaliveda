#--- Top-level CMake file for KaliVeda
cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

project(KaliVeda)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${PROJECT_SOURCE_DIR}/cmake)

#--look for ROOT & all essential libraries
find_package(ROOT
	REQUIRED Rint Graf Geom Spectrum Proof ProofPlayer Physics Hist
	RIO Tree MathCore Matrix Gpad Gui Net
   OPTIONAL_COMPONENTS RGL RooFit RooFitCore Html Minuit)

#--optional support for OpenGL
if(ROOT_opengl_FOUND)
	set(WITH_OPENGL yes)
	message(STATUS "ROOT with OpenGL support library")
else()
	message(STATUS "ROOT with no OpenGL support: KVEventViewer disabled")
endif()

#--check if HTML class reference can be made
if(ROOT_Html_LIBRARY)
   set(CAN_DO_HTMLDOC true)
endif(ROOT_Html_LIBRARY)


include(${ROOT_USE_FILE})
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
link_directories(${ROOT_LIBRARY_DIRS})

#--check for BOOST - minimum version corresponds progress_display
set(WITH_BOOST)
find_package(Boost 1.9)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   set(WITH_BOOST yes)
endif(Boost_FOUND)

include(KaliVedaInstallDirs)

include_directories(${CMAKE_BINARY_DIR})

include(KaliVedaMacros)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#---Version control informations
set(WITH_GIT_INFOS)
set(WITH_BZR_INFOS)
if(EXISTS ${PROJECT_SOURCE_DIR}/.git OR EXISTS ${PROJECT_SOURCE_DIR}/KVGitInfo.h)
   set(WITH_GIT_INFOS yes)
   #-enforce coding style
   #-make sure git pre-commit hook is in place
   if(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/tools/git-hook-pre-commit.sh ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
   endif()
   #-if we have git, enforce 'git pull --rebase' policy
   find_package(Git QUIET)
   if(GIT_FOUND)
     execute_process(COMMAND ${GIT_EXECUTABLE} config pull.rebase true WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
   endif(GIT_FOUND)
elseif(EXISTS ${PROJECT_SOURCE_DIR}/.bzr OR EXISTS ${PROJECT_SOURCE_DIR}/KVBzrInfo.h)
   set(WITH_BZR_INFOS yes)
endif()
#----------------------------------------------------------
#----------------------BUILD OPTIONS-----------------------
#----------------------------------------------------------
include(FeatureSummary)
include(CMakeDependentOption)

#---GNU-style installation directories or classic KaliVeda layout
option(gnuinstall "Use GNU-style installation directories (e.g. for packaging)" OFF)
add_feature_info(gnuinstall gnuinstall "Use GNU-style installation directories (e.g. for packaging)")
if(gnuinstall)
   set(WITH_GNU_INSTALL yes)
else(gnuinstall)
   set(WITH_GNU_INSTALL)
endif(gnuinstall)

#---Built-in package for reading GANIL acquisition data
option(USE_BUILTIN_GRU
		"Use built-in package for reading GANIL acquisition data (not MFM format)"
		ON)
add_feature_info(builtin_gru USE_BUILTIN_GRU "Use built-in package for reading GANIL acquisition data (not MFM format)")
if(USE_BUILTIN_GRU)
	set(WITH_BUILTIN_GRU yes)
	if(DEFINED ENV{SITE})
   	if($ENV{SITE} STREQUAL "CCIN2P3")	#-at IN2P3 Computing Centre, build with support for xrootd
			find_package(XROOTD)
			if(XROOTD_FOUND)
				set(CCIN2P3_XRD yes)
			endif()
		endif()
	endif()
	add_subdirectory(GanTape)
	include_directories(${BUILTIN_GRU_INC_DIR})
endif()

#---GRU Ganil ROOT Utilities for reading GANIL acquisition data
option(USE_GRU
		"Use GANIL ROOT utilities (GRU) library for reading data (including MFM format)"
		OFF)
add_feature_info(gru USE_GRU "Use GANIL ROOT utilities (GRU) library for reading data (including MFM format)")
if(USE_GRU)
	find_package(GRU)
	if(GRU_FOUND)
		set(WITH_GRULIB yes) #--- for #define WITH_GRULIB
		include_directories(${GRU_INC_DIR})
		link_directories(${GRU_LIB_DIR})
	endif(GRU_FOUND)
endif()

#---Ricardo Yanez 'range' tables
option(USE_RANGE
		"Build classes using Ricardo Yanez's 'range' library"
		OFF)
add_feature_info(range USE_RANGE "Build classes using Ricardo Yanez's 'range' library")
if(USE_RANGE)
	find_package(Range)
	if(RANGE_FOUND)
		set(WITH_RANGE_YANEZ yes)
		include_directories(${RANGE_INC_DIR})
		link_directories(${RANGE_LIB_DIR})
	endif()
endif(USE_RANGE)
CMAKE_DEPENDENT_OPTION(USE_MODIFIED_RANGE
	"Using modified version of range library with special treatment of gaseous materials..."
	OFF
	"WITH_RANGE_YANEZ"
	OFF
	)
if(USE_MODIFIED_RANGE)
	set(WITH_MODIFIED_RANGE_YANEZ yes)
endif()

#---Gemini++ interface
option(USE_GEMINI "Build interface to Gemini++ statistical decay code" ON)
if(USE_GEMINI)
	find_package(Gemini)
	if(GEMINI_FOUND)
		set(WITH_GEMINI yes)
		include_directories(${GEMINI_INC_DIR})
		link_directories(${GEMINI_LIB_DIR})
	else()
		set(USE_GEMINI OFF CACHE BOOL "" FORCE)
	endif()
endif(USE_GEMINI)
add_feature_info(gemini USE_GEMINI "Build interface to Gemini++ statistical decay code")

#--- ID Grid fitting routines provided by Laurent Tassan-Got
option(USE_FITLTG "Use Tassan-Got fits for deltaE-E identification grids" ON)
if(USE_FITLTG)
	#---check if fitltg module can be built; if not, option is disabled by default
	include(CheckFitLTG)
endif()
add_feature_info(fitltg USE_FITLTG "Use Tassan-Got fits for deltaE-E identification grids")
if(USE_FITLTG)
	set(WITH_FITLTG yes)
	add_subdirectory(fitltg-0.1)
	set(FITLTG_LIB fitltg)
endif()

#--- BackTrack package for model parameter optimisation
option(USE_BACKTRACK "Build BackTrack backtracing statistical analysis package" OFF)
if(USE_BACKTRACK)
	#---check if roofit is available
	if(NOT ROOT_roofit_FOUND)
      message(STATUS "You need to enable the RooFit package in ROOT to use BackTrack")
      set(USE_BACKTRACK OFF CACHE BOOL "" FORCE)
   endif()
endif()
add_feature_info(backtrack USE_BACKTRACK "Build BackTrack backtracing statistical analysis package")

#--- MicroStat package for calculating event statistical weights
option(USE_MICROSTAT "Build MicroStat package for event generation using statistical weights" OFF)
add_feature_info(microstat USE_MICROSTAT "Build MicroStat package for event generation using statistical weights")

#--- Libraries for INDRA data
option(USE_INDRA "Build libraries for analysis of INDRA data" OFF)

#--- Libraries for INDRA-VAMOS data
option(USE_INDRAVAMOS "Build libraries for analysis of INDRA-VAMOS data" OFF)
if(USE_INDRAVAMOS)
	set(USE_INDRA ON)
endif()

option(USE_DEPRECATED_VAMOS "Enable deprecated VAMOS code" OFF)
if (USE_DEPRECATED_VAMOS)
	set(USE_INDRAVAMOS ON)
	add_definitions(-D__ENABLE_DEPRECATED_VAMOS__)
endif()

#--- Libraries for FAZIA data
option(USE_FAZIA "Build libraries for analysis of FAZIA data" OFF)
if(USE_FAZIA)
	set(USE_INDRA ON)
endif()

#--- Enable build of all libraries
if(USE_ALL)
	set(USE_INDRA ON)
	set(USE_INDRAVAMOS ON)
	set(USE_FAZIA ON)
endif()
add_feature_info(indra USE_INDRA "Build libraries for analysis of INDRA data")
add_feature_info(indravamos USE_INDRAVAMOS "Build libraries for analysis of INDRA-VAMOS data")
add_feature_info(indravamosdep USE_DEPRECATED_VAMOS "Build libraries for deprecated INDRA-VAMOS analysis")
add_feature_info(fazia USE_FAZIA "Build libraries for analysis of FAZIA data")

#----------------------------------------------------------
#------------------END BUILD OPTIONS-----------------------
#----------------------------------------------------------

#------------------Print BUILD OPTIONS-----------------------
Feature_Summary(WHAT ENABLED_FEATURES DESCRIPTION "------ENABLED FEATURES:")
Feature_Summary(WHAT DISABLED_FEATURES DESCRIPTION "------DISABLED FEATURES:")


#---Set the library version in the main CMakeLists.txt------------------------------------------
file(READ ${CMAKE_SOURCE_DIR}/VERSION versionstr)
string(STRIP ${versionstr} versionstr)
string(REGEX REPLACE "([0-9]+)[.][0-9]+[/][0-9]+" "\\1" KV_MAJOR_VERSION ${versionstr})
string(REGEX REPLACE "[0-9]+[.]([0-9]+)[/][0-9]+" "\\1" KV_MINOR_VERSION ${versionstr})
string(REGEX REPLACE "[0-9]+[.][0-9]+[/]([0-9]+)" "\\1" KV_PATCH_VERSION ${versionstr})
set(KV_VERSION "${KV_MAJOR_VERSION}.${KV_MINOR_VERSION}.${KV_PATCH_VERSION}")
set(KV_VERSION_STRING "${versionstr}")
set(ROOT_LIBRARY_PROPERTIES ${ROOT_LIBRARY_PROPERTIES}
      VERSION ${KV_VERSION}
      SOVERSION ${KV_MAJOR_VERSION}
		)


#---configure the KVVersion.h file
include(CreateKaliVedaVersionHeader)
create_kaliveda_version_header()

#---configure the KVConfig.h file
include(CreateConfigHeader)
create_config_header()

add_subdirectory(KVMultiDet)
get_property(KVMultiDet_MOD_LIST GLOBAL PROPERTY KVMultiDet_MOD_LIST)
get_property(KVMultiDet_LIB_LIST GLOBAL PROPERTY KVMultiDet_LIB_LIST)
if(USE_BACKTRACK)
	add_subdirectory(BackTrack)
endif()
if(USE_MICROSTAT)
	add_subdirectory(MicroStat)
endif()
if(USE_INDRA)
	add_subdirectory(KVIndra)
	get_property(KVIndra_MOD_LIST GLOBAL PROPERTY KVIndra_MOD_LIST)
	get_property(KVIndra_LIB_LIST GLOBAL PROPERTY KVIndra_LIB_LIST)
endif()
if(USE_INDRAVAMOS)
	add_subdirectory(VAMOS)
endif()
if(USE_FAZIA)
	add_subdirectory(FAZIA)
endif()

add_subdirectory(execs)
add_subdirectory(etc/proof)

#---set up list of '.rootrc' config files to be read by KVBase::InitEnvironment
get_property(KALIVEDA_CONF_FILES GLOBAL PROPERTY KALIVEDA_CONF_FILES)
#-at IN2P3 Computing Centre, we need to tack on some infos for access
#-to the batch system & data repositories
if(DEFINED ENV{SITE})
   if($ENV{SITE} STREQUAL "CCIN2P3")
	   if($ENV{GROUP} STREQUAL "indra")
		   list(APPEND KALIVEDA_CONF_FILES ccali.rootrc)
		   install(FILES etc/ccali.rootrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
	   elseif($ENV{GROUP} STREQUAL "fazia")
		   list(APPEND KALIVEDA_CONF_FILES ccali-fazia.rootrc)
		   install(FILES etc/ccali-fazia.rootrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
	   endif()
   endif()
else()
    #-for standard install outside CCIN2P3, tack on the 'standard.rootrc' config
    #-(just defines default batch system)
    list(APPEND KALIVEDA_CONF_FILES standard.rootrc)
    install(FILES etc/standard.rootrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
endif()
configure_file(etc/config.files.in etc/config.files)
install(FILES ${CMAKE_BINARY_DIR}/etc/config.files DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

#---configure and install kaliveda-config script
get_property(KALIVEDA_SUBPROJ_LIST GLOBAL PROPERTY KALIVEDA_SUBPROJ_LIST)
CHANGE_LIST_TO_STRING(subprojlist ${KALIVEDA_SUBPROJ_LIST})
configure_file(kaliveda-config.in kaliveda-config @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/kaliveda-config DESTINATION ${CMAKE_INSTALL_BINDIR})

#---install scripts from tools/ directory in ${CMAKE_INSTALL_BINDIR}
install(PROGRAMS tools/SetUpKaliVeda.sh tools/SetUpROOT.sh tools/WhichROOT
   tools/SetUpKaliVeda.csh tools/SetUpROOT.csh tools/SetUpKaliVedaDirectories.sh
   tools/SetUpROOTDirectories.sh tools/WhichKaliVeda tools/thiskaliveda.sh tools/thiskaliveda.csh
   DESTINATION ${CMAKE_INSTALL_BINDIR})
#---install other stuff from tools/ directory in ${CMAKE_INSTALL_SYSCONFDIR}
install(FILES tools/.nedit DESTINATION ${CMAKE_INSTALL_SYSCONFDIR} RENAME nedit.cf)
install(FILES tools/kaliveda.m4 DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

#---install exported targets file for use by other projects
install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION ${CMAKE_INSTALL_CMAKEPKGDIR} FILE ${CMAKE_PROJECT_NAME}-targets.cmake)

#---generate config & version files for other projects using find_package()
include(CMakePackageConfigHelpers)
get_property(KALIVEDA_LIB_LIST GLOBAL PROPERTY KALIVEDA_LIB_LIST)
configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/KaliVedaConfig.cmake.in
                                 ${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfig.cmake
                                 INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEPKGDIR}
                                 PATH_VARS CMAKE_INSTALL_FULL_INCLUDEDIR CMAKE_INSTALL_FULL_LIBDIR CMAKE_INSTALL_FULL_BINDIR CMAKE_INSTALL_FULL_CMAKEPKGDIR
                                 )
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfigVersion.cmake
                                    VERSION ${KV_VERSION}
                                    COMPATIBILITY SameMajorVersion
                                    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfigVersion.cmake
          ${CMAKE_SOURCE_DIR}/cmake/KaliVedaUseFile.cmake DESTINATION ${CMAKE_INSTALL_CMAKEPKGDIR}
           )

#---add a 'make dist' target
set(tarprefix "kaliveda-${KV_VERSION}")
set(tarball "${tarprefix}.tar.gz")
if(WITH_GIT_INFOS)
   add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${tarball}
      COMMAND git archive --prefix=${tarprefix}/ -o ${CMAKE_CURRENT_BINARY_DIR}/${tarball} HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
   add_custom_target(dist DEPENDS ${CMAKE_BINARY_DIR}/${tarball})
endif()
if(WITH_BZR_INFOS)
   add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${tarball}
      COMMAND bzr export --root=${tarprefix}/ ${CMAKE_CURRENT_BINARY_DIR}/${tarball}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
   add_custom_target(dist DEPENDS ${CMAKE_BINARY_DIR}/${tarball})
endif()

#---install man pages for KaliVeda
install(DIRECTORY ${CMAKE_SOURCE_DIR}/man/ DESTINATION ${CMAKE_INSTALL_MANDIR})

#------------------------HTML CLASS REFERENCE & EXAMPLES GENERATION
if(CAN_DO_HTMLDOC)

   CONVERT_ALL_EXAMPLE_FUNCTIONS()

   get_property(KALIVEDA_EXFUNCS GLOBAL PROPERTY KALIVEDA_EXFUNCS)
   get_property(KALIVEDA_EXCLASS GLOBAL PROPERTY KALIVEDA_EXCLASS)

   #---add a 'html' target to generate ROOT class reference
   file(WRITE ${CMAKE_BINARY_DIR}/makehtml.C "{\nmakedoc(\"${CMAKE_BINARY_DIR}/htmldoc\");\n}")
   add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/htmldoc/index.html
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tools/html ${CMAKE_BINARY_DIR}
      COMMAND ${ROOT_EXECUTABLE} -q -l makehtml.C
      DEPENDS ${KALIVEDA_LIB_LIST}
      )
   add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/htmldoc/examples
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/htmldoc/examples
      )
   get_property(EXAMPLE_INDEX_TARGETS GLOBAL PROPERTY EXAMPLE_INDEX_TARGETS)
   get_property(EXAMPLE_SUBPROJ_TARGETS GLOBAL PROPERTY EXAMPLE_SUBPROJ_TARGETS)
   get_property(CLEAN_EXAMPLE_SUBPROJ_TARGETS GLOBAL PROPERTY CLEAN_EXAMPLE_SUBPROJ_TARGETS)
   add_custom_target(classref DEPENDS ${CMAKE_BINARY_DIR}/htmldoc/index.html)
   add_custom_target(exampledir DEPENDS ${CMAKE_BINARY_DIR}/htmldoc/examples)
   foreach(Exa ${KALIVEDA_EXFUNCS} ${KALIVEDA_EXCLASS})
      add_dependencies(${Exa} ${EXAMPLE_INDEX_TARGETS} exampledir)
   endforeach()
   add_custom_target(examples)
   add_dependencies(examples ${EXAMPLE_SUBPROJ_TARGETS})
   add_custom_target(html)
   add_dependencies(classref examples)
   add_dependencies(html ${CLEAN_EXAMPLE_SUBPROJ_TARGETS} classref examples)

endif(CAN_DO_HTMLDOC)
