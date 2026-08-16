/*
 * font.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Mike Byron
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Danny Gehl
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "font.h"
#include "src/vs_logging.h"
#include "gldrv/gl_globals.h"
#include "src/gfxlib.h"
#include <imgui.h>
#include <imgui_internal.h> // For access to ImFont metrics
#include "configuration/configuration.h"

void Font::calcMetricsIfNeeded(void) const {
    if (m_needMetrics) {
        const_cast<Font*>(this)->calcMetrics();
    }
}

// Calculates the metrics for this font.
// This does the real work, and doesn't check whether it needs to be done.
void Font::calcMetrics(void) {

    // Determine how much we need to scale the font 
    // to reach the desired 'm_size' (0.1 = 10% of the screen height).
    double scale_factor = static_cast<double>(m_size) / configuration().graphics.font_point_dbl;

    // Vertical Scaling: 
    // We adjust by the configured GUI scale factor.
    m_verticalScaling = scale_factor * configuration().graphics.gui_font_scale_dbl; 

    // Horizontal Scaling:
    // Correct for the aspect ratio so characters aren't squashed.
    double aspect_ratio = static_cast<double>(configuration().graphics.resolution_y) / static_cast<double>(configuration().graphics.resolution_x);
    m_horizontalScaling = m_verticalScaling * aspect_ratio;

    m_needMetrics = false;
    // printf("Vertical Scaling: %f, Horizontal Scaling: %f,\n", 
    //     m_verticalScaling, m_horizontalScaling);
}

float Font::drawChar(char c, float xOffset) const {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) {
        return 0.0f;
    }

    // Get the texture ID first
    ImTextureID tex_id = ctx->IO.Fonts->TexRef.GetTexID();

    // If tex_id is 0, the font atlas is not initialized or has been destroyed
    if (!tex_id || tex_id == static_cast<ImTextureID>(-1)) {
        return 0.0f;
    }

    calcMetricsIfNeeded();
    
    // Access the baked font data instead of the raw ImFont object
    ImFontBaked* font_baked = ImGui::GetFontBaked(); 
    if (!font_baked) {
        return 0.0f;
    }
    
    // Use the new FindGlyph method on the baked font
    const ImFontGlyph* glyph = font_baked->FindGlyph((ImWchar)c);
    if (!glyph) {
        return 0.0f;
    }

    // IMPORTANT: Tell OpenGL which unit we are binding to
    GFXActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(static_cast<intptr_t>(tex_id)));

    const float unit_scale = 1;//2.5f;
    // move up one line
    const float yOffset = -font_baked->Ascent;//-50.0f;

    float x0 = (glyph->X0 + xOffset) * unit_scale;
    float y0 = (glyph->Y0 + yOffset) * unit_scale;
    float x1 = (glyph->X1 + xOffset) * unit_scale;
    float y1 = (glyph->Y1 + yOffset) * unit_scale;

    // Define interleaved data: [x, y, z, u, v]
    const float verts[] = {
        x0, -y0, 0.0f, glyph->U0, glyph->V0,
        x1, -y0, 0.0f, glyph->U1, glyph->V0,
        x1, -y1, 0.0f, glyph->U1, glyph->V1,
        x0, -y1, 0.0f, glyph->U0, glyph->V1
    };

    // poor man's bold
    if(m_strokeWeight == BOLD_STROKE) {
        // Draw it again with a slight offset to make it look "thicker"
        const float boldOffset = 2.0f; // Tweak this for thickness
        const float boldVerts[] = {
            x0 + boldOffset, -y0, 0.0f, glyph->U0, glyph->V0,
            x1 + boldOffset, -y0, 0.0f, glyph->U1, glyph->V0,
            x1 + boldOffset, -y1, 0.0f, glyph->U1, glyph->V1,
            x0 + boldOffset, -y1, 0.0f, glyph->U0, glyph->V1
        };
        GFXDraw(GFXQUAD, boldVerts, 4, 3, 0, 2, 0);
    }

    // Draw using the GFXDraw signature: 
    // Type: GFXQUAD, Data: verts, vnum: 4, vsize: 3, csize: 0, tsize0: 2, tsize1: 0
    GFXDraw(GFXQUAD, verts, 4, 3, 0, 2, 0);

    // Return the advance value
    return static_cast<float>(glyph->AdvanceX);
}

double Font::charWidth(char c) const {
    calcMetricsIfNeeded();
    ImFontBaked* font_baked = ImGui::GetFontBaked();
    if (font_baked == nullptr) {
        return 0.0F;
    }
    const ImFontGlyph* glyph = font_baked->FindGlyph((ImWchar)c);
    return glyph ? static_cast<double>(glyph->AdvanceX) : 0.0;
}

double Font::stringWidth(const std::string &str) const {
    calcMetricsIfNeeded();
    double width = 0.0;
    for (char c : str) {
        width += charWidth(c);
    }
    return width;
}

double Font::verticalScaling(void) const {
    calcMetricsIfNeeded();
    return m_verticalScaling;
}

double Font::horizontalScaling(void) const {
    calcMetricsIfNeeded();
    return m_horizontalScaling;
}

double Font::strokeWidth(void) const {
    return m_strokeWidth;
}

double Font::ascent(void) const {
    ImFontBaked* font_baked = ImGui::GetFontBaked();
    return font_baked != nullptr ? font_baked->Ascent : 0.0F;
}

double Font::descent(void) const {
    ImFontBaked* font_baked = ImGui::GetFontBaked();
    return font_baked != nullptr ? font_baked->Descent : 0.0F;
}

bool useStroke(void) {
    return true;
}

ImFont* Font::getFont(void) const {
    if(m_strokeWeight == BOLD_STROKE) {
        return gui_Fonts.Bold;
    }
    return gui_Fonts.Regular;
}