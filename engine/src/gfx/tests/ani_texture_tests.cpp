/*
 * ani_texture_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


#include <gtest/gtest.h>
#include "gfx/ani_texture_utils.h"

#include <cstring>


// -------------------------------------------------------------------
// IsNoAlphamapToken tests
// -------------------------------------------------------------------

TEST(AniTextureUtils, IsNoAlphamapToken_empty_string) {
    EXPECT_TRUE(IsNoAlphamapToken(""));
}

TEST(AniTextureUtils, IsNoAlphamapToken_dash) {
    EXPECT_TRUE(IsNoAlphamapToken("-"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_true_lowercase) {
    EXPECT_TRUE(IsNoAlphamapToken("true"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_false_lowercase) {
    EXPECT_TRUE(IsNoAlphamapToken("false"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_true_uppercase) {
    EXPECT_TRUE(IsNoAlphamapToken("TRUE"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_false_uppercase) {
    EXPECT_TRUE(IsNoAlphamapToken("FALSE"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_true_mixed_case) {
    EXPECT_TRUE(IsNoAlphamapToken("TrUe"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_false_mixed_case) {
    EXPECT_TRUE(IsNoAlphamapToken("FaLsE"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_filename) {
    EXPECT_FALSE(IsNoAlphamapToken("alphamap.bmp"));
}

TEST(AniTextureUtils, IsNoAlphamapToken_random_string) {
    EXPECT_FALSE(IsNoAlphamapToken("something_else"));
}


// -------------------------------------------------------------------
// ParseAniLine tests
// -------------------------------------------------------------------

TEST(AniTextureUtils, ParseAniLine_single_token) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 1);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "");
    EXPECT_STREQ(opt, "");
}

TEST(AniTextureUtils, ParseAniLine_two_tokens) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp alpha.bmp", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "alpha.bmp");
    EXPECT_STREQ(opt, "");
}

TEST(AniTextureUtils, ParseAniLine_three_tokens) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp alpha.bmp addressMode=wrap", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 3);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "alpha.bmp");
    EXPECT_STREQ(opt, "addressMode=wrap");
}

TEST(AniTextureUtils, ParseAniLine_options_with_spaces) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp alpha.bmp addressMode=wrap mins=0 mint=0",
                             file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 3);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "alpha.bmp");
    EXPECT_STREQ(opt, "addressMode=wrap mins=0 mint=0");
}

TEST(AniTextureUtils, ParseAniLine_empty_line) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 0);
    EXPECT_STREQ(file, "");
    EXPECT_STREQ(alp, "");
    EXPECT_STREQ(opt, "");
}

TEST(AniTextureUtils, ParseAniLine_whitespace_only) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("   \t  ", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 0);
    EXPECT_STREQ(file, "");
    EXPECT_STREQ(alp, "");
    EXPECT_STREQ(opt, "");
}

TEST(AniTextureUtils, ParseAniLine_leading_whitespace) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("  image.bmp alpha.bmp", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "alpha.bmp");
    EXPECT_STREQ(opt, "");
}

TEST(AniTextureUtils, ParseAniLine_two_tokens_with_dash_alphamap) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp -", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "-");
    EXPECT_STREQ(opt, "");
    // The dash is a no-alphamap token
    EXPECT_TRUE(IsNoAlphamapToken(alp));
}

TEST(AniTextureUtils, ParseAniLine_two_tokens_with_true_alphamap) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp true", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "true");
    EXPECT_TRUE(IsNoAlphamapToken(alp));
}

TEST(AniTextureUtils, ParseAniLine_two_tokens_with_false_alphamap) {
    char file[512], alp[512], opt[512];
    int count = ParseAniLine("image.bmp false", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 2);
    EXPECT_STREQ(file, "image.bmp");
    EXPECT_STREQ(alp, "false");
    EXPECT_TRUE(IsNoAlphamapToken(alp));
}

TEST(AniTextureUtils, ParseAniLine_buffer_overflow_protection) {
    // Use tiny buffers to verify truncation works
    char file[4], alp[4], opt[4];
    int count = ParseAniLine("verylongfilename.bmp anotherlongalpha.bmp someoptions",
                             file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    EXPECT_EQ(count, 3);
    // Should be truncated to 3 characters + null
    EXPECT_STRNE(file, "verylongfilename.bmp");
    EXPECT_EQ(strlen(file), 3u);
    EXPECT_STRNE(alp, "anotherlongalpha.bmp");
    EXPECT_EQ(strlen(alp), 3u);
    EXPECT_STRNE(opt, "someoptions");
    EXPECT_EQ(strlen(opt), 3u);
}

TEST(AniTextureUtils, ParseAniLine_null_output_buffers) {
    // Should not crash when output buffers are null
    int count = ParseAniLine("image.bmp alpha.bmp opts", nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_EQ(count, 3);
}

TEST(AniTextureUtils, ParseAniLine_combined_check) {
    // Simulate the check used in LoadAni and LoadFrame:
    // if ((num_gets < 2) || isNoAlphamapToken(alp)) { alp[0] = '\0'; }
    char file[512], alp[512], opt[512];

    // Case: two tokens, but alp is "true" — should be cleared
    int count = ParseAniLine("image.bmp true", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    if ((count < 2) || IsNoAlphamapToken(alp)) {
        alp[0] = '\0';
    }
    EXPECT_STREQ(alp, "");

    // Case: two tokens, real alphamap — should survive
    count = ParseAniLine("image.bmp alpha.bmp", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    if ((count < 2) || IsNoAlphamapToken(alp)) {
        alp[0] = '\0';
    }
    EXPECT_STREQ(alp, "alpha.bmp");

    // Case: only one token — should be cleared
    count = ParseAniLine("image.bmp", file, sizeof(file), alp, sizeof(alp), opt, sizeof(opt));
    if ((count < 2) || IsNoAlphamapToken(alp)) {
        alp[0] = '\0';
    }
    EXPECT_STREQ(alp, "");
}
