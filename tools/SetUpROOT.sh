#!/bin/bash
###############################################################################
#BASH script for changing between different versions of ROOT on different machines,
#taking into account changes of (gcc) compiler version.
#Sets the environment variables ROOTSYS, PATH and LD_LIBRARY_PATH.
#
#Put SetUpROOT.sh somewhere in your PATH
#Add the following 'alias' to your .cshrc:
#alias SetROOTVersion='source `which SetUpROOT.csh`'
#
#To make sure that your shell is set up correctly to run ROOT,
#put the following line in your .cshrc:
#
#SetROOTVersion &> /dev/null
#
#If you want to change versions of ROOT during an interactive session:
#
#[user@localhost]> SetROOTVersion [version] [prefix]
#
#       Example:  SetROOTVersion v5.15.06
#
#       if gcc version = 4.0.2 this will set ROOTSYS = $HOME/.root-v5.15.06_gcc-4.0.2
#       if the directory does not exist, it will be created
#
#       Example:  SetROOTVersion v5.15.06 /space/franklan
#
#       if gcc version = 4.0.2 this will set ROOTSYS = /space/franklan/.root-v5.15.06_gcc-4.0.2
#       if the directory does not exist, it will be created
###############################################################################
#
#                   read arguments
#
VERSION=""
PREFIX=""
NEED_TO_UPDATE=0

if [ $# ]; then
   VERSION=$1
   NEED_TO_UPDATE=1
   shift
fi

if [ $# ]; then
   PREFIX=$1
   shift
fi
#
#                   handle version
#
VERS_FILE=$HOME/.rootsys
ROOTVER="root"

if [ "$VERSION" == "" ]; then
	if [[ -ef $VERS_FILE && !( -z $VERS_FILE ) ]]; then
	    VERSION=`cat $VERS_FILE | awk '{ print $1 }'`
        
        if [ "$VERSION" == "standard" ]; then
            VERSION=""
        fi 
 
        if [ "$VERSION" != "" ]; then
	        echo "Using ROOT version $VERSION"
            ROOTVER="root-$VERSION"
        fi 
    fi
else
    echo "Setting ROOT version to $VERSION"
    ROOTVER="root-$VERSION"
fi
#
#                   handle prefix
#
if [ "$PREFIX" == "" ]; then
	if [[ -ef $VERS_FILE && !( -z $VERS_FILE ) ]]; then
        PREFIX=`cat $VERS_FILE | awk '{ print $2 }'`
        if [ "${PREFIX}" != "" ]; then
	        echo "Using path : $PREFIX"
        fi 
    fi 
else
   echo "Setting path : $PREFIX"
fi

if [ "$PREFIX" == "" ]; then
   PREFIX=$HOME
   echo "Setting path : $PREFIX"
fi
#
#                   store new prefix and version
#
if [ $NEED_TO_UPDATE == 1 ]; then
    if [ "$VERSION" != "" ]; then
        echo $VERSION $PREFIX > $VERS_FILE
    else
      echo "standard $PREFIX" > $VERS_FILE
    fi 
fi
#
#                   get gcc version
#
GCCVER=`gcc -v 2>&1 |  awk '/(gcc version)/ {print $3}'`
#
#                   create installation directories if necessary
#
HOST_DIR_VER="$PREFIX/.${ROOTVER}_gcc-${GCCVER}"
#include machine type if not i386 (for backwards compatibility)
if [ $MACHTYPE ]; then
    if [ "$MACHTYPE" != "i386" ]; then
        MACHT=`echo $MACHTYPE | cut -d '-' -f1`
        HOST_DIR_VER="$PREFIX/.${ROOTVER}_gcc-${GCCVER}_${MACHT}"
    fi
fi 

HOST_DIR="$PREFIX/.${ROOTVER}"
SetUpROOTDirectories.sh $HOST_DIR_VER $HOST_DIR
#
#                   set ROOTSYS
#
export ROOTSYS="$HOST_DIR_VER"
#
#                   back up original PATH and LD_LIBRARY_PATH
#
if [ !$ROOT_PATH_BACKUP ]; then
	export ROOT_PATH_BACKUP="$PATH"

fi

if [ !$ROOT_LDPATH_BACKUP ]; then
   if [ $LD_LIBRARY_PATH ]; then
	   export ROOT_LDPATH_BACKUP="$LD_LIBRARY_PATH"
   else
      export ROOT_LDPATH_BACKUP="0"
  fi 
fi
#
#                   set paths
#
export PATH=${ROOTSYS}/bin:${ROOT_PATH_BACKUP}
if [ "$ROOT_LDPATH_BACKUP" == "0" ]; then
	   export LD_LIBRARY_PATH=${ROOTSYS}/lib
else
	   export LD_LIBRARY_PATH=${ROOTSYS}/lib:${ROOT_LDPATH_BACKUP}
fi
#if ROOT has not yet been installed (i.e. if this script was just called to create
#the necessary directories and position the paths prior to compilation of ROOT)
#then exit straight away
if [ ! -e $ROOTSYS/bin/root-config ]; then
    return 1
fi
#if SetKaliVedaVersion has previously been used to set KaliVeda
#paths and environment variables, we need to call it again (with no arguments)
#in order to reset the paths: the KaliVeda-related parts will have been
#erased by our work here.
#N.B. if the new ROOT installation has a different version to the previous one,
#this may create new Kaliveda installation directories!!!
if [[ $KV_PATH_BACKUP && -ef $HOME/.kalivedaversion && !( -z $HOME/.kalivedaversion ) ]]; then
   unset KV_PATH_BACKUP
   if [ $KV_LDPATH_BACKUP ]; then
      unset KV_LDPATH_BACKUP
   fi 

   source `$HOME/SetUpKaliVeda.sh`

else
   echo "ROOTSYS = $ROOTSYS"
   echo "PATH = $PATH"
   echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"
fi

return
