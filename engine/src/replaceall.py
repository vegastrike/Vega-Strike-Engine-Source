##
# replaceall.py
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

import sys
import os
def mymangle(s):
	
	if (len(s)>3):
		return s[0:2]+s[2].upper()+s[3].upper()+s[4:]
	else:
		return s[0:len(s)-1]+s[len(s)-1].upper()
for i in sys.argv[1:]:
	j=mymangle(i);
	os.system ("./replace "+i+" "+j+" "+sys.argv[1]+" "+sys.argv[1]);
