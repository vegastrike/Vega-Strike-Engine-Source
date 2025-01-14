#!/usr/bin/env python3
##
# add_gpl_license.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
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
#

"""This script takes a file path (or paths) as an argument, and rewrites the file
with the Vega Strike GPL 3 copyright notice prepended to it.

If no arguments are passed, the script will read from stdin.

The Copyright text is copied from the LICENSE file, which is assumed to be at
the root of the current git repository. Hence, this script needs to be run from
within a git repository containing a LICENSE file at the root. The lines at the
start of the LICENSE file, starting with 'Copyright (C)' and ending just before
'GNU GENERAL PUBLIC LICENSE', will be copied as the Copyright text for the GPL
notice. e.g. If the LICENSE file contains the following lines, they will be copied
in at the start of the GPL notice:

    Copyright (c) 2001-2002 Daniel Horn
    Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
    Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors

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
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Vega Strike is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>."""

# Characters to use for the start, middle and end of a comment block
C_LIKE_COMMENT = ['/*', ' *', ' */', '/**', '*', '*/']
SCRIPT_LIKE_COMMENT = ['##', '#', '#']

COMMENTS_BY_FILE_SUFFIX = {
    '.c': C_LIKE_COMMENT,
    '.cpp': C_LIKE_COMMENT,
    '.h': C_LIKE_COMMENT,
    '.hpp': C_LIKE_COMMENT,
    '.h.in': C_LIKE_COMMENT,
    '.py': SCRIPT_LIKE_COMMENT,
    '.cmake': SCRIPT_LIKE_COMMENT,
    '.txt': SCRIPT_LIKE_COMMENT,
    '.sh': SCRIPT_LIKE_COMMENT,
    '.ps1': SCRIPT_LIKE_COMMENT,
}


def find_git_root() -> Path:
    """If we're in a git repository, return the path to the git root directory.
    Otherwise, raise an error."""

    pwd: Path = Path.cwd()
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
    """Return lines starting with 'Copyright (c)' from the start of a given
    LICENSE file. Stop reading the file after the license title is found:
        GNU GENERAL PUBLIC LICENSE
    """
    copyright_lines: list[str] = []
    with LICENSE.open('r') as file:
        for line in file:
            if 'GNU GENERAL PUBLIC LICENSE' in line:
                break
            copyright_lines.append(line)

    return ''.join(copyright_lines).removesuffix('\n')


def is_a_comment(line: str, comment_prefixes: list[str]) -> bool:
    for prefix in comment_prefixes:
        if line.startswith(prefix):
            return True
    return False


def remove_license_header(filepath: Path) -> None:
    """Remove the GPL license notice at the start of the given file, if present."""
    print(f"Removing license header from {filepath.name}")

    suffix: str = ''.join(filepath.suffixes)
    comment_type: list[str] = COMMENTS_BY_FILE_SUFFIX[suffix]

    with NamedTemporaryFile('w', delete=False, newline='\n') as output_file:
        with filepath.open('r') as input_file:
            first_line: str = input_file.readline()

            # If first line is a shebang, leave it intact
            if first_line.startswith('#!'):
                output_file.write(first_line)

            # Handle the line that a few of our C++ source files have at or near the top that looks like this:
            # // -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
            elif first_line.startswith('// -*- '):
                output_file.write(first_line)

            # If first line isn't a comment, then short-circuit this whole process. There is no comment block to delete
            elif not is_a_comment(first_line, comment_type):
                output_file.close()
                Path.unlink(Path(output_file.name))
                return

            found_gpl: bool = False

            # Remove the initial comment block at the start of the file.
            while True:
                line: str = input_file.readline()

                # Check for GPL text snippet in each line in the initial comment block.
                if 'GENERAL PUBLIC LICENSE' in line or 'General Public License' in line:
                    found_gpl = True

                if not line:
                    # We've reached the end of the initial comment block
                    break

                if not is_a_comment(line, comment_type):
                    # We've reached the end of the initial comment block
                    output_file.write(line)
                    break

            # Only remove the copyright block if it contained a reference to the GPL.
            # Otherwise, short-circuit the rest of this process
            if not found_gpl:
                output_file.close()
                Path.unlink(Path(output_file.name))
                return

            output_file.write(input_file.read())

    # Copy original file attributes and permissions to temp file
    copystat(filepath, output_file.name)
    # Move temp file into place
    move(output_file.name, filepath)

def add_gpl_license(filepath: Path, license_path: Path) -> None:
    """Add a GPL license notice to the start of the given file.
    This function tries to choose sensible comment characters based on the file
    extension of the given file. It also is smart enough to add the license
    notice AFTER the shebang line, if one is detected.

    However, it doesn't check if there is already a license notice in the file.
    Any file passed to this function will have a license notice added, even if
    it already has a license notice..."""

    print(f"Adding license header to {filepath.name}")

    # Construct comment for filetype
    suffix: str = ''.join(filepath.suffixes)
    comment_type: list[str] = COMMENTS_BY_FILE_SUFFIX[suffix]
    license_block: str = LICENSE_TEXT.format(
        filename=filepath.name,
        copyright_notice=get_copyright_notice(license_path))

    comment_block: list[str] = [f'{comment_type[1]} {line}' if line else comment_type[1]
                                for line in license_block.split('\n')]

    comment: str = '\n'.join([comment_type[0], *comment_block, comment_type[2]]) + '\n'

    # Use binary mode for files to avoid encoding issues
    with NamedTemporaryFile('w', delete=False, newline='\n') as output_file:
        with filepath.open('r') as input_file:
            first_line: str = input_file.readline()
            # If first line is a shebang, insert comment after first line.
            if first_line.startswith('#!'):
                output_file.write(first_line)
                output_file.write(comment)
                output_file.write(input_file.read())
            # Else insert comment as first line
            else:
                output_file.write(comment)
                output_file.write(first_line)
                output_file.write(input_file.read())

    # Copy original file attributes and permissions to temp file
    copystat(filepath, output_file.name)
    # Move temp file into place
    move(output_file.name, filepath)


def main():
    license_path: Path = find_git_root()/'LICENSE'
    if len(sys.argv) > 1 and sys.argv[1] in ('-h', '--help'):
        print(__doc__)
        return
    elif len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        files = sys.stdin

    for filepath in map(Path, (f.removesuffix('\n') for f in files)):
        remove_license_header(filepath)
        add_gpl_license(filepath, license_path)

if __name__ == '__main__':
    main()
