#!/bin/sh
perl replace.pl x=\" x=\"- $1 *.xmesh
perl replace.pl z=\" z=\"- $1 *.xmesh
perl replace.pl i=\" i=\"- $1 *.xmesh
perl replace.pl k=\" k=\"- $1 *.xmesh
perl replace.pl \"-- \" $1 *.xmesh