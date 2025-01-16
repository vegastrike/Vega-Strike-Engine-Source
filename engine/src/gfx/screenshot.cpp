/*
 * screenshot.cpp
 *
 * Copyright (c) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "in_kb.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "vsimage.h"
// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#include <glut.h>

using namespace VSFileSystem;

void Screenshot(const KBData &, KBSTATE state) {
    if (state == PRESS) {
        GLint xywh[4] = {0, 0, 0, 0};
        xywh[2] = g_game.x_resolution;
        xywh[3] = g_game.y_resolution;
        glGetIntegerv(GL_VIEWPORT, xywh);
        unsigned char *tmp = (unsigned char *) malloc(xywh[2] * xywh[3] * 4 * sizeof(unsigned char));
        //memset(tmp,0x7f,xywh[2]*xywh[3]*4*sizeof(char));
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, xywh[2]);
        glFinish();
        glReadPixels(0, 0, xywh[2], xywh[3], GL_RGB, GL_UNSIGNED_BYTE, tmp);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        ::VSImage image;
        VSFileSystem::VSFile f;
        static int count = 0;
        std::string filename = "Screenshot" + XMLSupport::tostring(count) + ".png";
        for (;;) {
            if (f.OpenReadOnly(filename, TextureFile) <= VSFileSystem::Ok) {
                f.Close();
                filename = "Screenshot" + XMLSupport::tostring(++count) + ".png";
            } else {
                break;
            }
        }
        char *tmpchar = strdup(filename.c_str());
        image.WriteImage(tmpchar, tmp, PngImage, xywh[2], xywh[3], false, 8, TextureFile, true);
        free(tmpchar);
    }
}

