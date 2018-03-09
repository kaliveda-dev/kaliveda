# - update doxygen doc for branch $BRANCH,
#   push it up to gh-pages branch (overwriting history)
# need write access to gh-pages branch

BRANCH=1.10
DOXYFILE=Doxyfile

if [ ! -d ./kaliveda.git ]; then
   git clone --single-branch -b $BRANCH git@github.com:kaliveda-dev/kaliveda.git kaliveda.git
else
   cd ./kaliveda.git
   git pull
   cd ..
fi

if [ ! -d ./html ]; then
   git clone --single-branch -b gh-pages git@github.com:kaliveda-dev/kaliveda.git html
fi

# remove history from gh-pages
cd html
git reset --hard $(git rev-list --max-parents=0 --abbrev-commit HEAD)

# reformat comments
kaliveda -b -q

export KALIVEDA_VERSION=$(kaliveda-config --version)
cp kaliveda.git/README.md kaliveda.doxygen/
export INPUT_DIR=kaliveda.doxygen && doxygen $DOXYFILE

# commit and push
cd html
git add .
git commit -m "Doxygen update"
git push -f origin gh-pages
cd ..
