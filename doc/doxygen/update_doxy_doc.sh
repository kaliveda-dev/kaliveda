# - Run script to update local doxygen doc in html directory

DOXYFILE=Doxyfile

# reformat comments
kaliveda -b -q

export KALIVEDA_VERSION=$(kaliveda-config --version)

# main page
cp kaliveda.git/README.md kaliveda.doxygen/

# release notes
cp kaliveda.git/doc/doxygen/release_notes.md kaliveda.doxygen/

# documentation for each module (and definition of groups!)
cp kaliveda.git/*/*/doc/* kaliveda.doxygen/

# KVError.H
cp kaliveda.git/KVMultiDet/base/KVError.h kaliveda.doxygen/KVMultiDet/base/

# download ROOT tag file to link ROOT documentation
if [ ! -f ./ROOT.tag ]; then
   wget https://root.cern/doc/master/ROOT.tag
fi

export INPUT_DIR=kaliveda.doxygen && doxygen $DOXYFILE
