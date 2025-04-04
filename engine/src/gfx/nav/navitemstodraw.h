/**
 * navitemstodraw.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
// NO HEADER GUARD

//This draws the mouse cursor
//**********************************
void NavigationSystem::DrawCursor(float x, float y, float wid, float hei, const GFXColor &col) {
    float sizex, sizey;
    static bool modern_nav_cursor =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "nav", "modern_mouse_cursor", "true"));
    if (modern_nav_cursor) {
        static string
                mouse_cursor_sprite = vs_config->getVariable("graphics", "nav", "mouse_cursor_sprite", "mouse.spr");
        static VSSprite MouseVSSprite(mouse_cursor_sprite.c_str(), BILINEAR, GFXTRUE);
        GFXBlendMode(SRCALPHA, INVSRCALPHA);
        GFXColorf(GUI_OPAQUE_WHITE());

        //Draw the cursor sprite.
        GFXEnable(TEXTURE0);
        GFXDisable(DEPTHTEST);
        GFXDisable(TEXTURE1);
        MouseVSSprite.GetSize(sizex, sizey);
        MouseVSSprite.SetPosition(x + sizex / 2, y + sizey / 2);
        MouseVSSprite.Draw();
    } else {
        GFXColorf(col);
        GFXDisable(TEXTURE0);
        GFXDisable(LIGHTING);
        GFXBlendMode(SRCALPHA, INVSRCALPHA);

        const float verts[8 * 3] = {
                x, y, 0,
                x, y - hei, 0,
                x, y, 0,
                x + wid, y - 0.75f * hei, 0,
                x, y - hei, 0,
                x + 0.35f * wid, y - 0.6f * hei, 0,
                x + 0.35f * wid, y - 0.6f * hei, 0,
                x + wid, y - 0.75f * hei, 0,
        };
        GFXDraw(GFXLINE, verts, 8);

        GFXEnable(TEXTURE0);
    }
}
//**********************************

//This draws the grid over the nav screen area
//**********************************
void NavigationSystem::DrawGrid(float &x1, float &x2, float &y1, float &y2, const GFXColor &col) {
    if (!vega_config::config->graphics.hud.draw_nav_grid) {
        return;
    }
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    float deltax = x2 - x1;
    deltax = deltax / 10;
    float deltay = y2 - y1;
    deltay = deltay / 10;

    static VertexBuilder<> verts;
    verts.clear();
    for (int i = 1; i < 10; i++) {
        verts.insert(x1 + i * deltax, y1, 0);
        verts.insert(x1 + i * deltax, y2, 0);
    }
    for (int i = 1; i < 10; i++) {
        verts.insert(x1, y1 + i * deltay, 0);
        verts.insert(x2, y1 + i * deltay, 0);
    }
    GFXDraw(GFXLINE, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a circle over the screen
//**********************************
void NavigationSystem::DrawCircle(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    // 20 segments
    static VertexBuilder<> verts;
    verts.clear();
    for (float i = 0; i < 2 * M_PI + M_PI / 10; i += M_PI / 10) {
        verts.insert(
                x + 0.5 * size * cos(i),
                y + 0.5 * size * sin(i),
                0
        );
    }
    GFXDraw(GFXLINESTRIP, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a half circle, centered at the top 1/4 center
//**********************************
void NavigationSystem::DrawHalfCircleTop(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    // 10 segments
    static VertexBuilder<> verts;
    verts.clear();
    for (float i = 0; i < M_PI + M_PI / 10; i += M_PI / 10) {
        verts.insert(
                x + 0.5 * size * cos(i),
                y + 0.5 * size * sin(i) - 0.25 * size,
                0
        );
    }
    GFXDraw(GFXLINESTRIP, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a half circle, centered at the bottom 1/4 center
//**********************************
void NavigationSystem::DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    // 10 segments
    static VertexBuilder<> verts;
    verts.clear();
    for (float i = M_PI; i < 2 * M_PI + M_PI / 10; i += M_PI / 10) {
        verts.insert(
                x + 0.5 * size * cos(i),
                y + 0.5 * size * sin(i) + 0.25 * size,
                0
        );
    }
    GFXDraw(GFXLINESTRIP, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a planet icon. circle + lightning thingy
//**********************************
void NavigationSystem::DrawPlanet(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    static VertexBuilder<> verts;
    verts.clear();
    for (float i = 0; i < 2 * M_PI; i += M_PI / 10) {
        verts.insert(
                x + 0.5 * size * cos(i),
                y + 0.5 * size * sin(i),
                0
        );
        verts.insert(
                x + 0.5 * size * cos(i + M_PI / 10),
                y + 0.5 * size * sin(i + M_PI / 10),
                0
        );
    }
    verts.insert(x - 0.5 * size, y, 0);
    verts.insert(x, y + 0.2 * size, 0);
    verts.insert(x, y + 0.2 * size, 0);
    verts.insert(x, y - 0.2 * size, 0);
    verts.insert(x, y - 0.2 * size, 0);
    verts.insert(x + 0.5 * size, y, 0);
    GFXDraw(GFXLINE, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a station icon. 3x3 grid
//**********************************
void NavigationSystem::DrawStation(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    float segment = size / 3;
    static VertexBuilder<> verts;
    verts.clear();
    for (int i = 0; i < 4; i++) {
        verts.insert(
                x - 0.5 * size,
                y - 0.5 * size + i * segment,
                0
        );
        verts.insert(
                x + 0.5 * size,
                y - 0.5 * size + i * segment,
                0
        );
    }
    for (int i = 0; i < 4; i++) {
        verts.insert(
                x - 0.5 * size + i * segment,
                y - 0.5 * size,
                0
        );
        verts.insert(
                x - 0.5 * size + i * segment,
                y + 0.5 * size,
                0
        );
    }
    GFXDraw(GFXLINE, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a jump node icon
//**********************************
void NavigationSystem::DrawJump(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    static VertexBuilder<> verts;
    verts.clear();
    for (float i = 0; i < 2 * M_PI; i += M_PI / 10) {
        verts.insert(
                x + 0.5 * size * cos(i),
                y + 0.5 * size * sin(i),
                0
        );
        verts.insert(
                x + 0.5 * size * cos(i + M_PI / 10),
                y + 0.5 * size * sin(i + M_PI / 10),
                0
        );
    }
    verts.insert(x, y + 0.5 * size, 0);
    verts.insert(x + 0.125 * size, y + 0.125 * size, 0);
    verts.insert(x, y + 0.5 * size, 0);
    verts.insert(x - 0.125 * size, y + 0.125 * size, 0);
    verts.insert(x, y - 0.5 * size, 0);
    verts.insert(x + 0.125 * size, y - 0.125 * size, 0);
    verts.insert(x, y - 0.5 * size, 0);
    verts.insert(x - 0.125 * size, y - 0.125 * size, 0);
    verts.insert(x - 0.5 * size, y, 0);
    verts.insert(x - 0.125 * size, y + 0.125 * size, 0);
    verts.insert(x - 0.5 * size, y, 0);
    verts.insert(x - 0.125 * size, y - 0.125 * size, 0);
    verts.insert(x + 0.5 * size, y, 0);
    verts.insert(x + 0.125 * size, y + 0.125 * size, 0);
    verts.insert(x + 0.5 * size, y, 0);
    verts.insert(x + 0.125 * size, y - 0.125 * size, 0);
    GFXDraw(GFXLINE, verts);

    GFXEnable(TEXTURE0);
}

//**********************************

//This will draw a missile icon
//**********************************
void NavigationSystem::DrawMissile(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    const float verts[12 * 3] = {
            x - 0.5f * size, y - 0.125f * size, 0,
            x, y + 0.375f * size, 0,
            x + 0.5f * size, y - 0.125f * size, 0,
            x, y + 0.375f * size, 0,
            x - 0.25f * size, y - 0.125f * size, 0,
            x - 0.25f * size, y + 0.125f * size, 0,
            x + 0.25f * size, y - 0.125f * size, 0,
            x + 0.25f * size, y + 0.125f * size, 0,
            x - 0.25f * size, y + 0.125f * size, 0,
            x, y - 0.125f * size, 0,
            x + 0.25f * size, y + 0.125f * size, 0,
            x, y - 0.125f * size, 0,
    };
    GFXDraw(GFXLINE, verts, 12);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw a square set of corners
//**********************************
void NavigationSystem::DrawTargetCorners(float x, float y, float size, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    const float verts[16 * 3] = {
            x - 0.5f * size, y + 0.5f * size, 0,
            x - 0.3f * size, y + 0.5f * size, 0,
            x - 0.5f * size, y + 0.5f * size, 0,
            x - 0.5f * size, y + 0.3f * size, 0,
            x + 0.5f * size, y + 0.5f * size, 0,
            x + 0.3f * size, y + 0.5f * size, 0,
            x + 0.5f * size, y + 0.5f * size, 0,
            x + 0.5f * size, y + 0.3f * size, 0,
            x - 0.5f * size, y - 0.5f * size, 0,
            x - 0.3f * size, y - 0.5f * size, 0,
            x - 0.5f * size, y - 0.5f * size, 0,
            x - 0.5f * size, y - 0.3f * size, 0,
            x + 0.5f * size, y - 0.5f * size, 0,
            x + 0.3f * size, y - 0.5f * size, 0,
            x + 0.5f * size, y - 0.5f * size, 0,
            x + 0.5f * size, y - 0.3f * size, 0,
    };
    GFXDraw(GFXLINE, verts, 16);

    GFXEnable(TEXTURE0);
}
//**********************************

//This will draw an oriented circle
//**********************************
void NavigationSystem::DrawNavCircle(float x, float y, float size, float rot_x, float rot_y, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    const int circles = 4;
    const int segments = 20;
    const int segments2 = 12;
    const int vnum = 2 * (circles * segments + segments2);
    static VertexBuilder<float, 3, 0, 4> verts;
    verts.clear();
    verts.reserve(vnum);
    for (float i = 0; i < 2 * M_PI; i += (2 * M_PI / segments)) {
        GFXColor ci(col.r, col.g, col.b * fabs(sin(i / 2.0)), col.a);
        QVector pos1((0.6 * size * cos(i)), (0.6 * size * sin(i)), 0);
        QVector pos2((0.6 * size * cos(i + (2 * M_PI / segments))), (0.6 * size * sin(i + (6.28 / segments))), 0);

        pos1 = dxyz(pos1, 0, 0, rot_y);
        pos1 = dxyz(pos1, rot_x, 0, 0);
        pos2 = dxyz(pos2, 0, 0, rot_y);
        pos2 = dxyz(pos2, rot_x, 0, 0);

        float standard_unit = 0.25 * 1.2 * size;
        float zdistance1 = ((1.2 * size) - pos1.k);
        float zdistance2 = ((1.2 * size) - pos2.k);
        float zscale1 = standard_unit / zdistance1;
        float zscale2 = standard_unit / zdistance2;
        pos1 *= (zscale1 * 5.0);
        pos2 *= (zscale2 * 5.0);

        for (int j = circles; j > 0; j--) {
            pos1 *= (float(j) / float(circles));
            pos2 *= (float(j) / float(circles));

            Vector pos1t((x + pos1.i), (y + (pos1.j)), 0);
            Vector pos2t((x + pos2.i), (y + (pos2.j)), 0);

            verts.insert(GFXColorVertex(pos1t, ci));
            verts.insert(GFXColorVertex(pos2t, ci));
        }
    }
    for (float i = 0; i < 2 * M_PI; i += (2 * M_PI / segments2)) {
        GFXColor ci(col.r, col.g, col.b * fabs(sin(i / 2.0)), col.a);
        QVector pos1((0.6 * size * cos(i) / float(circles * 2)), (0.6 * size * sin(i) / float(circles * 2)), 0);
        QVector pos2((0.6 * size * cos(i)), (0.6 * size * sin(i)), 0);

        if ((fabs(i - 1.57) < 0.01) || (fabs(i - 3.14) < 0.01) || (fabs(i - 4.71) < 0.01) || (i < 0.01)) {
            pos2 *= 1.1;
        }
        pos1 = dxyz(pos1, 0, 0, rot_y);
        pos1 = dxyz(pos1, rot_x, 0, 0);
        pos2 = dxyz(pos2, 0, 0, rot_y);
        pos2 = dxyz(pos2, rot_x, 0, 0);

        float standard_unit = 0.25 * 1.2 * size;
        float zdistance1 = ((1.2 * size) - pos1.k);
        float zdistance2 = ((1.2 * size) - pos2.k);
        float zscale1 = standard_unit / zdistance1;
        float zscale2 = standard_unit / zdistance2;
        pos1 *= (zscale1 * 5.0);
        pos2 *= (zscale2 * 5.0);

        pos1.i += x;
        pos1.j += y;
        pos2.i += x;
        pos2.j += y;

        // pos, col
        verts.insert(GFXColorVertex(pos1.Cast(), ci));
        verts.insert(GFXColorVertex(pos2.Cast(), ci));
    }
    GFXDraw(GFXLINE, verts);

    GFXEnable(TEXTURE0);
}
//**********************************

