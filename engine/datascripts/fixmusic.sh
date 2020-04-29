#!/bin/sh
DOTVS=".vegastrike"
if test -e ../Version.txt; then
DOTVS=`cat ../Version.txt`
fi
find ../$DOTVS -name "*.m3u" -exec sh -c 'sed -e s/.ogg/.mid/g {} > {}.bak'  \;
find ../$DOTVS -name "*.m3u" -exec sh -c 'sed -e s/.mid/.ogg/g {} > {}.fore'  \;
find ../$DOTVS -name "*.m3u" -exec mv {}.bak {} \;
for i in $1; do
    find ../$DOTVS -name "*.m3u" -exec mv {}.fore {} \;
done
rm -f ../$DOTVS/*.fore