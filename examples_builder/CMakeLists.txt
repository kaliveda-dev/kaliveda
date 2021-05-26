# Generate and compile example analysis classes

#---set paths to find all KVMultiDet headers
KALIVEDA_SET_INCLUDE_DIRS(KVMultiDet MODULES ${KVMultiDet_MOD_LIST})
add_executable(generate_compile_example_analysis generate_compile_example_analysis.cpp)
target_link_libraries(generate_compile_example_analysis ${KVMultiDet_LIB_LIST})

install(TARGETS generate_compile_example_analysis RUNTIME DESTINATION bin)
