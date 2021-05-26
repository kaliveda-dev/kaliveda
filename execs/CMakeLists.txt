#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

if(USE_INDRA)
    #---set paths to find all KVIndra headers
    KALIVEDA_SET_INCLUDE_DIRS(KVIndra MODULES ${KVIndra_MOD_LIST})
endif()
set(build_targets
    analyse_logs delete_logs kvdatanalyser kvtreeanalyzer
    KaliVedaAnalysis update_runlist kaliveda kaliveda-sim
    KaliVedaGUI
    )
if(USE_INDRA)
    list(APPEND build_targets KVDataBaseGUI)
endif()
if(USE_INDRAFAZIA AND WITH_MFM)
    list(APPEND build_targets irods_uploader)
    KALIVEDA_SET_INCLUDE_DIRS(FAZIA MODULES ${FAZIA_MOD_LIST})
endif()

set(kaliveda_SRC kaliveda.cpp)

set(kaliveda-sim_SRC kaliveda-sim.cpp)

set(analyse_logs_SRC analyse_logs.cxx)

set(delete_logs_SRC delete_logs.cxx)

set(kvdatanalyser_SRC kvdatanalyser.cxx)

set(kvtreeanalyzer_SRC kvtreeanalyzer.cxx)

set(KVDataBaseGUI_SRC runsheetreader.cpp)
set(KVDataBaseGUI_LIB ${KVIndra_LIB_LIST})

set(KaliVedaAnalysis_SRC KaliVedaAnalysis.cpp)

set(KaliVedaGUI_SRC KaliVedaGUI.cpp)

set(update_runlist_SRC update_runlist.cxx)

set(irods_uploader_SRC irods_uploader.cpp)

foreach(t ${build_targets})
    add_executable(${t} ${${t}_SRC})
    target_link_libraries(${t} ${KVMultiDet_LIB_LIST} ${${t}_LIB})
endforeach()

#---install built executables
install(TARGETS ${build_targets} EXPORT ${CMAKE_PROJECT_NAME}Exports RUNTIME DESTINATION bin) 

#---install shell script for batch
set(scripts KVA)
install(PROGRAMS ${scripts} DESTINATION bin)
