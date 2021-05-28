#--- Top-level CMake file for KaliVeda
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    " KaliVeda must be built out-of-source.\n"
    " Please see http://indra.in2p3.fr/kaliveda for instructions.")
endif()

project(KaliVeda)

#-- Fixing warning on Mac OSX due to CMake version
if(CMAKE_MAJOR_VERSION GREATER 2)
   set(CMAKE_MACOSX_RPATH 1)
endif()

#- set path to our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

#--look for ROOT & all essential libraries
find_package(ROOT QUIET
   COMPONENTS Rint Graf Geom Spectrum Proof ProofPlayer Physics Hist
   RIO Tree MathCore Matrix Gpad Gui Net Graf3d Thread
   OPTIONAL_COMPONENTS RGL RooFit RooFitCore Minuit RSQLite SQLite)

if(NOT ROOT_FOUND)
   message(STATUS "Looking for ROOT built without cmake...")
#- may have ROOT built without cmake support
#- try again with included FindROOT.cmake
   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/ROOT)
   find_package(ROOT REQUIRED
      Rint Graf Geom Spectrum Proof ProofPlayer Physics Hist
      RIO Tree MathCore Matrix Gpad Gui Net Graf3d Thread
      OPTIONAL_COMPONENTS RGL RooFit RooFitCore Minuit RSQLite SQLite)
endif(NOT ROOT_FOUND)

#--optional support for OpenGL
if(ROOT_opengl_FOUND)
	set(WITH_OPENGL yes)
	message(STATUS "ROOT with OpenGL support library")
else()
	message(STATUS "ROOT with no OpenGL support: KVEventViewer disabled")
endif()

include(SetUpROOTBuild)

#--determine what C++ standard is used by the compiler
include(GetCompilerStandard)
GET_COMPILER_STANDARD(CURRENT_CXX_COMPILER_STANDARD)

#--if C++11 support is not enabled (i.e. current standard is < C++11)
#--we require it here (this can only happen with ROOT5)
if(CURRENT_CXX_COMPILER_STANDARD LESS CXX_STANDARD_11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED yes)
    message(STATUS "KaliVeda: C++11 support is required and has been enabled")
endif()

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

# use to add any extra paths to headers that will need to be added to ROOT_INCLUDE_PATH
# (for ROOT6) in order for interpreter to work correctly
set(EXTRA_ROOT_INCLUDE_PATH)

# kludge to fix bug with ROOT5/CINT when utility libraries are installed in system directories
# in this case rootmap autoloading fails if system directories not in LD_LIBRARY_PATH
set(ADD_DYN_PATH)

#---Version control informations
set(WITH_GIT_INFOS)
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

#--optional support for SQLite
option(USE_SQLITE "Enable sqlite database interface KVSQLite" ON)
if(USE_SQLITE)
   if(ROOT_sqlite_FOUND) # sqlite is in output of root-config --features
      if(ROOT_SQLite_LIBRARY OR ROOT_RSQLite_LIBRARY)
         set(WITH_RSQLITE yes)
		   #--up to 6/10/2015 if ROOT was built using autoconf not cmake
         #  the library was called libSQLite not libRSQLite
		   if(ROOT_SQLite_LIBRARY)
			   set(ROOT_RSQLite_LIBRARY ${ROOT_SQLite_LIBRARY})
		   endif(ROOT_SQLite_LIBRARY)
      else(ROOT_SQLite_LIBRARY OR ROOT_RSQLite_LIBRARY)
         message(STATUS "ROOT with no SQLite support: KVSQLite disabled")
         set(USE_SQLITE OFF CACHE BOOL "" FORCE)
      endif(ROOT_SQLite_LIBRARY OR ROOT_RSQLite_LIBRARY)
   else()
      message(STATUS "ROOT with no SQLite support: KVSQLite disabled")
      set(USE_SQLITE OFF CACHE BOOL "" FORCE)
   endif()
endif(USE_SQLITE)
add_feature_info(sqlite USE_SQLITE "Enable sqlite database interface KVSQLite")

#--set up build environment for CCIN2P3
option(CCIN2P3_BUILD "Configure build for CCIN2P3 environment" OFF)
if(CCIN2P3_BUILD)
    message(STATUS "Configuring build for CCIN2P3 environment")
endif(CCIN2P3_BUILD)
add_feature_info(ccin2p3 CCIN2P3_BUILD "Configure build for CCIN2P3 environment")

#---Built-in package for reading GANIL acquisition data
if(APPLE)
    option(USE_BUILTIN_GRU
        "Use built-in package for reading GANIL acquisition data (not MFM format) -- Disable by default if compiling on MacOS"
        OFF)
    add_feature_info(builtin_gru USE_BUILTIN_GRU "Use built-in package for reading GANIL acquisition data (not MFM format) -- Disable by default if compiling on MacOS")
else()
    option(USE_BUILTIN_GRU
        "Use built-in package for reading GANIL acquisition data (not MFM format)"
        ON)
    add_feature_info(builtin_gru USE_BUILTIN_GRU "Use built-in package for reading GANIL acquisition data (not MFM format)")
endif(APPLE)

if(USE_BUILTIN_GRU)
	set(WITH_BUILTIN_GRU yes)
        if(CCIN2P3_BUILD)
            #-at IN2P3 Computing Centre, build with support for xrootd
            find_package(XROOTD)
            if(XROOTD_FOUND)
                set(CCIN2P3_XRD yes)
            endif(XROOTD_FOUND)
        endif(CCIN2P3_BUILD)
	add_subdirectory(GanTape)
	include_directories(${BUILTIN_GRU_INC_DIR})
endif()

#---MFM library for GANIL MFM data
#---sources available from: https://gitlab.in2p3.fr/jdfcode/mfmlib.git
option(USE_MFM
                "Enable support for reading GANIL MFM format data"
                ON)
add_feature_info(mfm USE_MFM "Enable support for reading GANIL MFM format data")
if(USE_MFM)
    find_package(mfmlib QUIET)
    if(mfmlib_FOUND)
       include_directories(${mfmlib_INCLUDE_DIRS})
       set(WITH_MFM yes)
       list(APPEND EXTRA_ROOT_INCLUDE_PATH ${mfmlib_INCLUDE_DIRS})
    else(mfmlib_FOUND)
       message(STATUS "You need to install the mfmlib package in order to enable this option")
       message(STATUS "You can clone the sources from: https://gitlab.in2p3.fr/jdfcode/mfmlib.git")
       set(USE_MFM OFF CACHE BOOL "" FORCE)
    endif(mfmlib_FOUND)
endif()

#---Google Protobuffers for reading FAZIA data

option(USE_PROTOBUF "Enable support for PROTOBUF needed to read FAZIA acquisition data" ON)
if(USE_PROTOBUF)
find_package(Protobuf)
if(NOT PROTOBUF_FOUND)
   # we may need pkg-config to find installation
   find_package(PkgConfig QUIET)
   
   if(PkgConfig_FOUND)
      message(STATUS "Looking for protobuf with pkg-config")
      ## use pkg-config to get hints for protobuf locations
      pkg_check_modules(PROTOBUF QUIET protobuf)
      if(PROTOBUF_FOUND)
         set(CMAKE_LIBRARY_PATH ${PROTOBUF_LIBRARY_DIRS} ${CMAKE_LIBRARY_PATH})
         set(PROTOBUF_INCLUDE_DIR ${PROTOBUF_INCLUDE_DIRS})
         set(Protobuf_INCLUDE_DIR ${PROTOBUF_INCLUDE_DIRS})
         find_package(Protobuf)
      endif(PROTOBUF_FOUND)
   endif(PkgConfig_FOUND)

endif(NOT PROTOBUF_FOUND)
if(PROTOBUF_FOUND)
    set(WITH_PROTOBUF yes)
    include_directories(${PROTOBUF_INCLUDE_DIRS})
    if(${ROOT_VERSION} VERSION_LESS 6)
               # kludge to fix bug with ROOT5/CINT when protobuf installed in system directories
               # in this case rootmap autoloading fails if libprotobuf directory not in LD_LIBRARY_PATH
               get_filename_component(ADD_PROTOBUF_DYN_PATH ${PROTOBUF_LIBRARY} PATH)
               list(APPEND ADD_DYN_PATH ${ADD_PROTOBUF_DYN_PATH})
    endif()
endif(PROTOBUF_FOUND)
endif(USE_PROTOBUF)

#---software for reading Mesytec data (new INDRA DAQ)
option(USE_MESYTEC
                "Enable support for reading Mesytec DAQ data"
                ON)
add_feature_info(mesytec USE_MESYTEC "Enable support for reading Mesytec DAQ data")
if(USE_MESYTEC)
    find_package(mesytec_data QUIET)
    if(mesytec_data_FOUND)
       include_directories(${mesytec_data_INCLUDE_DIRS})
       set(WITH_MESYTEC yes)
       list(APPEND EXTRA_ROOT_INCLUDE_PATH ${mesytec_data_INCLUDE_DIRS})
    else(mesytec_data_FOUND)
       set(USE_MESYTEC OFF CACHE BOOL "" FORCE)
    endif(mesytec_data_FOUND)
endif()

#---optional support for zeroMQ sockets
find_package(ZMQ)
if(ZMQ_FOUND)
    include_directories(${ZMQ_INCLUDE_DIRS})
    set(WITH_ZMQ yes)
    if(${ROOT_VERSION} VERSION_LESS 6)
        # kludge to fix bug with ROOT5/CINT when zeromq installed in system directories
        # in this case rootmap autoloading fails if libzmq directory not in LD_LIBRARY_PATH
        get_filename_component(ADD_ZMQ_DYN_PATH ${ZMQ_LIBRARIES} PATH)
        list(APPEND ADD_DYN_PATH ${ADD_ZMQ_DYN_PATH})
    endif()
endif(ZMQ_FOUND)

#---online analysis of reconstructed data
#---give path to 'dataflow' installation directory -DDATAFLOW_INSTALL_DIR=/...
if(DEFINED DATAFLOW_INSTALL_DIR)
    message(STATUS "Checking for DataFlow installation in ${DATAFLOW_INSTALL_DIR}...")
    find_file(DATAFLOW_PORTS_HDR dataflowports.h PATHS ${DATAFLOW_INSTALL_DIR}/include)
    if(DATAFLOW_PORTS_HDR)
        message(STATUS "---found header file ${DATAFLOW_PORTS_HDR}")
        set(WITH_DATAFLOW yes)
        include_directories(${DATAFLOW_INSTALL_DIR}/include)
    endif()
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

# no more support for BackTrack from 6.18
if(ROOT_VERSION VERSION_LESS 6.18.00)
   #--- BackTrack package for model parameter optimisation
   option(USE_BACKTRACK "Build BackTrack backtracing statistical analysis package" ON)
   if(USE_BACKTRACK)
	   #---check if roofit is available
	   if(NOT ROOT_roofit_FOUND)
         message(STATUS "You need to enable the RooFit package in ROOT to use BackTrack")
         set(USE_BACKTRACK OFF CACHE BOOL "" FORCE)
      endif()
   endif()
   add_feature_info(backtrack USE_BACKTRACK "Build BackTrack backtracing statistical analysis package")
else(ROOT_VERSION VERSION_LESS 6.18.00)
   message(STATUS "BackTrack package not available from ROOT v6.18 onwards")
   set(USE_BACKTRACK OFF CACHE BOOL "" FORCE)
endif(ROOT_VERSION VERSION_LESS 6.18.00)

#--- MicroStat package for calculating event statistical weights
option(USE_MICROSTAT "Build MicroStat package for event generation using statistical weights" ON)
add_feature_info(microstat USE_MICROSTAT "Build MicroStat package for event generation using statistical weights")

#--- Libraries for INDRA data
option(USE_INDRA "Build libraries for analysis of INDRA data" OFF)

#--- Libraries for INDRA-VAMOS data - disabled - does not compile
#option(USE_INDRAVAMOS "Build libraries for analysis of INDRA-VAMOS data" OFF)
#if(USE_INDRAVAMOS)
#	set(USE_INDRA ON)
#endif()

#option(USE_DEPRECATED_VAMOS "Enable deprecated VAMOS code" OFF)
#if (USE_DEPRECATED_VAMOS)
#	set(USE_INDRAVAMOS ON)
#	add_definitions(-D__ENABLE_DEPRECATED_VAMOS__)
#endif()

#--- Libraries for FAZIA data
option(USE_FAZIA "Build libraries for analysis of FAZIA data" OFF)

#--- Libraries for IDNRAFAZIA data
option(USE_INDRAFAZIA "Build libraries for analysis of INDRAFAZIA data" OFF)
if(USE_INDRAFAZIA)
	set(USE_INDRA ON)
	set(USE_FAZIA ON)   
endif()

#--- Enable build of all libraries
if(USE_ALL)
	set(USE_INDRA ON)
	set(USE_FAZIA ON)
	set(USE_INDRAFAZIA ON)
endif()
add_feature_info(indra USE_INDRA "Build libraries for analysis of INDRA data")
#add_feature_info(indravamos USE_INDRAVAMOS "Build libraries for analysis of INDRA-VAMOS data")
#add_feature_info(indravamosdep USE_DEPRECATED_VAMOS "Build libraries for deprecated INDRA-VAMOS analysis")
add_feature_info(fazia USE_FAZIA "Build libraries for analysis of FAZIA data")
add_feature_info(indrafazia USE_INDRAFAZIA "Build libraries for analysis of INDRAFAZIA data")

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
   if(CCIN2P3_BUILD)
      if($ENV{GROUP} STREQUAL "indra")
         #- if CERNLIB is available, compile Fortran for reading DSTs
         find_package(CERNLIB QUIET)
         if(CERNLIB_FOUND)
            set(USE_INDRA2ROOT ON)
         endif(CERNLIB_FOUND)
      endif($ENV{GROUP} STREQUAL "indra")
   endif(CCIN2P3_BUILD)
	add_subdirectory(KVIndra)
	get_property(KVIndra_MOD_LIST GLOBAL PROPERTY KVIndra_MOD_LIST)
	get_property(KVIndra_LIB_LIST GLOBAL PROPERTY KVIndra_LIB_LIST)
endif()
#if(USE_INDRAVAMOS)
#	add_subdirectory(VAMOS)
#endif()
if(USE_FAZIA)
	add_subdirectory(FAZIA)
	get_property(FAZIA_MOD_LIST GLOBAL PROPERTY FAZIA_MOD_LIST)
	get_property(FAZIA_LIB_LIST GLOBAL PROPERTY FAZIA_LIB_LIST)
endif()
if(USE_INDRAFAZIA)
	add_subdirectory(indrafazia)
endif()

add_subdirectory(execs)
add_subdirectory(etc/proof)
add_subdirectory(examples_builder)

#---set up list of '.rootrc' config files to be read by KVBase::InitEnvironment
get_property(KALIVEDA_CONF_FILES GLOBAL PROPERTY KALIVEDA_CONF_FILES)
#-at IN2P3 Computing Centre, we need to tack on some infos for access
#-to the batch system & data repositories
if(CCIN2P3_BUILD)
   if($ENV{GROUP} STREQUAL "indra")
      list(APPEND KALIVEDA_CONF_FILES ccali-indra.rootrc)
      install(FILES etc/ccali-indra.rootrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
   elseif($ENV{GROUP} STREQUAL "fazia")
      list(APPEND KALIVEDA_CONF_FILES ccali-fazia.rootrc)
      install(FILES etc/ccali-fazia.rootrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
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
CHANGE_LIST_TO_COLONATED_STRING(EXTRA_ROOT_INCLUDE_PATH ${EXTRA_ROOT_INCLUDE_PATH})
configure_file(tools/thiskaliveda.sh.in thiskaliveda.sh @ONLY)
configure_file(tools/thiskaliveda.csh.in thiskaliveda.csh @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/thiskaliveda.sh ${CMAKE_BINARY_DIR}/thiskaliveda.csh
   DESTINATION ${CMAKE_INSTALL_BINDIR})
#---install other stuff from tools/ directory in ${CMAKE_INSTALL_SYSCONFDIR}
install(FILES tools/nedit.cf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

#---install README, COPYING, etc. in ${CMAKE_INSTALL_DOCDIR}
install(FILES COPYING INSTALL README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

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
install(FILES
         ${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfig.cmake
         ${CMAKE_CURRENT_BINARY_DIR}/KaliVedaConfigVersion.cmake
         ${CMAKE_SOURCE_DIR}/cmake/KaliVedaUseFile.cmake
         ${CMAKE_SOURCE_DIR}/cmake/FindZMQ.cmake
         DESTINATION ${CMAKE_INSTALL_CMAKEPKGDIR}
        )

#---in order to be able to reuse cmake macros for building ROOT libraries with either ROOT5 or ROOT6
install(FILES 
   ${CMAKE_SOURCE_DIR}/cmake/GenerateRootLibrary.cmake ${CMAKE_SOURCE_DIR}/cmake/SetUpROOTBuild.cmake
    DESTINATION ${CMAKE_INSTALL_CMAKEPKGDIR}
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

#---install man pages for KaliVeda
install(DIRECTORY ${CMAKE_SOURCE_DIR}/man/ DESTINATION ${CMAKE_INSTALL_MANDIR})
