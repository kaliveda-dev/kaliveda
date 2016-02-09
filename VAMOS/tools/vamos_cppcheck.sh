#!/bin/bash
#
# This script runs the cppcheck static analysis program on the code contained
# within the specified directories.
#
# NOTE: Not all warnings are errors and they should each be carefully
# considered.
#
# Ignore the warning about missing headers, otherwise you'll be analysing all of
# kaliveda and ROOT (not our principal concern).
#
# Author: Peter Wigg (peter.wigg.314159@gmail.com)

logfile=/tmp/vamos_cppcheck.log

function msg()
{
  echo "$@" | tee -a "${logfile}"
}

function check_directory()
{
  local directory="${1}"

  msg ""
  msg "***********************"
  msg "*** CHECKING DIRECTORY: ${directory}"
  msg "***********************"
  msg ""

  # NDEBUG Defined ('Release' build)
  cppcheck \
    -DNDEBUG \
    --enable=all \
    "${directory}/*.cxx" \
    "${directory}/*.cpp" \
    "${directory}/*.h" \
    "${directory}/*.hpp" \
    2>&1 | tee -a "${logfile}"
}

if [ $# -eq 1 ]; then
  # Single directory requested (rather than the whole VAMOS code)
  check_directory "${1}"
  exit 0
fi

cat > "${logfile}" << EOF
Cppcheck log file for VAMOS.
Started @ $(date)
EOF

directories="exp_events \
  daq_cec \
  identification \
  geometry \
  calibration \
  db \
  factory \
  analysis"

for d in ${directories}; do
  check_directory "${d}"
done



