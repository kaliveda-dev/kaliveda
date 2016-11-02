# Source this script to set up the KaliVeda build that this script is part of.
#
# Conveniently an alias like this can be defined in .bashrc:
#   alias thiskaliveda=". bin/thiskaliveda.sh"
#
# This script is for the bash like shells, see thiskaliveda.csh for csh like shells.
#
# Author: Fons Rademakers, 18/8/2006

drop_from_path()
{
   # Assert that we got enough arguments
   if test $# -ne 2 ; then
      echo "drop_from_path: needs 2 arguments"
      return 1
   fi

   p=$1
   drop=$2

   newpath=`echo $p | sed -e "s;:${drop}:;:;g" \
                          -e "s;:${drop};;g"   \
                          -e "s;${drop}:;;g"   \
                          -e "s;${drop};;g"`
}

if [ -n "${KVROOT}" ] ; then
   old_KVROOT=${KVROOT}
fi

if [ "x${BASH_ARGV[0]}" = "x" ]; then
    if [ ! -f bin/thiskaliveda.sh ]; then
        echo ERROR: must "cd where/kaliveda/is" before calling ". bin/thiskaliveda.sh" for this version of bash!
        KVROOT=; export KVROOT
        return 1
    fi
    KVROOT="$PWD"; export KVROOT
else
    # get param to "."
    thiskaliveda=$(dirname ${BASH_ARGV[0]})
    KVROOT=$(cd ${thiskaliveda}/..;pwd); export KVROOT
fi

# is it an old-style or a GNU-style install ?
new_mandir="share/man"
if [ -d "${KVROOT}/KVFiles" ]; then
   new_mandir="man"
fi
if [ -n "${old_KVROOT}" ] ; then
   # was it an old-style or a GNU-style install ?
   old_mandir="share/man"
   if [ -d "${old_KVROOT}/KVFiles" ]; then
      old_mandir="man"
   fi
   if [ -n "${PATH}" ]; then
      drop_from_path "$PATH" ${old_KVROOT}/bin
      PATH=$newpath
   fi
   if [ -n "${LD_LIBRARY_PATH}" ]; then
      drop_from_path $LD_LIBRARY_PATH ${old_KVROOT}/lib
      LD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${MANPATH}" ]; then
      drop_from_path $MANPATH ${old_KVROOT}/${old_mandir}
      MANPATH=$newpath
   fi
fi

if [ -z "${MANPATH}" ]; then
   # Grab the default man path before setting the path to avoid duplicates
   if `which manpath > /dev/null 2>&1` ; then
      default_manpath=`manpath`
   else
      default_manpath=`man -w 2> /dev/null`
   fi
fi

if [ -z "${PATH}" ]; then
   PATH=$KVROOT/bin; export PATH
else
   PATH=$KVROOT/bin:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$KVROOT/lib; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH=$KVROOT/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${MANPATH}" ]; then
   MANPATH=`dirname $KVROOT/${new_mandir}/man1`:${default_manpath}; export MANPATH
else
   MANPATH=`dirname $KVROOT/${new_mandir}/man1`:$MANPATH; export MANPATH
fi

# for ROOT6: need to set ROOT_INCLUDE_PATH environment variable
# otherwise class dictionaries will not load correctly
if [ -z "${ROOT_INCLUDE_PATH}" ]; then
   ROOT_INCLUDE_PATH=`$KVROOT/bin/kaliveda-config --incdir`; export ROOT_INCLUDE_PATH
else
   ROOT_INCLUDE_PATH=`$KVROOT/bin/kaliveda-config --incdir`:$ROOT_INCLUDE_PATH; export ROOT_INCLUDE_PATH
fi

unset old_KVROOT
unset thiskaliveda

