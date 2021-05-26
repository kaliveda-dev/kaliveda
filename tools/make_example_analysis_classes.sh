#!/bin/bash
# Run this script to generate a full set of example analysis classes.
#
# Each will be compiled, and if succesful, will be moved to the sources
#
generate_compile_example_analysis KVMultiDet INDRAFAZIA RawAnal ExampleRawAnalysis && \
generate_compile_example_analysis KVMultiDet INDRAFAZIA RawReconAnal ExampleReconRawAnalysis && \
generate_compile_example_analysis KVMultiDet INDRAFAZIA ReconAnal ExampleReconAnalysis && \
generate_compile_example_analysis KVMultiDet INDRAFAZIA SimAnal ExampleSimDataAnalysis && \
generate_compile_example_analysis KVMultiDet INDRAFAZIA FiltAnal ExampleFilteredSimDataAnalysis && \
generate_compile_example_analysis KVIndra INDRA_camp1 ReconAnal ExampleINDRAAnalysis
