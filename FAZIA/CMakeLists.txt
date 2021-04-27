#---list of datasets
set(datasets FAZIAPRE FAZIAPRE-MAY2018 TESTLNS17 LNS_2014 ISOFAZIA FAZIASYM FAZIANFS LNS_2016 FAZIACOR FAZIADEM FAZIAT2103)
set(ignored GANIL_2010 LNL_2007 LNS_2009 LNS_2011 LNS_2012)

if(WITH_PROTOBUF)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/protobuf)
else(WITH_PROTOBUF)
    set(ignored ${ignored} protobuf daq_cec)
endif(WITH_PROTOBUF)

#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})

BUILD_KALIVEDA_SUBPROJECT(DATASETS ${datasets}
			IGNORE_DIRS ${ignored}
)
