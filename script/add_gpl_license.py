#!/usr/bin/env python3

# add_gpl_license.py
# 
# Copyright (C) 2021 David Wales and other Vega Strike
# contributors
# 
# https://github.com/vegastrike/Vega-Strike-Engine-Source
# 
# This file is part of Vega Strike.
# 
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.

"""This script takes a file path (or paths) as an argument, and rewrites the file
with the Vega Strike GPL 3 copyright notice prepended to it.

If no arguments are passed, the script will read from stdin.

Usage:
    python3 add_gpl_license.py [-h|--help]
    python3 add_gpl_license.py PATH [PATH ...]
"""

import sys
from pathlib import Path
from shutil import copystat, move
from tempfile import NamedTemporaryFile

LICENSE_TEXT = """{filename}

Copyright (C) 2001-2002 Daniel Horn
Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
contributors

https://github.com/vegastrike/Vega-Strike-Engine-Source

This file is part of Vega Strike.

Vega Strike is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Vega Strike is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Vega Strike. If not, see <https://www.gnu.org/licenses/>."""

# Characters to use for the start, middle and end of a comment block
C_LIKE_COMMENT = ('/**', '*', '*/')
SCRIPT_LIKE_COMMENT = ('##', '#', '#')

COMMENTS_BY_FILE_SUFFIX = {
    '.c': C_LIKE_COMMENT,
    '.cpp': C_LIKE_COMMENT,
    '.h': C_LIKE_COMMENT,
    '.hpp': C_LIKE_COMMENT,
    '.in': C_LIKE_COMMENT,
    '.py': SCRIPT_LIKE_COMMENT,
    '.cmake': SCRIPT_LIKE_COMMENT,
    '.txt': SCRIPT_LIKE_COMMENT,
    '.sh': SCRIPT_LIKE_COMMENT,
    '.ps1': SCRIPT_LIKE_COMMENT,
}

def add_gpl_license(filepath: Path):
    print(filepath.name)
    print(filepath.suffix)

    # Construct comment for filetype
    comment_type = COMMENTS_BY_FILE_SUFFIX[filepath.suffix]
    license_block = LICENSE_TEXT.format(filename=filepath.name)

    comment_block = [f'{comment_type[1]} {line}' if line else comment_type[1]
            for line in license_block.split('\n')]

    comment = '\n'.join([comment_type[0], *comment_block, comment_type[2]]) + '\n\n'

    # Use binary mode for files to avoid encoding issues
    with NamedTemporaryFile('w+b', delete=False) as output_file:
        output_filepath = output_file.name
        with filepath.open('r+b') as input_file:
            first_line = input_file.readline()
            # If first line is a shebang, insert comment after first line.
            if first_line.startswith('#!'.encode()):
                output_file.write(first_line)
                output_file.write('\n'.encode())
                output_file.write(comment.encode())
                output_file.write(input_file.read())
            # Else insert comment as first line
            else:
                output_file.write(comment.encode())
                output_file.write(first_line)
                output_file.write(input_file.read())

    # Copy original file attributes and permissions to temp file
    copystat(filepath, output_file.name)
    # Move temp file into place
    move(output_file.name, filepath)

def main():
    if len(sys.argv) > 1:
        if sys.argv[1] in ('-h', '--help'):
            print(__doc__)
        else:
            for filepath in map(Path, sys.argv[1:]):
                add_gpl_license(filepath)
    else:
        for filepath in map(Path, (f.removesuffix('\n') for f in sys.stdin)):
            add_gpl_license(filepath)

if __name__ == '__main__':
    main()
