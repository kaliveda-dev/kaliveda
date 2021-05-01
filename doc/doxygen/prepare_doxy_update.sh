# Set up directory in order to update doxygen class documentation
# Run this script in an empty directory like so:
#
#    [path to source directory]/doc/doxygen prepare_doxy_update.sh
#
KV_SOURCE=$(kaliveda-config --srcdir)
if [ ! -d ./kaliveda.git ]; then
   ln -s $KV_SOURCE kaliveda.git
fi

mkdir -p ./html

# link scripts etc.
list=$(ls $KV_SOURCE/doc/doxygen)
for f in $list; do
   if [ ! -L $f ]; then
      ln -s $KV_SOURCE/doc/doxygen/$f
   fi
done
