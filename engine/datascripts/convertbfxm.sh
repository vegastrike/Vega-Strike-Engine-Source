#!/bin/sh
mesher $1 0_0.xmesh bxc
for i in *.xmesh; do
replace usenormals=\"0\"  usenormals=\"1\" $i $i
done
rm $1
for i in *_0.xmesh; do
if [ "$i" = "0_0.xmesh" ]; then
mesher $i $1 xbc
else
mesher $i $1 xba
fi

done
rm *.xmesh
