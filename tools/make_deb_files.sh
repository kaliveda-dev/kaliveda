#!/bin/bash
#--- make_deb_files [source dir] [build dir]
#
# This script is used to update the debian/ubuntu packaging information in case
# e.g. new classes are added to the libraries.
#
# Call this script with two arguments: full paths to source & build directories
# Make sure the ROOT version used is compatible with current Ubuntu package!
# This will configure & build all libraries (option USE_ALL=yes), then perform
# a DESTDIR installation in [build dir]. This will be used to update the debian
# package information in the [source dir]/debian directory, such as:
#
#  kaliveda.dirs
#  kaliveda.install
#  libkaliveda-dev.dirs
#  libkaliveda-dev.install
# 
# etc. etc. You should then commit the new files to the version control system
# for the next build to take them into account.

makeDebFiles()
{
# $1 = root source directory
# $2 = build directory
# $3 = subproject name (KVMultiDet, KVIndra, etc)
# $4 = package name (libkaliveda etc.)

echo "makeDebFiles: analysing installation of $3 for package $4..."
version=`cat $1/VERSION | sed 's/\//./'`

# headers
source_headers=`find $1/$3 -name '*.h'`
base_source_headers=`basename -a $source_headers`
cd $2/tmp
SUBPROJ_HEADERS=""
for header in $base_source_headers; do
   insth=`find usr/include/kaliveda -name $header`
   if [ "x$insth" != "x" ]; then
      binsth=`basename $insth`
      if [ "$binsth" = "$header" ]; then
         SUBPROJ_HEADERS="$SUBPROJ_HEADERS $header"
      fi
   fi
done
if [ "x$3" = "xKVMultiDet" ]; then
  SUBPROJ_HEADERS="$SUBPROJ_HEADERS KVConfig.h KVVersion.h"
fi

# libraries
# .so symlinks + rootmaps => dev package
devlibs=`find usr/lib -name lib$3'*'.so`
rootmaps=`find usr/lib -name lib$3'*'.rootmap`
allibs=`find usr/lib -name lib$3'*' | grep -v pcm | grep -v rootmap`
# remove from allibs any lib which is in devlibs
libs=""
for dlib in $allibs; do
   good="yes"
   for lib in $devlibs; do
      if [ "$dlib" = "$lib" ]; then
         good="no"
      fi
   done
   if [ "x$good" = "xyes" ]; then
      libs="$libs $dlib"
   fi
done

# data
datafiles=""
[ -d $1/$3/data ] && datafiles=`ls $1/$3/data`

# templates
tmplfiles=""
[ -d $1/$3/factory ] && tmplfiles=`ls $1/$3/factory`

# examples
exmplfiles=""
[ -d $1/$3/examples ] && exmplfiles=`ls $1/$3/examples`

# etc
etcfiles=""
[ -d $1/$3/etc ] && etcfiles=`ls $1/$3/etc`

# cmake helper files
cmakefiles=`find usr/lib -name '*.cmake'`

# dataset directories
src_datasets=`find $1/$3 -name Runlist.csv`
dataset_files=""
if [ "x$src_datasets" != "x" ]; then
   # names of source dataset directories
   tmp1=`dirname $src_datasets`
   dataset_dirs=`basename -a $tmp1`
   # look for installed versions
   for ds in $dataset_dirs; do
      # look for installed directory
      tmp1=`find usr -name $ds`
      if [ "x$tmp1" != "x" ]; then
         dataset_files="$dataset_files `find $tmp1 -type f`"
      fi
   done
fi

# write files
cd $1
tool_install_file=$1/debian/$4.install
install_file=$1/debian/lib$4.install
dev_install_file=$1/debian/lib$4-dev.install
tool_dirs_file=$1/debian/$4.dirs
dirs_file=$1/debian/lib$4.dirs
dev_dirs_file=$1/debian/lib$4-dev.dirs
rm -f $tool_install_file $tool_dirs_file $install_file $dev_install_file $dirs_file $dev_dirs_file

echo "makeDebFiles: updating $install_file $dev_install_file..."
for file in $datafiles; do
   echo "/usr/share/kaliveda/data/$file" >> $install_file
done
for tmpl in $tmplfiles; do
   echo "/usr/share/kaliveda/templates/$tmpl" >> $install_file
done
for etc in $etcfiles; do
   echo "/usr/share/kaliveda/etc/$etc" >> $install_file
done
for exmpl in $exmplfiles; do
   echo "/usr/share/doc/kaliveda/examples/$3/$exmpl" >> $dev_install_file
done
for lib in $devlibs; do
   echo "/$lib" >> $dev_install_file
done
for rmap in $rootmaps; do
   echo "/$rmap" >> $dev_install_file
done
for header in $SUBPROJ_HEADERS; do
   echo "/usr/include/kaliveda/$header" >> $dev_install_file
done
for lib in $libs; do
   echo "/$lib" >> $install_file
done
if [ "x$3" = "xKVMultiDet" ]; then
  extras="/usr/share/doc/kaliveda/README.md /usr/share/doc/kaliveda/COPYING /usr/share/doc/kaliveda/INSTALL /usr/share/kaliveda/etc/config.files /usr/share/kaliveda/etc/KaliVeda.par /usr/lib/libfitltg.so /usr/lib/libgan_tape.so"
  tools="/usr/bin/kaliveda /usr/bin/kaliveda-sim /usr/bin/update_runlist /usr/bin/kvtreeanalyzer /usr/bin/kvdatanalyser /usr/bin/KaliVedaAnalysis"
  dev_extras="/usr/share/kaliveda/etc/nedit.cf /usr/bin/kaliveda-config"
  for e in $extras; do
     echo "$e" >> $install_file
  done
   echo "makeDebFiles: updating $tool_install_file..."
  for t in $tools; do
     echo "$t" >> $tool_install_file
  done
  for e in $dev_extras; do
     echo "$e" >> $dev_install_file
  done
  for e in $cmakefiles; do
     echo "/$e" >> $dev_install_file
  done
elif [ "x$3" = "xKVIndra" ]; then
  tools="/usr/bin/KaliVeda /usr/bin/KaliVedaGUI /usr/bin/KVDataBaseGUI"
   echo "makeDebFiles: updating $tool_install_file..."
  for t in $tools; do
     echo "$t" >> $tool_install_file
  done
fi
if [ "x$dataset_files" != "x" ]; then
   for file in $dataset_files; do
      echo "/$file" >> $install_file
   done
fi
# make '.dirs' files
   echo "makeDebFiles: updating .dirs files..."
dirname `cat $install_file` | sort -u > $dirs_file
dirname `cat $dev_install_file` | sort -u > $dev_dirs_file
[ -f $tool_install_file ] && dirname `cat $tool_install_file` | sort -u > $tool_dirs_file
   echo "makeDebFiles: done"
}

if [ $# -lt 2 ]; then
   echo "`basename $0` [source dir] [build dir]"
   exit 0
fi

read -p "Delete build directory $2 ? (y/[n]): " reply
reply=${reply:-n}
if [ "x$reply" = "xy" ]; then
   rm -rf $2
   mkdir -p $2
fi
cd $2
cmake $1 -DCMAKE_INSTALL_PREFIX=/usr -Dgnuinstall=yes -DUSE_MICROSTAT=yes -DUSE_ALL=yes -DUSE_GEMINI=no -DUSE_SQLITE=no
if [ $? -ne 0 ]; then
   exit 1
fi
make -j4 install DESTDIR=$2/tmp
if [ $? -ne 0 ]; then
   exit 1
fi
makeDebFiles $1 $2 KVMultiDet kaliveda
makeDebFiles $1 $2 KVIndra kaliveda-indra
makeDebFiles $1 $2 VAMOS kaliveda-indravamos
makeDebFiles $1 $2 FAZIA kaliveda-fazia
makeDebFiles $1 $2 MicroStat kaliveda-microstat
