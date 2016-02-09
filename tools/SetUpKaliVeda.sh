#!/bin/bash
###############################################################################
#BASH shell script for installing KaliVeda on several machines with the
#same account, taking into account different versions of gcc compiler & ROOT,
#and also different versions of KaliVeda.
#This script makes sure all necessary directories exist, and sets the
#environment variables KVROOT, PATH and LD_LIBRARY_PATH.
#
#Put SetUpKaliVeda.sh and SetUpKaliVedaDirectories.sh somewhere in your PATH
#Add the following 'alias' to your .bash_profile:
#
#alias SetKaliVedaVersion 'source `which SetUpKaliVeda.csh`'
#
#To make sure that your shell is set up correctly to run or install KaliVeda,
#put the following line in your .bash_profile:
#
#SetKaliVedaVersion >& /dev/null
#
#You can optionally give a version number and an installation prefix (see below).
#
#If you want to change versions of KaliVeda during an interactive session:
#
#[user@localhost]> SetKaliVedaVersion [version]
#
###############################################################################
#Use : SetUpKaliVeda.sh [version] [prefix]
#
# with version = KaliVeda version identifier
#                default is to use version defined in previous call to SetUp..
#                or no version identifier if none was defined
#       prefix = directory in which KaliVeda directories will be created
#                default is to use user's home directory, or the last value
#                given in a call to SetUp...
#
###############################################################################
#
#                   read arguments
#
#if ROOT has not been installed, we cannot do anything - exit!
which root-config &> /dev/null
ROOTCONF=$?

if [ $ROOTCONF != 0 ]; then
   echo 'Cannot find root-config script. You cannot use KaliVeda.'
   return
fi 

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
VERS_FILE=${HOME}/.kalivedaversion
KVROOTVER="kvroot"
if [ "$VERSION" == "" ]; then
    if [[ -ef ${VERS_FILE} && ! (-z ${VERS_FILE}) ]]; then
        VERSION=`cat ${VERS_FILE} | awk '{print $1}'`
        if [ "${VERSION}" == "standard" ]; then
            VERSION=""
        fi
        if [ "$VERSION" != "" ]; then
	        echo "Using KaliVeda version $VERSION"
            KVROOTVER="kvroot-${VERSION}"
        fi
    fi
else
    echo "Setting KaliVeda version to $VERSION"
    KVROOTVER="kvroot-${VERSION}"
fi
#
#                   handle prefix
#
if [ "$PREFIX" == "" ]; then
    if [[ -ef ${VERS_FILE} && ! (-z ${VERS_FILE} ) ]]; then
        PREFIX=`cat ${VERS_FILE} | awk '{ print $2 }'`
        if [ "$PREFIX" != "" ]; then
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
if [ ${NEED_TO_UPDATE} == 1 ]; then
    if [ "${VERSION}" != "" ]; then
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
#                   get root version
#
ROOTVER=`root-config --version | sed '/\//s//./g'`
#
#                   create installation directories if necessary
#
HOST_DIR_VER="${PREFIX}/.${KVROOTVER}_gcc-${GCCVER}_root-${ROOTVER}"
#include machine type if not i386 (for backwards compatibility)
if [ $MACHTYPE ]; then
    if [ "$MACHTYPE" != "i386" ]; then
        MACHT=`echo $MACHTYPE | cut -d '-' -f1`
        HOST_DIR_VER="${PREFIX}/.${KVROOTVER}_gcc-${GCCVER}_${MACHT}_root-${ROOTVER}"
    fi
fi

HOST_DIR="${PREFIX}/.${KVROOTVER}"
SetUpKaliVedaDirectories.sh $HOST_DIR_VER $HOST_DIR
#
#                   set KVROOT
#
export KVROOT="$HOST_DIR_VER"
#
#                   back up original PATH and LD_LIBRARY_PATH
#
if [ ! $KV_PATH_BACKUP ]; then
    KV_PATH_BACKUP="$PATH"
fi
if [ ! $KV_LDPATH_BACKUP ]; then
    if [ $LD_LIBRARY_PATH ]; then
	   KV_LDPATH_BACKUP="$LD_LIBRARY_PATH"
    else
      KV_LDPATH_BACKUP="0"
    fi  
fi
#
#                   set paths
#
export PATH=${KVROOT}/bin:${KV_PATH_BACKUP}
if [ "$KV_LDPATH_BACKUP" == "0" ]; then
	   export LD_LIBRARY_PATH=${KVROOT}/lib
else
	   export LD_LIBRARY_PATH=${KVROOT}/lib:${KV_LDPATH_BACKUP}
fi
#
#                   print result
#
echo "KVROOT = $KVROOT"
echo "PATH = $PATH"
echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"
