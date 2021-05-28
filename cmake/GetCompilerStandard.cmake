set(CXX_STANDARD_11 201103)
set(CXX_STANDARD_14 201402)
set(CXX_STANDARD_17 201703)
# -------------------------------------------------------------------------------------------
#--- GET_COMPILER_STANDARD(<variable>)
#
# Sets <variable> to numeric value of __cplusplus i.e. the C++ standard used by the compiler
#--------------------------------------------------------------------------------------------
function(GET_COMPILER_STANDARD variable)

    try_run(RUN_RESULT_VAR COMPILE_RESULT_VAR
       ${CMAKE_CURRENT_BINARY_DIR}
       ${PROJECT_SOURCE_DIR}/tools/get_compiler_standard.cpp
       RUN_OUTPUT_VARIABLE cxx_standard
       )

   set(${variable} ${cxx_standard} PARENT_SCOPE)

endfunction()

# -------------------------------------------------------------------------------------------
#--- GET_NAME_OF_COMPILER_STANDARD(<variable> cxx_std_code)
#
# Sets <variable> to the name of the C++ standard used by the compiler
# cxx_std_code is a value of __cplusplus such as returned by GET_COMPILER_STANDARD for the
# current compiler settings
#--------------------------------------------------------------------------------------------
function(GET_NAME_OF_COMPILER_STANDARD variable cxx_std_code)

    if(cxx_std_code LESS CXX_STANDARD_11)
        set(${variable} "C++03/C++98" PARENT_SCOPE)
    elseif(cxx_std_code LESS CXX_STANDARD_14)
        set(${variable} "C++11" PARENT_SCOPE)
    elseif(cxx_std_code LESS CXX_STANDARD_17)
        set(${variable} "C++14" PARENT_SCOPE)
    elseif(cxx_std_code EQUAL CXX_STANDARD_17)
        set(${variable} "C++17" PARENT_SCOPE)
    else()
        set(${variable} "C++20" PARENT_SCOPE)
    endif()

endfunction()
