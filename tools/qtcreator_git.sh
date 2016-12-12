#!/bin/sh
# Set up QTCreator project files:
# fill kaliveda.files with all source files under git control
# fill kaliveda.includes with output of root-config --incdir and all
#   directories under git control

git ls-files --exclude-standard | \egrep '\.h$|\.hh$|\.H$|\.for$|\.f$|\.rootrc$|\.cpp$|\.c$|\.cxx$|\.C$|\.sh$|\.csh$|\.m4$|\.cmake$|\.in$|CMakeLists.txt' > kaliveda.files
git ls-files --exclude-standard debian >> kaliveda.files
rm -rf .root-headers
echo '.' > kaliveda.includes
dirname `find . -name '*.h'` | sort -u > kaliveda.includes

# to fix a bug in QtCreator 3.0, copy all ROOT headers used by KV sources to local directory
echo '.root-headers' >> kaliveda.includes

mkdir -p .root-headers

HEADERLIST=`\grep '^#include [",<][R,T].*\.h[",>]' */*/*.h | awk '{print $2}' | sed 's/["<]//' | sed 's/[">]//' | sort | uniq`;
for h in $HEADERLIST; do
	cp -u `root-config --incdir`/$h .root-headers/;
done

HEADERLIST=`\grep '^#include [",<][R,T].*\.h[",>]' */*/*.cpp | awk '{print $2}' | sed 's/["<]//' | sed 's/[">]//' | sort | uniq`;
for h in $HEADERLIST; do
	cp -u `root-config --incdir`/$h .root-headers/;
done

HEADERLIST=`\grep '^#include [",<][R,T].*\.h[",>]' execs/*.c* | awk '{print $2}' | sed 's/["<]//' | sed 's/[">]//' | sort | uniq`;
for h in $HEADERLIST; do
	cp -u `root-config --incdir`/$h .root-headers/;
done

