#!/bin/sh
perl replace.pl z=\" G=\"- $1 *.xmesh
perl replace.pl y=\" z=\" $1 *.xmesh
perl replace.pl G=\" y=\" $1 *.xmesh
perl replace.pl k=\" G=\"- $1 *.xmesh
perl replace.pl j=\" k=\" $1 *.xmesh
perl replace.pl G=\" j=\" $1 *.xmesh
perl replace.pl \"-- \" $1 *.xmesh