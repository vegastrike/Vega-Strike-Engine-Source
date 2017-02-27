//      Normalmap types:
#define CINEMUT_NM           1
#define RED_IN_ALPHA_NM      2
#define TRADITIONAL_NM       3
#define DXT5_NM              4

//      Shininess sources
#define AD_HOC_SHININESS     1
#define GLOSS_IN_SPEC_ALPHA  2

/**********************************/
//  CUSTOMIZATION  (EDITABLE)
/**********************************/
#ifndef SHININESS_FROM
#define SHININESS_FROM       AD_HOC_SHININESS
#endif
#ifndef SHININESS_FROM_GLASS
#define SHININESS_FROM_GLASS AD_HOC_SHININESS
#endif
#ifndef NORMALMAP_TYPE
#define NORMALMAP_TYPE       DXT5_NM
#endif
#define NM_FREQ_SCALING      5.777
#define NM_Z_SCALING         0.015625
#define CORNER_TRIMMING_POW  177.77
#define DEGAMMA              1
#define DEGAMMA_SPECULAR     1
#define DEGAMMA_GLOW_MAP     1
#define DEGAMMA_LIGHTS       1
#define DEGAMMA_ENVIRONMENT  1
#define DEGAMMA_TEXTURES     1
#define SANITIZE             0
#define PARALLAX             1
#define SCATTER              1
#define DIM_SHALLOW_REFLECT  0
/**********************************/

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE) (all should be zero for normal operation)
/**********************************/
// Light source switches:
#define SUPRESS_AMBIENT      0
#define SUPRESS_LIGHTS       0
#define SUPRESS_ENVIRONMENT  0
#define SUPRESS_GLOWMAP      0
// Material switches:
#define SUPRESS_DIFFUSE      0
#define USE_DIFF_AS_METAL    0
#define SUPRESS_SPECULAR     0
#define SUPRESS_DIELECTRIC   0
#define FORCE_FULL_REFLECT   0
#define SUPRESS_NORMALMAP    0
// Hack switches:
#define SUPRESS_CORNER_TRIM  0
#define SHOW_FLAT_SHADED     0
#define SUPRESS_HI_Q_VNORM   0
#define SUPRESS_GAREFLECT    0
#define SUPRESS_NMRELEV      1
/**********************************/

//  SPECIALS:
//      Special showings for debugging (do not edit)
#define SHOW_NO_SPECIAL      0
#define SHOW_MAT             1
#define SHOW_NORMAL          2
#define SHOW_TANGENTX        3
#define SHOW_TANGENTY        4
#define SHOW_TANGENTZ        5
#define SHOW_BINORMX         6
#define SHOW_BINORMY         7
#define SHOW_BINORMZ         8
#define SHOW_NOR_DOT_VIEW    9
#define SHOW_TAN_DOT_VIEW   10
#define SHOW_BIN_DOT_VIEW   11
#define SHOW_NOR_DOT_LIGHT0 12
#define SHOW_TAN_DOT_LIGHT0 13
#define SHOW_BIN_DOT_LIGHT0 14
#define SHOW_NOR_DOT_LIGHT1 15
#define SHOW_TAN_DOT_LIGHT1 16
#define SHOW_BIN_DOT_LIGHT1 17
#define SHOW_NOR_DOT_VNORM  18
#define SHOW_IS_PERIPHERY   19
#define SHOW_IS_NEAR_VERT   20
#define SHOW_IS_UGLY_CORNER 21
#define SHOW_MA_NO_CORNERS  22
#define SHOW_VNOR_DOT_FNOR  23

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE)
// (set to SHOW_NO_SPECIAL for normal operation)
/**********************************/
#define SHOW_SPECIAL SHOW_NO_SPECIAL
/**********************************/

//      CONSTANTS
#define TWO_PI     (6.2831853071795862)
#define HALF_PI    (1.5707963267948966)
#define PI_OVER_3  (1.0471975511965976)



/**********************************/
//  SELF-ADAPTATION DEFINES
//  (externally controlled)
/**********************************/

#ifndef SRGB_FRAMEBUFFER
#define SRGB_FRAMEBUFFER 0
#endif

#if (SRGB_FRAMEBUFFER != 0)
#define REGAMMA 0
#else
#define REGAMMA 1
#endif

