# Source this script to set up the KaliVeda build that this script is part of.
#
# Conveniently an alias like this can be defined in ~/.cshrc:
#   alias thisKaliVeda "source bin/thiskaliveda.sh"
#
# This script if for the csh like shells, see thiskaliveda.sh for bash like shells.
#
# Author: Fons Rademakers, 18/8/2006

if ($?KVROOT) then
   set old_KVROOT="$KVROOT"
endif

# $_ should be source .../thiskaliveda.csh
set ARGS=($_)
if ("$ARGS" != "") then
   set thiskaliveda="`dirname ${ARGS[2]}`"
else
   # But $_ might not be set if the script is source non-interactively.
   # In [t]csh the sourced file is inserted 'in place' inside the
   # outer script, so we need an external source of information
   # either via the current directory or an extra parameter.
   if ( -e thiskaliveda.csh ) then
      set thiskaliveda=${PWD}
   else if ( -e bin/thiskaliveda.csh ) then
      set thiskaliveda=${PWD}/bin
   else if ( "$1" != "" ) then
      if ( -e ${1}/bin/thiskaliveda.csh ) then
         set thiskaliveda=${1}/bin
      else if ( -e ${1}/thiskaliveda.csh ) then
         set thiskaliveda=${1}
      else
         echo "thiskaliveda.csh: ${1} does not contain a KaliVeda installation"
      endif
   else
      echo 'Error: The call to "source where_kaliveda_is/bin/thiskaliveda.csh" can not determine the location of the KaliVeda installation'
      echo "because it was embedded another script (this is an issue specific to csh)."
      echo "Use either:"
      echo "   cd where_kaliveda_is; source bin/thiskaliveda.csh"
      echo "or"
      echo "   source where_kaliveda_is/bin/thiskaliveda.csh where_kaliveda_is"
   endif
endif

if ($?thiskaliveda) then

setenv KVROOT "`(cd ${thiskaliveda}/..;pwd)`"

# is it an old-style or a GNU-style install ?
set new_mandir="share/man"
if ( -d ${KVROOT}/KVFiles ) then
   set new_mandir="man"
endif
if ($?old_KVROOT) then
   # was it an old-style or a GNU-style install ?
   set old_mandir="share/man"
   if ( -d ${old_KVROOT}/KVFiles ) then
      set old_mandir="man"
   endif
   setenv PATH `echo $PATH | sed -e "s;:$old_KVROOT/bin:;:;g" \
                                 -e "s;:$old_KVROOT/bin;;g"   \
                                 -e "s;$old_KVROOT/bin:;;g"   \
                                 -e "s;$old_KVROOT/bin;;g"`
   if ($?LD_LIBRARY_PATH) then
      setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
                             sed -e "s;:$old_KVROOT/lib:;:;g" \
                                 -e "s;:$old_KVROOT/lib;;g"   \
                                 -e "s;$old_KVROOT/lib:;;g"   \
                                 -e "s;$old_KVROOT/lib;;g"`
   endif
   if ($?MANPATH) then
      setenv MANPATH `echo $MANPATH | \
                             sed -e "s;:$old_KVROOT/${old_mandir}:;:;g" \
                                 -e "s;:$old_KVROOT/${old_mandir};;g"   \
                                 -e "s;$old_KVROOT/${old_mandir}:;;g"   \
                                 -e "s;$old_KVROOT/${old_mandir};;g"`
   endif
endif

if ($?MANPATH) then
# Nothing to do
else
   # Grab the default man path before setting the path to avoid duplicates
   if ( -X manpath ) then
      set default_manpath = `manpath`
   else
      set default_manpath = `man -w`
   endif
endif

set path = ($KVROOT/bin $path)

if ($?LD_LIBRARY_PATH) then
   setenv LD_LIBRARY_PATH $KVROOT/lib:$LD_LIBRARY_PATH      # Linux, ELF HP-UX
else
   setenv LD_LIBRARY_PATH $KVROOT/lib
endif

if ($?MANPATH) then
   setenv MANPATH `dirname $KVROOT/${new_mandir}/man1`:$MANPATH
else
   setenv MANPATH `dirname $KVROOT/${new_mandir}/man1`:$default_manpath
endif

# for ROOT6: need to set ROOT_INCLUDE_PATH environment variable
# otherwise class dictionaries will not load correctly
if ($?ROOT_INCLUDE_PATH) then
   setenv ROOT_INCLUDE_PATH `$KVROOT/bin/kaliveda-config --incdir`:$ROOT_INCLUDE_PATH
else
   setenv ROOT_INCLUDE_PATH `$KVROOT/bin/kaliveda-config --incdir`
endif

endif # if ("$thiskaliveda" != "")

set thiskaliveda=
set old_mandir=
set new_mandir=
set old_KVROOT=

