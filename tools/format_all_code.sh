#!/bin/bash
# Automatically reformat all source code under git control using astyle

ASTYLE_PARAMETERS="--suffix=none --options=./tools/astyle.rc"
FILE_PATTERN="\.(c|cpp|cxx|C|h)$"

FILES=`git ls-files --exclude-standard |  egrep $FILE_PATTERN`

if [ "x$FILES" != "x" ]; then

   ASTYLE=$(which astyle)
   if [ $? -ne 0 ]; then
	   echo "astyle reformatting:"
	   echo "astyle not installed."
	   echo "You must install astyle if you want to reformat code."
	   exit 1
   fi

   for file in $FILES; do
	   $ASTYLE $ASTYLE_PARAMETERS $file
      git add $file
   done

fi
