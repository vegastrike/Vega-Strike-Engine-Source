#!/usr/bin/env python3

# add_gpl_license.py
#
# Copyright (C) 2021-2022 David Wales, Stephen G. Tuggy, and other 
# Vega Strike contributors
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

The Copyright text is copied from the LICENSE file, which is assumed to be at
the root of the current git repository. Hence this script needs to be run from
within a git repository containing a LICENSE file at the root. Lines
containing the text 'Copyright (C)' at the start of the LICENSE file will be
copied as the Copyright text for the GPL notice. e.g. If the LICENSE file
contains the following lines, they will be copied in at the start of the GPL
notice:

    Copyright (C) 2001-2002 Daniel Horn
    Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
    Copyright (C) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors

Usage:
    python3 add_gpl_license.py [-h|--help]
    python3 add_gpl_license.py PATH [PATH ...]
"""

import sys
from pathlib import Path
from shutil import copystat, move
from tempfile import NamedTemporaryFile

LICENSE_TEXT = """{filename}

{copyright_notice}

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
    '.h.in': C_LIKE_COMMENT,
    '.tab.cpp.h': C_LIKE_COMMENT,
    '.tab.cpp': C_LIKE_COMMENT,
    '.yy.cpp': C_LIKE_COMMENT,
    '.py': SCRIPT_LIKE_COMMENT,
    '.cmake': SCRIPT_LIKE_COMMENT,
    '.deprecated.cmake': SCRIPT_LIKE_COMMENT,
    '.txt': SCRIPT_LIKE_COMMENT,
    '.sh': SCRIPT_LIKE_COMMENT,
    '.x11.sh': SCRIPT_LIKE_COMMENT,
    '.ps1': SCRIPT_LIKE_COMMENT,
}


def find_git_root() -> Path:
    """If we're in a git repository, return the path to the git root directory.
    Otherwise, raise an error."""

    pwd = Path.cwd()
    # Check if we're already at the git root
    if (pwd/'.git').exists():
        return pwd
    # Else iterate through the parent directories until we find the git root,
    # or we hit the filesystem root directory.
    else:
        for directory in pwd.parents:
            if (directory/'.git').exists():
                return directory
        raise FileNotFoundError('Not a git repository!')


def get_copyright_notice(LICENSE: Path) -> str:
    """Return lines starting with 'Copyright (C)' from the start of a given
    LICENSE file. Stop reading the file after the license title is found:
        GNU GENERAL PUBLIC LICENSE
    """
    copyright_lines = []
    with LICENSE.open() as file:
        for line in file:
            if line.startswith('Copyright (C)'):
                copyright_lines.append(line)
            elif 'GNU GENERAL PUBLIC LICENSE' in line:
                break

    return ''.join(copyright_lines).removesuffix('\n')


def add_gpl_license(filepath: Path, license_path: Path) -> None:
    """Add a GPL license notice to the start of the given file.
    This function tries to choose sensible comment characters based on the file
    extension of the given file. It also is smart enough to add the license
    notice AFTER the shebang line, if one is detected.

    However, it doesn't check if there is already a license notice in the file.
    Any file passed to this function will have a license notice added, even if
    it already has a license notice already..."""

    print(filepath.name)

    # Construct comment for filetype
    suffix = ''.join(filepath.suffixes)
    comment_type = COMMENTS_BY_FILE_SUFFIX[suffix]
    license_block = LICENSE_TEXT.format(
            filename=filepath.name,
            copyright_notice=get_copyright_notice(license_path))

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
    license_path = find_git_root()/'LICENSE'
    if len(sys.argv) > 1 and sys.argv[1] in ('-h', '--help'):
        print(__doc__)
    elif len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        files = sys.stdin

    for filepath in map(Path, (f.removesuffix('\n') for f in files)):
        add_gpl_license(filepath, license_path)

if __name__ == '__main__':
    main()
