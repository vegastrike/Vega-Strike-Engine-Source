#!/bin/bash

##
# build-authors.sh
#
# Copyright (C) 2001-2002 Daniel Horn
# Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

set -e


AUTHORS_FILE="$(pwd)/Authors"
AUTHORS_UNIQUE_FILE="${AUTHORS_FILE}.uniq"
AUTHORS_TEMP_FILE="${AUTHORS_TRACED_FILE}.temp"
AUTHORS_FINAL="$(pwd)/AUTHORS"
ATT_DEBUG="${AUTHORS_FINAL}.debug"

# clean out the old traced file
echo > ${ATT_DEBUG}

# Extract the author line from all the git commits
git log | grep Author > ${AUTHORS_FILE}

if [ -n "${GIT_REPOSITORY}" ]; then
	echo "Returning to local directory"
	popd
else
	echo "Already in local directory"
fi

# shrink the list to just the unique contributors
cat ${AUTHORS_FILE} | sort | uniq > ${AUTHORS_UNIQUE_FILE}

# Drop the git data so it's just the author data
cat ${AUTHORS_UNIQUE_FILE} | cut -f 2 -d ':' > ${AUTHORS_TEMP_FILE}

# Generate the AUTHORS file:
echo "The below is the list of all the known authors.
It has been auto-generated from the GitHub commit logs
using ./sh/build-authors.sh.
" > ${AUTHORS_FINAL}

# Generate a nice file to work with
IFS="
"
for AUTHOR in `cat ${AUTHORS_TEMP_FILE}`
do
	# Extract Username and Email
	NAME=`echo ${AUTHOR} | cut -f 1 -d '<' | tr -d ' '`
	EMAIL=`echo ${AUTHOR} | cut -f 2 -d '<' | cut -f 1 -d '>'`

	# Record to AUTHORS
	echo "* ${NAME} <${EMAIL}>" >> ${AUTHORS_FINAL}

	# Record to the debug tracer
	echo "Author: ${AUTHOR}" >> ${ATT_DEBUG}
	echo "Name: ${NAME}" >> ${ATT_DEBUG}
	echo "Email: ${EMAIL}" >> ${ATT_DEBUG}
	echo "-------------------------------------------------------------" >> ${ATT_DEBUG}

	# Record to the traced file
done

# Clean up the temp file
rm ${AUTHORS_TEMP_FILE}
rm ${AUTHORS_FILE}
rm ${AUTHORS_UNIQUE_FILE}
rm ${ATT_DEBUG}
