//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

// EMap MapGuide hatch pattern set or Heidi, version 1.0.
// call by, or example, hr.set_ace_pattern(mg_hatchPatterns[UserHatchPattern::Net]);



struct Helper_Line_Set
{
public:
    double m_x; 
    double m_y;
    double m_angle;
    double m_spacing;
    double m_skew;
    WT_Unsigned_Integer32 m_patsize;
    const double * m_patdata;
public:
    Helper_Line_Set(const double & x, 
                    const double & y,
                    const double & angle,
                    const double & spacing,
                    const double & skew,
                    WT_Unsigned_Integer32 patsize,
                    const double * patdata )
    : m_x(x)
    , m_y(y)
    , m_angle(angle)
    , m_spacing(spacing)                  
    , m_skew(skew)
    , m_patsize(patsize)
    , m_patdata(patdata)
    {}

    Helper_Line_Set(const double & x, 
                    const double & y,
                    const double & angle,
                    const double & spacing)
    : m_x(x)
    , m_y(y)
    , m_angle(angle)
    , m_spacing(spacing)                  
    , m_skew(0)
    , m_patsize(0)
    , m_patdata(WD_Null)
    {}
};
    
struct Helper_Hatch_Pattern
{
public:
     public:
        enum Enum {
            Net,              //  0
            Net_45,           //  1
            Line,             //  2
            Line_45,          //  3
            Line_90,          //  4
            Line_135,         //  5
            Square,           //  6
            Angle,            //  7
            Box,              //  8
            Brass,            //  9
            Brick,            // 10
            Brstone,          // 11
            Clay,             // 12
            Cork,             // 13
            Cross,            // 14
            Dash,             // 15
            Dolmit,           // 16
            Dots,             // 17
            Earth,            // 18
            Escher,           // 19
            Flex,             // 20
            Grass,            // 21
            Grate,            // 22
            Hex,              // 23
            Honey,            // 24
            Hound,            // 25
            Insul,            // 26
            Mudst,            // 27
            Net3,             // 28
            Plast,            // 29
            Plasti,           // 30
            Sacncr,           // 31
            Stars,            // 32
            Steel,            // 33
            Swamp,            // 34
            Trans,            // 35
            Triang,           // 36
            Zigzag,           // 37
            Ar_b816,          // 38
            Ar_b816c,         // 39
            Ar_b88,           // 40
            Ar_brelm,         // 41
            Ar_brstd,         // 42
            Ar_conc,          // 43
            Ar_hbone,         // 44
            Ar_parq1,         // 45
            Ar_rroo,         // 46
            Ar_rroo_90,      // 47
            Ar_rshke,         // 48
            Ar_rshke_90,      // 49
            Ar_rshke_180,     // 50
            Ar_rshke_270,     // 51
            Ar_sand,          // 52
            Ansi31,           // 53
            Ansi32,           // 54
            Ansi33,           // 55
            Ansi34,           // 56
            Ansi35,           // 57
            Ansi36,           // 58
            Ansi37,           // 59
            Ansi38,           // 60
            Acad_iso02w100,   // 61
            Acad_iso03w100,   // 62
            Acad_iso04w100,   // 63
            Acad_iso05w100,   // 64
            Acad_iso06w100,   // 65
            Acad_iso07w100,   // 66
            Acad_iso08w100,   // 67
            Acad_iso09w100,   // 68
            Acad_iso10w100,   // 69
            Acad_iso11w100,   // 70
            Acad_iso12w100,   // 71
            Acad_iso13w100,   // 72
            Acad_iso14w100,   // 73
            Acad_iso15w100,   // 74
            count
        };
public:
    WT_Unsigned_Integer32 m_size;
    Helper_Line_Set* m_sets;
public:
    Helper_Hatch_Pattern(WT_Unsigned_Integer32 size,
                         const Helper_Line_Set* sets)
                        : m_size(size)
                        , m_sets((Helper_Line_Set*)sets)

    {;}
};


/* NET - Horizontal / vertical grid (heidi crosshatch) */
static Helper_Line_Set const mgpls_net[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.200000)
};


/* NET_45 - Diagonal grid (heidi diamonds) */

static Helper_Line_Set const mgpls_net_45[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.141421),
    Helper_Line_Set (0.000000, 0.000000, 2.35619, 0.141421)
};


/* LINE - Parallel horizontal lines (heidi horizontal bars) */

static Helper_Line_Set const mgpls_line[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000)
};


/* LINE_45 - Slant Left (heidi slant left) */

static Helper_Line_Set const mgpls_line_45[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.141421)
};


/* LINE_90 - Vertical Bars (heidi vertical bars) */

static Helper_Line_Set const mgpls_line_90[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.200000)
};


/* LINE_135 - Slant Right (heidi slant right) */

static Helper_Line_Set const mgpls_line_135[] = {
    Helper_Line_Set (0.000000, 0.000000, 2.35619, 0.141421)
};


/* SQUARE - Small aligned squares (heidi checkerboard) */

static double const mgdp_square__1 [] = {0.100000, 0.100000};
static double const mgdp_square__2 [] = {0.100000, 0.100000};
static Helper_Line_Set const mgpls_square[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.100000, 0.000000, 2, mgdp_square__1),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.100000, 0.000000, 2, mgdp_square__2)
};


/* ANGLE - Angle steel */

static double const mgdp_angle__1 [] = {0.175000, 0.0750000};
static double const mgdp_angle__2 [] = {0.175000, 0.0750000};
static Helper_Line_Set const mgpls_angle[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.250000, 0.000000, 2, mgdp_angle__1),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.250000, 0.000000, 2, mgdp_angle__2)
};


/* BOX - Box steel */

static double const mgdp_box__3 [] = {0.0, 0.158750, 0.158750, 0.0};
static double const mgdp_box__4 [] = {0.0, 0.158750, 0.158750, 0.0};
static double const mgdp_box__5 [] = {0.158750, 0.158750};
static double const mgdp_box__6 [] = {0.158750, 0.158750};
static double const mgdp_box__7 [] = {0.158750, 0.158750};
static double const mgdp_box__8 [] = {0.158750, 0.158750};
static Helper_Line_Set const mgpls_box[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.635000),
    Helper_Line_Set (0.158750, 0.000000, 1.57080, 0.635000),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.635000, 0.000000, 4, mgdp_box__3),
    Helper_Line_Set (0.000000, 0.158750, 0.000000, 0.635000, 0.000000, 4, mgdp_box__4),
    Helper_Line_Set (0.000000, 0.317500, 0.000000, 0.635000, 0.000000, 2, mgdp_box__5),
    Helper_Line_Set (0.000000, 0.476250, 0.000000, 0.635000, 0.000000, 2, mgdp_box__6),
    Helper_Line_Set (0.317500, 0.000000, 1.57080, 0.635000, 0.000000, 2, mgdp_box__7),
    Helper_Line_Set (0.476250, 0.000000, 1.57080, 0.635000, 0.000000, 2, mgdp_box__8)
};


/* BRASS - Brass material  */

static double const mgdp_brass__2 [] = {0.158750, 0.0793750};
static Helper_Line_Set const mgpls_brass[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.317500),
    Helper_Line_Set (0.000000, 0.158750, 0.000000, 0.317500, 0.000000, 2, mgdp_brass__2)
};


/* BRICK - Brick or masonry-type surace  */

static double const mgdp_brick__2 [] = {0.206375, 0.206375};
static double const mgdp_brick__3 [] = {0.0, 0.206375, 0.206375, 0.0};
static Helper_Line_Set const mgpls_brick[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.206375),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.412750, 0.000000, 2, mgdp_brick__2),
    Helper_Line_Set (0.206375, 0.000000, 1.57080, 0.412750, 0.000000, 4, mgdp_brick__3)
};


/* BRSTONE - Brick and stone  */

static double const mgdp_brstone__2 [] = {0.300013, 0.300013};
static double const mgdp_brstone__3 [] = {0.300013, 0.300013};
static double const mgdp_brstone__4 [] = {0.0, 0.818217, 0.0909130, 0.0};
static double const mgdp_brstone__5 [] = {0.0, 0.818217, 0.0909130, 0.0};
static double const mgdp_brstone__6 [] = {0.0, 0.818217, 0.0909130, 0.0};
static double const mgdp_brstone__7 [] = {0.0, 0.818217, 0.0909130, 0.0};
static double const mgdp_brstone__8 [] = {0.0, 0.818217, 0.0909130, 0.0};
static Helper_Line_Set const mgpls_brstone[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.300013),
    Helper_Line_Set (0.818217, 0.000000, 1.57080, 0.454565, 0.300013, 2, mgdp_brstone__2),
    Helper_Line_Set (0.727304, 0.000000, 1.57080, 0.454565, 0.300013, 2, mgdp_brstone__3),
    Helper_Line_Set (0.818217, 0.0500021, 0.000000, 0.300013, 0.454565, 4, mgdp_brstone__4),
    Helper_Line_Set (0.818217, 0.100004, 0.000000, 0.300013, 0.454565, 4, mgdp_brstone__5),
    Helper_Line_Set (0.818217, 0.150006, 0.000000, 0.300013, 0.454565, 4, mgdp_brstone__6),
    Helper_Line_Set (0.818217, 0.200008, 0.000000, 0.300013, 0.454565, 4, mgdp_brstone__7),
    Helper_Line_Set (0.818217, 0.250011, 0.000000, 0.300013, 0.454565, 4, mgdp_brstone__8)
};


/* CLAY - Clay material  */

static double const mgdp_clay__4 [] = {0.300000, 0.200000};
static Helper_Line_Set const mgpls_clay[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.300000),
    Helper_Line_Set (0.000000, 0.0500000, 0.000000, 0.300000),
    Helper_Line_Set (0.000000, 0.100000, 0.000000, 0.300000),
    Helper_Line_Set (0.000000, 0.200000, 0.000000, 0.300000, 0.000000, 2, mgdp_clay__4)
};


/* CORK - Cork material  */

static double const mgdp_cork__2 [] = {0.282843, 0.282843};
static double const mgdp_cork__3 [] = {0.282843, 0.282843};
static double const mgdp_cork__4 [] = {0.282843, 0.282843};
static Helper_Line_Set const mgpls_cork[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000),
    Helper_Line_Set (0.100000, -0.100000, 2.35619, 0.565686, 0.000000, 2, mgdp_cork__2),
    Helper_Line_Set (0.150000, -0.100000, 2.35619, 0.565686, 0.000000, 2, mgdp_cork__3),
    Helper_Line_Set (0.200000, -0.100000, 2.35619, 0.565686, 0.000000, 2, mgdp_cork__4)
};


/* CROSS - A series o crosses  */

static double const mgdp_cross__1 [] = {0.150003, 0.450009};
static double const mgdp_cross__2 [] = {0.150003, 0.450009};
static Helper_Line_Set const mgpls_cross[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.300006, 0.300006, 2, mgdp_cross__1),
    Helper_Line_Set (0.0750014, -0.0750014, 1.57080, 0.300006, 0.300006, 2, mgdp_cross__2)
};


/* DASH - Dashed lines  */

static double const mgdp_dash__1 [] = {0.150003, 0.150003};
static Helper_Line_Set const mgpls_dash[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.150003, 0.150003, 2, mgdp_dash__1)
};


/* DOLMIT - Geological rock layering  */

static double const mgdp_dolmit__2 [] = {0.282843, 0.565685};
static Helper_Line_Set const mgpls_dolmit[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000),
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.565685, 0.000000, 2, mgdp_dolmit__2)
};


/* DOTS - A series o dots  */

static double const mgdp_dots__1 [] = {0.000000, 0.200000};
static Helper_Line_Set const mgpls_dots[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000, 0.100000, 2, mgdp_dots__1)
};


/* EARTH - Earth or ground (subterranean)  */

static double const mgdp_earth__1 [] = {0.555625, 0.555625};
static double const mgdp_earth__2 [] = {0.555625, 0.555625};
static double const mgdp_earth__3 [] = {0.555625, 0.555625};
static double const mgdp_earth__4 [] = {0.555625, 0.555625};
static double const mgdp_earth__5 [] = {0.555625, 0.555625};
static double const mgdp_earth__6 [] = {0.555625, 0.555625};
static Helper_Line_Set const mgpls_earth[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.555625, 0.555625, 2, mgdp_earth__1),
    Helper_Line_Set (0.000000, 0.208359, 0.000000, 0.555625, 0.555625, 2, mgdp_earth__2),
    Helper_Line_Set (0.000000, 0.416719, 0.000000, 0.555625, 0.555625, 2, mgdp_earth__3),
    Helper_Line_Set (0.0694531, 0.486172, 1.57080, 0.555625, 0.555625, 2, mgdp_earth__4),
    Helper_Line_Set (0.277812, 0.486172, 1.57080, 0.555625, 0.555625, 2, mgdp_earth__5),
    Helper_Line_Set (0.486172, 0.486172, 1.57080, 0.555625, 0.555625, 2, mgdp_earth__6)
};


/* ESCHER - Escher pattern */

static double const mgdp_escher__1 [] = {0.698500, 0.0635000};
static double const mgdp_escher__2 [] = {0.698500, 0.0635000};
static double const mgdp_escher__3 [] = {0.698500, 0.0635000};
static double const mgdp_escher__4 [] = {0.127000, 0.635000};
static double const mgdp_escher__5 [] = {0.127000, 0.635000};
static double const mgdp_escher__6 [] = {0.127000, 0.635000};
static double const mgdp_escher__7 [] = {0.127000, 0.635000};
static double const mgdp_escher__8 [] = {0.127000, 0.635000};
static double const mgdp_escher__9 [] = {0.127000, 0.635000};
static double const mgdp_escher__10 [] = {0.127000, 0.635000};
static double const mgdp_escher__11 [] = {0.127000, 0.635000};
static double const mgdp_escher__12 [] = {0.127000, 0.635000};
static double const mgdp_escher__13 [] = {0.127000, 0.635000};
static double const mgdp_escher__14 [] = {0.127000, 0.635000};
static double const mgdp_escher__15 [] = {0.127000, 0.635000};
static double const mgdp_escher__16 [] = {0.444500, 0.317500};
static double const mgdp_escher__17 [] = {0.444500, 0.317500};
static double const mgdp_escher__18 [] = {0.444500, 0.317500};
static double const mgdp_escher__19 [] = {0.444500, 0.317500};
static double const mgdp_escher__20 [] = {0.444500, 0.317500};
static double const mgdp_escher__21 [] = {0.444500, 0.317500};
static Helper_Line_Set const mgpls_escher[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.04720, 0.659913, -0.381000, 2, mgdp_escher__1),
    Helper_Line_Set (0.000000, 0.000000, 3.14159, 0.659913, -0.381000, 2, mgdp_escher__2),
    Helper_Line_Set (0.000000, 0.000000, 5.23599, 0.659913, 0.381000, 2, mgdp_escher__3),
    Helper_Line_Set (0.0635000, 0.000000, 1.04720, 0.659913, -0.381000, 2, mgdp_escher__4),
    Helper_Line_Set (0.0635000, 0.000000, 5.23599, 0.659913, 0.381000, 2, mgdp_escher__5),
    Helper_Line_Set (-0.0317500, 0.0549925, 1.04720, 0.659913, -0.381000, 2, mgdp_escher__6),
    Helper_Line_Set (-0.0317500, 0.0549925, 3.14159, 0.659913, -0.381000, 2, mgdp_escher__7),
    Helper_Line_Set (-0.0317500, -0.0549925, 5.23599, 0.659913, 0.381000, 2, mgdp_escher__8),
    Helper_Line_Set (-0.0317500, -0.0549925, 3.14159, 0.659913, -0.381000, 2, mgdp_escher__9),
    Helper_Line_Set (-0.254000, 0.000000, 1.04720, 0.659913, -0.381000, 2, mgdp_escher__10),
    Helper_Line_Set (-0.254000, 0.000000, 5.23599, 0.659913, 0.381000, 2, mgdp_escher__11),
    Helper_Line_Set (0.127000, -0.219970, 1.04720, 0.659913, -0.381000, 2, mgdp_escher__12),
    Helper_Line_Set (0.127000, -0.219970, 3.14159, 0.659913, -0.381000, 2, mgdp_escher__13),
    Helper_Line_Set (0.127000, 0.219970, 5.23599, 0.659913, 0.381000, 2, mgdp_escher__14),
    Helper_Line_Set (0.127000, 0.219970, 3.14159, 0.659913, -0.381000, 2, mgdp_escher__15),
    Helper_Line_Set (0.127000, 0.109985, 0.000000, 0.659913, -0.381000, 2, mgdp_escher__16),
    Helper_Line_Set (0.127000, -0.109985, 0.000000, 0.659913, -0.381000, 2, mgdp_escher__17),
    Helper_Line_Set (0.0317500, 0.164978, 2.09440, 0.659913, 0.381000, 2, mgdp_escher__18),
    Helper_Line_Set (-0.158750, 0.0549925, 2.09440, 0.659913, 0.381000, 2, mgdp_escher__19),
    Helper_Line_Set (-0.158750, -0.0549925, 4.18879, 0.659913, 0.381000, 2, mgdp_escher__20),
    Helper_Line_Set (0.0317500, -0.164978, 4.18879, 0.659913, 0.381000, 2, mgdp_escher__21)
};


/* FLEX - Flexible material  */

static double const mgdp_lex__1 [] = {0.254000, 0.254000};
static double const mgdp_lex__2 [] = {0.0635000, 0.232210, 0.0635000, 0.359210};
static Helper_Line_Set const mgpls_lex[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.254000, 0.000000, 2, mgdp_lex__1),
    Helper_Line_Set (0.254000, 0.000000, 0.785398, 0.179605, 0.179605, 4, mgdp_lex__2)
};


/* GRASS - Grass area */

static double const mgdp_grass__1 [] = {0.119063, 0.778963};
static double const mgdp_grass__2 [] = {0.119063, 0.515938};
static double const mgdp_grass__3 [] = {0.119063, 0.515938};
static Helper_Line_Set const mgpls_grass[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.449013, 0.449013, 2, mgdp_grass__1),
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.635000, 0.000000, 2, mgdp_grass__2),
    Helper_Line_Set (0.000000, 0.000000, 2.35619, 0.635000, 0.000000, 2, mgdp_grass__3)
};


/* GRATE - Grated area  */

static Helper_Line_Set const mgpls_grate[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.119062),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.476250)
};


/* HEX - Hexagons  */

static double const mgdp_hex__1 [] = {0.146844, 0.293688};
static double const mgdp_hex__2 [] = {0.146844, 0.293688};
static double const mgdp_hex__3 [] = {0.146844, 0.293688};
static Helper_Line_Set const mgpls_hex[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.254341, 0.000000, 2, mgdp_hex__1),
    Helper_Line_Set (0.000000, 0.000000, 2.09440, 0.254341, 0.000000, 2, mgdp_hex__2),
    Helper_Line_Set (0.146844, 0.000000, 1.04720, 0.254341, 0.000000, 2, mgdp_hex__3)
};


/* HONEY - Honeycomb pattern  */

static double const mgdp_honey__1 [] = {0.150813, 0.301625};
static double const mgdp_honey__2 [] = {0.150813, 0.301625};
static double const mgdp_honey__3 [] = {0.0, 0.301625, 0.150813, 0.0};
static Helper_Line_Set const mgpls_honey[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.130607, 0.226219, 2, mgdp_honey__1),
    Helper_Line_Set (0.000000, 0.000000, 2.09440, 0.130607, 0.226219, 2, mgdp_honey__2),
    Helper_Line_Set (0.000000, 0.000000, 1.04720, 0.130607, 0.226219, 4, mgdp_honey__3)
};


/* HOUND - Houndstooth check  */

static double const mgdp_hound__1 [] = {2.54000, 1.27000};
static double const mgdp_hound__2 [] = {2.54000, 1.27000};
static Helper_Line_Set const mgpls_hound[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.158750, 0.635000, 2, mgdp_hound__1),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.158750, -0.635000, 2, mgdp_hound__2)
};


/* INSUL - Insulation material  */

static double const mgdp_insul__2 [] = {0.100000, 0.100000};
static double const mgdp_insul__3 [] = {0.100000, 0.100000};
static Helper_Line_Set const mgpls_insul[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.300000),
    Helper_Line_Set (0.000000, 0.100000, 0.000000, 0.300000, 0.000000, 2, mgdp_insul__2),
    Helper_Line_Set (0.000000, 0.200000, 0.000000, 0.300000, 0.000000, 2, mgdp_insul__3)
};


/* MUDST - Mud and sand  */

static double const mgdp_mudst__1 [] = {0.200000, 0.200000, 0.000000, 0.200000, 0.000000, 0.200000};
static Helper_Line_Set const mgpls_mudst[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000, 0.400000, 6, mgdp_mudst__1)
};


/* NET3 - Network pattern 0-60-120  */

static Helper_Line_Set const mgpls_net3[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.205978),
    Helper_Line_Set (0.000000, 0.000000, 1.04720, 0.205978),
    Helper_Line_Set (0.000000, 0.000000, 2.09440, 0.205978)
};


/* PLAST - Plastic material  */

static Helper_Line_Set const mgpls_plast[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.400000),
    Helper_Line_Set (0.000000, 0.0500000, 0.000000, 0.400000),
    Helper_Line_Set (0.000000, 0.100000, 0.000000, 0.400000)
};


/* PLASTI - Plastic material  */

static Helper_Line_Set const mgpls_plasti[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.400000),
    Helper_Line_Set (0.000000, 0.0500000, 0.000000, 0.400000),
    Helper_Line_Set (0.000000, 0.100000, 0.000000, 0.400000),
    Helper_Line_Set (0.000000, 0.250000, 0.000000, 0.400000)
};


/* SACNCR - Concrete  */

static double const mgdp_sacncr__2 [] = {0.000000, 0.178594};
static Helper_Line_Set const mgpls_sacncr[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.178594),
    Helper_Line_Set (0.126285, 0.000000, 0.785398, 0.178594, 0.000000, 2, mgdp_sacncr__2)
};


/* STARS - Star o David  */

static double const mgdp_stars__1 [] = {0.150813, 0.150813};
static double const mgdp_stars__2 [] = {0.150813, 0.150813};
static double const mgdp_stars__3 [] = {0.150813, 0.150813};
static Helper_Line_Set const mgpls_stars[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.261215, 0.000000, 2, mgdp_stars__1),
    Helper_Line_Set (0.000000, 0.000000, 1.04720, 0.261215, 0.000000, 2, mgdp_stars__2),
    Helper_Line_Set (0.0754063, 0.130607, 2.09440, 0.261215, 0.000000, 2, mgdp_stars__3)
};


/* STEEL - Steel material  */

static Helper_Line_Set const mgpls_steel[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.250007),
    Helper_Line_Set (0.000000, 0.125004, 0.785398, 0.250007)
};


/* SWAMP - Swampy area  */

static double const mgdp_swamp__1 [] = {0.180181, 1.26127};
static double const mgdp_swamp__2 [] = {0.0900906, 2.40657};
static double const mgdp_swamp__3 [] = {0.0720725, 2.42459};
static double const mgdp_swamp__4 [] = {0.0720725, 2.42459};
static double const mgdp_swamp__5 [] = {0.0576580, 1.38379};
static double const mgdp_swamp__6 [] = {0.0576580, 1.38379};
static Helper_Line_Set const mgpls_swamp[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 1.24833, 0.720725, 2, mgdp_swamp__1),
    Helper_Line_Set (0.0900906, 0.000000, 1.57080, 0.720725, 1.24833, 2, mgdp_swamp__2),
    Helper_Line_Set (0.112614, 0.000000, 1.57080, 0.720725, 1.24833, 2, mgdp_swamp__3),
    Helper_Line_Set (0.0675677, 0.000000, 1.57080, 0.720725, 1.24833, 2, mgdp_swamp__4),
    Helper_Line_Set (0.135136, 0.000000, 1.04720, 1.24833, 0.720725, 2, mgdp_swamp__5),
    Helper_Line_Set (0.0450453, 0.000000, 2.09440, 1.24833, 0.720725, 2, mgdp_swamp__6)
};


/* TRANS - Heat transer material  */

static double const mgdp_trans__2 [] = {0.100000, 0.100000};
static Helper_Line_Set const mgpls_trans[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000),
    Helper_Line_Set (0.000000, 0.100000, 0.000000, 0.200000, 0.000000, 2, mgdp_trans__2)
};


/* TRIANG - Equilateral triangles  */

static double const mgdp_triang__1 [] = {0.256223, 0.256223};
static double const mgdp_triang__2 [] = {0.256223, 0.256223};
static double const mgdp_triang__3 [] = {0.256223, 0.256223};
static Helper_Line_Set const mgpls_triang[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.04720, 0.443790, 0.256223, 2, mgdp_triang__1),
    Helper_Line_Set (0.000000, 0.000000, 2.09440, 0.443790, 0.256223, 2, mgdp_triang__2),
    Helper_Line_Set (-0.128111, 0.221895, 0.000000, 0.443790, 0.256223, 2, mgdp_triang__3)
};


/* ZIGZAG - Staircase eect  */

static double const mgdp_zigzag__1 [] = {0.200000, 0.200000};
static double const mgdp_zigzag__2 [] = {0.200000, 0.200000};
static Helper_Line_Set const mgpls_zigzag[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.200000, 0.200000, 2, mgdp_zigzag__1),
    Helper_Line_Set (0.200000, 0.000000, 1.57080, 0.200000, 0.200000, 2, mgdp_zigzag__2)
};


/* AR_B816 -  8x16 block elevation stretcher bond  */

static double const mgdp_ar_b816__2 [] = {0.249999, 0.249999};
static Helper_Line_Set const mgpls_ar_b816[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.249999),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.249999, 0.249999, 2, mgdp_ar_b816__2)
};


/* AR_B816C -  8x16 block elevation stretcher bond with mortar joints  */

static double const mgdp_ar_b816c__1 [] = {0.488355, 0.0117205};
static double const mgdp_ar_b816c__2 [] = {0.488355, 0.0117205};
static double const mgdp_ar_b816c__3 [] = {0.0, 0.261758, 0.238317, 0.0};
static double const mgdp_ar_b816c__4 [] = {0.0, 0.261758, 0.238317, 0.0};
static Helper_Line_Set const mgpls_ar_b816c[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.250038, 0.250038, 2, mgdp_ar_b816c__1),
    Helper_Line_Set (-0.250038, 0.0117205, 0.000000, 0.250038, 0.250038, 2, mgdp_ar_b816c__2),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.250038, 0.250038, 4, mgdp_ar_b816c__3),
    Helper_Line_Set (-0.0117205, 0.000000, 1.57080, 0.250038, 0.250038, 4, mgdp_ar_b816c__4)
};


/* AR_B88 -  8x8 block elevation stretcher bond  */

static double const mgdp_ar_b88__2 [] = {0.249999, 0.249999};
static Helper_Line_Set const mgpls_ar_b88[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.249999),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.125000, 0.249999, 2, mgdp_ar_b88__2)
};


/* AR_BRELM -  standard brick elevation english bond with mortar joints  */

static double const mgdp_ar_brelm__1 [] = {0.857738, 0.0421838};
static double const mgdp_ar_brelm__2 [] = {0.857738, 0.0421838};
static double const mgdp_ar_brelm__3 [] = {0.407777, 0.0421838};
static double const mgdp_ar_brelm__4 [] = {0.407777, 0.0421838};
static double const mgdp_ar_brelm__5 [] = {0.253103, 0.346920};
static double const mgdp_ar_brelm__6 [] = {0.253103, 0.346920};
static double const mgdp_ar_brelm__7 [] = {0.253103, 0.346920};
static double const mgdp_ar_brelm__8 [] = {0.253103, 0.346920};
static Helper_Line_Set const mgpls_ar_brelm[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.600025, 0.000000, 2, mgdp_ar_brelm__1),
    Helper_Line_Set (0.000000, 0.253103, 0.000000, 0.600025, 0.000000, 2, mgdp_ar_brelm__2),
    Helper_Line_Set (0.224981, 0.300011, 0.000000, 0.600025, 0.000000, 2, mgdp_ar_brelm__3),
    Helper_Line_Set (0.224981, 0.553115, 0.000000, 0.600025, 0.000000, 2, mgdp_ar_brelm__4),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.899922, 0.000000, 2, mgdp_ar_brelm__5),
    Helper_Line_Set (-0.0421838, 0.000000, 1.57080, 0.899922, 0.000000, 2, mgdp_ar_brelm__6),
    Helper_Line_Set (0.224981, 0.300011, 1.57080, 0.449961, 0.000000, 2, mgdp_ar_brelm__7),
    Helper_Line_Set (0.182797, 0.300011, 1.57080, 0.449961, 0.000000, 2, mgdp_ar_brelm__8)
};


/* AR_BRSTD -  standard brick elevation stretcher bond  */

static double const mgdp_ar_brstd__2 [] = {0.300011, 0.300011};
static Helper_Line_Set const mgpls_ar_brstd[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.300011),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.449961, 0.300011, 2, mgdp_ar_brstd__2)
};


/* AR_CONC -  random dot and stone pattern  */

static double const mgdp_ar_conc__1 [] = {0.0421862, 0.464048};
static double const mgdp_ar_conc__2 [] = {0.0337490, 0.371239};
static double const mgdp_ar_conc__3 [] = {0.0358528, 0.394380};
static double const mgdp_ar_conc__4 [] = {0.0632793, 0.696073};
static double const mgdp_ar_conc__5 [] = {0.0537791, 0.591570};
static double const mgdp_ar_conc__6 [] = {0.0506235, 0.556858};
static double const mgdp_ar_conc__7 [] = {0.0421862, 0.464048};
static double const mgdp_ar_conc__8 [] = {0.0337490, 0.371239};
static double const mgdp_ar_conc__9 [] = {0.0358528, 0.394380};
static double const mgdp_ar_conc__10 [] = {0.000000, 0.366739, 0.000000, 0.376864, 0.000000, 0.372645};
static double const mgdp_ar_conc__11 [] = {0.000000, 0.214869, 0.000000, 0.358302, 0.000000, 0.142027};
static double const mgdp_ar_conc__12 [] = {0.000000, 0.140621, 0.000000, 0.438737, 0.000000, 0.582170};
static double const mgdp_ar_conc__13 [] = {0.000000, 0.182807, 0.000000, 0.291366, 0.000000, 0.413425};
static Helper_Line_Set const mgpls_ar_conc[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.872665, -0.331748, 0.232292, 2, mgdp_ar_conc__1),
    Helper_Line_Set (0.000000, 0.000000, 6.19592, 0.414683, -0.114623, 2, mgdp_ar_conc__2),
    Helper_Line_Set (0.0336205, -0.00294141, 1.75320, -0.390350, 0.322336, 2, mgdp_ar_conc__3),
    Helper_Line_Set (0.000000, 0.112497, 0.806066, -0.497620, 0.348436, 2, mgdp_ar_conc__4),
    Helper_Line_Set (0.0500253, 0.104738, 1.68661, -0.585525, 0.483503, 2, mgdp_ar_conc__5),
    Helper_Line_Set (0.000000, 0.112497, 6.12932, 0.622024, 0.435546, 2, mgdp_ar_conc__6),
    Helper_Line_Set (0.0562483, 0.0843725, 0.366519, -0.331748, 0.232292, 2, mgdp_ar_conc__7),
    Helper_Line_Set (0.0562483, 0.0843725, 5.68977, 0.414683, -0.114623, 2, mgdp_ar_conc__8),
    Helper_Line_Set (0.0842274, 0.0655003, 1.24706, -0.390350, 0.322336, 2, mgdp_ar_conc__9),
    Helper_Line_Set (0.000000, 0.000000, 0.654498, 0.144389, 0.119415, 6, mgdp_ar_conc__10),
    Helper_Line_Set (0.000000, 0.000000, 0.130900, 0.200638, 0.175663, 6, mgdp_ar_conc__11),
    Helper_Line_Set (-0.125434, 0.000000, -0.567232, 0.150633, 0.260058, 6, mgdp_ar_conc__12),
    Helper_Line_Set (-0.181682, 0.000000, -0.741765, 0.263129, 0.203810, 6, mgdp_ar_conc__13)
};


/* AR_HBONE -  standard brick herringbone pattern @ 45 degrees  */

static double const mgdp_ar_hbone__1 [] = {0.624840, 0.208280};
static double const mgdp_ar_hbone__2 [] = {0.624840, 0.208280};
static Helper_Line_Set const mgpls_ar_hbone[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.208280, 0.208280, 2, mgdp_ar_hbone__1),
    Helper_Line_Set (0.147276, 0.147276, 2.35619, 0.208280, -0.208280, 2, mgdp_ar_hbone__2)
};


/* AR_PARQ1 -  2x12 parquet looring: pattern o 12x12  */

static double const mgdp_ar_parq1__1 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__2 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__3 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__4 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__5 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__6 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__7 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__8 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__9 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__10 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__11 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__12 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__13 [] = {0.750037, 0.750037};
static double const mgdp_ar_parq1__14 [] = {0.750037, 0.750037};
static Helper_Line_Set const mgpls_ar_parq1[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__1),
    Helper_Line_Set (0.125006, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__2),
    Helper_Line_Set (0.250012, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__3),
    Helper_Line_Set (0.375018, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__4),
    Helper_Line_Set (0.500024, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__5),
    Helper_Line_Set (0.625031, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__6),
    Helper_Line_Set (0.750037, 0.000000, 1.57080, 0.750037, 0.750037, 2, mgdp_ar_parq1__7),
    Helper_Line_Set (0.000000, 0.750037, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__8),
    Helper_Line_Set (0.000000, 0.875043, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__9),
    Helper_Line_Set (0.000000, 1.00005, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__10),
    Helper_Line_Set (0.000000, 1.12505, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__11),
    Helper_Line_Set (0.000000, 1.25006, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__12),
    Helper_Line_Set (0.000000, 1.37507, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__13),
    Helper_Line_Set (0.000000, 1.50007, 0.000000, 0.750037, 0.750037, 2, mgdp_ar_parq1__14)
};


/* AR_RROOF -  roo shingle texture  */

static double const mgdp_ar_rroo__1 [] = {4.69925, 0.626567, 1.56642, 0.313284};
static double const mgdp_ar_rroo__2 [] = {0.939851, 0.103384, 1.87970, 0.234963};
static double const mgdp_ar_rroo__3 [] = {2.50627, 0.438597, 1.25313, 0.313284};
static Helper_Line_Set const mgpls_ar_rroo[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.313284, 0.689224, 4, mgdp_ar_rroo__1),
    Helper_Line_Set (0.416667, 0.156642, 0.000000, 0.416667, -0.313284, 4, mgdp_ar_rroo__2),
    Helper_Line_Set (0.156642, 0.266291, 0.000000, 0.209900, 1.62907, 4, mgdp_ar_rroo__3)
};


/* AR_RROOF_90 -  roo shingle texture rotated 90 degrees  */

static double const mgdp_ar_rroo_90__1 [] = {4.69925, 0.626567, 1.56642, 0.313284};
static double const mgdp_ar_rroo_90__2 [] = {0.939851, 0.103384, 1.87970, 0.234963};
static double const mgdp_ar_rroo_90__3 [] = {2.50627, 0.438597, 1.25313, 0.313284};
static Helper_Line_Set const mgpls_ar_rroo_90[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.313284, 0.689224, 4, mgdp_ar_rroo_90__1),
    Helper_Line_Set (-0.156642, 0.416667, 1.57080, 0.416667, -0.313284, 4, mgdp_ar_rroo_90__2),
    Helper_Line_Set (-0.266291, 0.156642, 1.57080, 0.209900, 1.62907, 4, mgdp_ar_rroo_90__3)
};


/* AR_RSHKE -  roo wood shake texture  */

static double const mgdp_ar_rshke__1 [] = {0.250012, 0.208344, 0.291681, 0.125006, 0.375018, 0.166675};
static double const mgdp_ar_rshke__2 [] = {0.208344, 0.791705, 0.166675, 0.250012};
static double const mgdp_ar_rshke__3 [] = {0.125006, 1.29173};
static double const mgdp_ar_rshke__4 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke__5 [] = {0.468773, 1.53132};
static double const mgdp_ar_rshke__6 [] = {0.437521, 1.56258};
static double const mgdp_ar_rshke__7 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke__8 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke__9 [] = {0.458356, 1.54174};
static Helper_Line_Set const mgpls_ar_rshke[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.500024, 1.06255, 6, mgdp_ar_rshke__1),
    Helper_Line_Set (0.250012, 0.0208344, 0.000000, 0.500024, 1.06255, 4, mgdp_ar_rshke__2),
    Helper_Line_Set (0.750037, -0.0312515, 0.000000, 0.500024, 1.06255, 2, mgdp_ar_rshke__3),
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__4),
    Helper_Line_Set (0.250012, 0.000000, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__5),
    Helper_Line_Set (0.458356, 0.000000, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__6),
    Helper_Line_Set (0.750037, -0.0312515, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__7),
    Helper_Line_Set (0.875043, -0.0312515, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__8),
    Helper_Line_Set (1.25006, 0.000000, 1.57080, 0.354184, 0.500024, 2, mgdp_ar_rshke__9)
};


/* AR_RSHKE_90 -  roo wood shake texture rotated 90 degrees  */

static double const mgdp_ar_rshke_90__1 [] = {0.250012, 0.208344, 0.291681, 0.125006, 0.375018, 0.166675};
static double const mgdp_ar_rshke_90__2 [] = {0.208344, 0.791705, 0.166675, 0.250012};
static double const mgdp_ar_rshke_90__3 [] = {0.125006, 1.29173};
static double const mgdp_ar_rshke_90__4 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_90__5 [] = {0.468773, 1.53132};
static double const mgdp_ar_rshke_90__6 [] = {0.437521, 1.56258};
static double const mgdp_ar_rshke_90__7 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_90__8 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_90__9 [] = {0.458356, 1.54174};
static Helper_Line_Set const mgpls_ar_rshke_90[] = {
    Helper_Line_Set (0.000000, 0.000000, 1.57080, 0.500024, 1.06255, 6, mgdp_ar_rshke_90__1),
    Helper_Line_Set (-0.0208344, 0.250012, 1.57080, 0.500024, 1.06255, 4, mgdp_ar_rshke_90__2),
    Helper_Line_Set (0.0312515, 0.750037, 1.57080, 0.500024, 1.06255, 2, mgdp_ar_rshke_90__3),
    Helper_Line_Set (0.000000, 0.000000, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__4),
    Helper_Line_Set (0.000000, 0.250012, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__5),
    Helper_Line_Set (0.000000, 0.458356, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__6),
    Helper_Line_Set (0.0312515, 0.750037, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__7),
    Helper_Line_Set (0.0312515, 0.875043, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__8),
    Helper_Line_Set (0.000000, 1.25006, 3.14159, 0.354184, 0.500024, 2, mgdp_ar_rshke_90__9)
};


/* AR_RSHKE_180 -  roo wood shake texture rotated 180 degrees  */

static double const mgdp_ar_rshke_180__1 [] = {0.250012, 0.208344, 0.291681, 0.125006, 0.375018, 0.166675};
static double const mgdp_ar_rshke_180__2 [] = {0.208344, 0.791705, 0.166675, 0.250012};
static double const mgdp_ar_rshke_180__3 [] = {0.125006, 1.29173};
static double const mgdp_ar_rshke_180__4 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_180__5 [] = {0.468773, 1.53132};
static double const mgdp_ar_rshke_180__6 [] = {0.437521, 1.56258};
static double const mgdp_ar_rshke_180__7 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_180__8 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_180__9 [] = {0.458356, 1.54174};
static Helper_Line_Set const mgpls_ar_rshke_180[] = {
    Helper_Line_Set (0.000000, 0.000000, 3.14159, 0.500024, 1.06255, 6, mgdp_ar_rshke_180__1),
    Helper_Line_Set (-0.250012, -0.0208344, 3.14159, 0.500024, 1.06255, 4, mgdp_ar_rshke_180__2),
    Helper_Line_Set (-0.750037, 0.0312515, 3.14159, 0.500024, 1.06255, 2, mgdp_ar_rshke_180__3),
    Helper_Line_Set (0.000000, 0.000000, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__4),
    Helper_Line_Set (-0.250012, 0.000000, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__5),
    Helper_Line_Set (-0.458356, 0.000000, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__6),
    Helper_Line_Set (-0.750037, 0.0312515, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__7),
    Helper_Line_Set (-0.875043, 0.0312515, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__8),
    Helper_Line_Set (-1.25006, 0.000000, 4.71239, 0.354184, 0.500024, 2, mgdp_ar_rshke_180__9)
};


/* AR_RSHKE_270 -  roo wood shake texture rotated 270 degrees  */

static double const mgdp_ar_rshke_270__1 [] = {0.250012, 0.208344, 0.291681, 0.125006, 0.375018, 0.166675};
static double const mgdp_ar_rshke_270__2 [] = {0.208344, 0.791705, 0.166675, 0.250012};
static double const mgdp_ar_rshke_270__3 [] = {0.125006, 1.29173};
static double const mgdp_ar_rshke_270__4 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_270__5 [] = {0.468773, 1.53132};
static double const mgdp_ar_rshke_270__6 [] = {0.437521, 1.56258};
static double const mgdp_ar_rshke_270__7 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_270__8 [] = {0.479190, 1.52091};
static double const mgdp_ar_rshke_270__9 [] = {0.458356, 1.54174};
static Helper_Line_Set const mgpls_ar_rshke_270[] = {
    Helper_Line_Set (0.000000, 0.000000, 4.71239, 0.500024, 1.06255, 6, mgdp_ar_rshke_270__1),
    Helper_Line_Set (0.0208344, -0.250012, 4.71239, 0.500024, 1.06255, 4, mgdp_ar_rshke_270__2),
    Helper_Line_Set (-0.0312515, -0.750037, 4.71239, 0.500024, 1.06255, 2, mgdp_ar_rshke_270__3),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__4),
    Helper_Line_Set (0.000000, -0.250012, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__5),
    Helper_Line_Set (0.000000, -0.458356, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__6),
    Helper_Line_Set (-0.0312515, -0.750037, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__7),
    Helper_Line_Set (-0.0312515, -0.875043, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__8),
    Helper_Line_Set (0.000000, -1.25006, 0.000000, 0.354184, 0.500024, 2, mgdp_ar_rshke_270__9)
};


/* AR_SAND -  random dot pattern  */

static double const mgdp_ar_sand__1 [] = {0.000000, 0.193040, 0.000000, 0.215900, 0.000000, 0.206375};
static double const mgdp_ar_sand__2 [] = {0.000000, 0.104140, 0.000000, 0.173990, 0.000000, 0.0666750};
static double const mgdp_ar_sand__3 [] = {0.000000, 0.0635000, 0.000000, 0.228600, 0.000000, 0.298450};
static double const mgdp_ar_sand__4 [] = {0.000000, 0.0317500, 0.000000, 0.149860, 0.000000, 0.171450};
static Helper_Line_Set const mgpls_ar_sand[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.654498, 0.199009, 0.142621, 6, mgdp_ar_sand__1),
    Helper_Line_Set (0.000000, 0.000000, 0.130900, 0.326009, 0.269621, 6, mgdp_ar_sand__2),
    Helper_Line_Set (-0.156210, 0.000000, -0.567232, 0.213106, 0.333172, 6, mgdp_ar_sand__3),
    Helper_Line_Set (-0.156210, 0.000000, -0.741765, 0.340106, 0.206172, 6, mgdp_ar_sand__4)
};


/* ANSI31 - ANSI Iron, Brick, Stone masonry  */

static Helper_Line_Set const mgpls_ansi31[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.158750)
};


/* ANSI32 - ANSI Steel  */

static Helper_Line_Set const mgpls_ansi32[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.476250),
    Helper_Line_Set (0.224507, 0.000000, 0.785398, 0.476250)
};


/* ANSI33 - ANSI Bronze, Brass, Copper  */

static double const mgdp_ansi33__2 [] = {0.158750, 0.0793750};
static Helper_Line_Set const mgpls_ansi33[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.317500),
    Helper_Line_Set (0.224507, 0.000000, 0.785398, 0.317500, 0.000000, 2, mgdp_ansi33__2)
};


/* ANSI34 - ANSI Plastic, Rubber  */

static Helper_Line_Set const mgpls_ansi34[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.952500),
    Helper_Line_Set (0.224507, 0.000000, 0.785398, 0.952500),
    Helper_Line_Set (0.449013, 0.000000, 0.785398, 0.952500),
    Helper_Line_Set (0.673520, 0.000000, 0.785398, 0.952500)
};


/* ANSI35 - ANSI Fire brick, Reractory material  */

static double const mgdp_ansi35__2 [] = {0.396875, 0.0793750, 0.000000, 0.0793750};
static Helper_Line_Set const mgpls_ansi35[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.317500),
    Helper_Line_Set (0.224507, 0.000000, 0.785398, 0.317500, 0.000000, 4, mgdp_ansi35__2)
};


/* ANSI36 - ANSI Marble, Slate, Glass  */

static double const mgdp_ansi36__1 [] = {0.377031, 0.0754063, 0.000000, 0.0754063};
static Helper_Line_Set const mgpls_ansi36[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.150813, 0.263922, 4, mgdp_ansi36__1)
};


/* ANSI37 - ANSI Lead, Zinc, Magnesium, Sound/Heat/Elec Insulation  */

static Helper_Line_Set const mgpls_ansi37[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.158750),
    Helper_Line_Set (0.000000, 0.000000, 2.35619, 0.158750)
};


/* ANSI38 - ANSI Aluminum  */

static double const mgdp_ansi38__2 [] = {0.396875, 0.238125};
static Helper_Line_Set const mgpls_ansi38[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.785398, 0.158750),
    Helper_Line_Set (0.000000, 0.000000, 2.35619, 0.158750, 0.317500, 2, mgdp_ansi38__2)
};


/* ACAD_ISO02W100 -  dashed line  */

static double const mgdp_acad_iso02w100__1 [] = {0.450000, 0.112500};
static Helper_Line_Set const mgpls_acad_iso02w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 2, mgdp_acad_iso02w100__1)
};


/* ACAD_ISO03W100 -  dashed space line  */

static double const mgdp_acad_iso03w100__1 [] = {0.450000, 0.675000};
static Helper_Line_Set const mgpls_acad_iso03w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 2, mgdp_acad_iso03w100__1)
};


/* ACAD_ISO04W100 -  long dashed dotted line  */

static double const mgdp_acad_iso04w100__1 [] = {0.900000, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso04w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso04w100__1)
};


/* ACAD_ISO05W100 -  long dashed double dotted line  */

static double const mgdp_acad_iso05w100__1 [] = {0.900000, 0.112500, 0.0187500, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso05w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso05w100__1)
};


/* ACAD_ISO06W100 -  long dashed triplicate dotted line  */

static double const mgdp_acad_iso06w100__1 [] = {0.900000, 0.112500, 0.0187500, 0.112500, 0.0187500, 0.243750};
static double const mgdp_acad_iso06w100__2 [] = {0.0, 1.27500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso06w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso06w100__1),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso06w100__2)
};


/* ACAD_ISO07W100 -  dotted line  */

static double const mgdp_acad_iso07w100__1 [] = {0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso07w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 2, mgdp_acad_iso07w100__1)
};


/* ACAD_ISO08W100 -  long dashed short dashed line  */

static double const mgdp_acad_iso08w100__1 [] = {0.900000, 0.112500, 0.225000, 0.112500};
static Helper_Line_Set const mgpls_acad_iso08w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso08w100__1)
};


/* ACAD_ISO09W100 -  long dashed double-short-dashed line  */

static double const mgdp_acad_iso09w100__1 [] = {0.900000, 0.112500, 0.225000, 0.112500, 0.225000, 0.112500};
static Helper_Line_Set const mgpls_acad_iso09w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso09w100__1)
};


/* ACAD_ISO10W100 -  dashed dotted line  */

static double const mgdp_acad_iso10w100__1 [] = {0.450000, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso10w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso10w100__1)
};


/* ACAD_ISO11W100 -  double-dashed dotted line  */

static double const mgdp_acad_iso11w100__1 [] = {0.450000, 0.112500, 0.450000, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso11w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso11w100__1)
};


/* ACAD_ISO12W100 -  dashed double-dotted line  */

static double const mgdp_acad_iso12w100__1 [] = {0.450000, 0.112500, 0.0187500, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso12w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso12w100__1)
};


/* ACAD_ISO13W100 -  double-dashed double-dotted line  */

static double const mgdp_acad_iso13w100__1 [] = {0.450000, 0.112500, 0.450000, 0.112500, 0.0187500, 0.243750};
static double const mgdp_acad_iso13w100__2 [] = {0.0, 1.25625, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso13w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso13w100__1),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso13w100__2)
};


/* ACAD_ISO14W100 -  dashed triplicate-dotted line  */

static double const mgdp_acad_iso14w100__1 [] = {0.450000, 0.112500, 0.0187500, 0.112500, 0.0187500, 0.243750};
static double const mgdp_acad_iso14w100__2 [] = {0.0, 0.825000, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso14w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso14w100__1),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 4, mgdp_acad_iso14w100__2)
};


/* ACAD_ISO15W100 -  double-dashed triplicate-dotted line  */

static double const mgdp_acad_iso15w100__1 [] = {0.450000, 0.112500, 0.450000, 0.112500, 0.0187500, 0.375000};
static double const mgdp_acad_iso15w100__2 [] = {0.0, 1.25625, 0.0187500, 0.112500, 0.0187500, 0.112500};
static Helper_Line_Set const mgpls_acad_iso15w100[] = {
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso15w100__1),
    Helper_Line_Set (0.000000, 0.000000, 0.000000, 0.187500, 0.000000, 6, mgdp_acad_iso15w100__2)
};


static Helper_Hatch_Pattern const Helper_Hatch_Patterns[] = {
    Helper_Hatch_Pattern ( 2, mgpls_net),
    Helper_Hatch_Pattern ( 2, mgpls_net_45),
    Helper_Hatch_Pattern ( 1, mgpls_line),
    Helper_Hatch_Pattern ( 1, mgpls_line_45),
    Helper_Hatch_Pattern ( 1, mgpls_line_90),
    Helper_Hatch_Pattern ( 1, mgpls_line_135),
    Helper_Hatch_Pattern ( 2, mgpls_square),
    Helper_Hatch_Pattern ( 2, mgpls_angle),
    Helper_Hatch_Pattern ( 8, mgpls_box),
    Helper_Hatch_Pattern ( 2, mgpls_brass),
    Helper_Hatch_Pattern ( 3, mgpls_brick),
    Helper_Hatch_Pattern ( 8, mgpls_brstone),
    Helper_Hatch_Pattern ( 4, mgpls_clay),
    Helper_Hatch_Pattern ( 4, mgpls_cork),
    Helper_Hatch_Pattern ( 2, mgpls_cross),
    Helper_Hatch_Pattern ( 1, mgpls_dash),
    Helper_Hatch_Pattern ( 2, mgpls_dolmit),
    Helper_Hatch_Pattern ( 1, mgpls_dots),
    Helper_Hatch_Pattern ( 6, mgpls_earth),
    Helper_Hatch_Pattern (21, mgpls_escher),
    Helper_Hatch_Pattern ( 2, mgpls_lex),
    Helper_Hatch_Pattern ( 3, mgpls_grass),
    Helper_Hatch_Pattern ( 2, mgpls_grate),
    Helper_Hatch_Pattern ( 3, mgpls_hex),
    Helper_Hatch_Pattern ( 3, mgpls_honey),
    Helper_Hatch_Pattern ( 2, mgpls_hound),
    Helper_Hatch_Pattern ( 3, mgpls_insul),
    Helper_Hatch_Pattern ( 1, mgpls_mudst),
    Helper_Hatch_Pattern ( 3, mgpls_net3),
    Helper_Hatch_Pattern ( 3, mgpls_plast),
    Helper_Hatch_Pattern ( 4, mgpls_plasti),
    Helper_Hatch_Pattern ( 2, mgpls_sacncr),
    Helper_Hatch_Pattern ( 3, mgpls_stars),
    Helper_Hatch_Pattern ( 2, mgpls_steel),
    Helper_Hatch_Pattern ( 6, mgpls_swamp),
    Helper_Hatch_Pattern ( 2, mgpls_trans),
    Helper_Hatch_Pattern ( 3, mgpls_triang),
    Helper_Hatch_Pattern ( 2, mgpls_zigzag),
    Helper_Hatch_Pattern ( 2, mgpls_ar_b816),
    Helper_Hatch_Pattern ( 4, mgpls_ar_b816c),
    Helper_Hatch_Pattern ( 2, mgpls_ar_b88),
    Helper_Hatch_Pattern ( 8, mgpls_ar_brelm),
    Helper_Hatch_Pattern ( 2, mgpls_ar_brstd),
    Helper_Hatch_Pattern (13, mgpls_ar_conc),
    Helper_Hatch_Pattern ( 2, mgpls_ar_hbone),
    Helper_Hatch_Pattern (14, mgpls_ar_parq1),
    Helper_Hatch_Pattern ( 3, mgpls_ar_rroo),
    Helper_Hatch_Pattern ( 3, mgpls_ar_rroo_90),
    Helper_Hatch_Pattern ( 9, mgpls_ar_rshke),
    Helper_Hatch_Pattern ( 9, mgpls_ar_rshke_90),
    Helper_Hatch_Pattern ( 9, mgpls_ar_rshke_180),
    Helper_Hatch_Pattern ( 9, mgpls_ar_rshke_270),
    Helper_Hatch_Pattern ( 4, mgpls_ar_sand),
    Helper_Hatch_Pattern ( 1, mgpls_ansi31),
    Helper_Hatch_Pattern ( 2, mgpls_ansi32),
    Helper_Hatch_Pattern ( 2, mgpls_ansi33),
    Helper_Hatch_Pattern ( 4, mgpls_ansi34),
    Helper_Hatch_Pattern ( 2, mgpls_ansi35),
    Helper_Hatch_Pattern ( 1, mgpls_ansi36),
    Helper_Hatch_Pattern ( 2, mgpls_ansi37),
    Helper_Hatch_Pattern ( 2, mgpls_ansi38),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso02w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso03w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso04w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso05w100),
    Helper_Hatch_Pattern ( 2, mgpls_acad_iso06w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso07w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso08w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso09w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso10w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso11w100),
    Helper_Hatch_Pattern ( 1, mgpls_acad_iso12w100),
    Helper_Hatch_Pattern ( 2, mgpls_acad_iso13w100),
    Helper_Hatch_Pattern ( 2, mgpls_acad_iso14w100),
    Helper_Hatch_Pattern ( 2, mgpls_acad_iso15w100)
};

static const char* Helper_Hatch_Pattern_Names[] = 
{
            "Net",              //  0
            "Net_45",            //  1
            "Line",             //  2
            "Line_45",          //  3
            "Line_90",          //  4
            "Line_135",         //  5
            "Square",           //  6
            "Angle",            //  7
            "Box",              //  8
            "Brass",            //  9
            "Brick",            // 10
            "Brstone",          // 11
            "Clay",             // 12
            "Cork",             // 13
            "Cross",            // 14
            "Dash",             // 15
            "Dolmit",           // 16
            "Dots",             // 17
            "Earth",            // 18
            "Escher",           // 19
            "Flex",             // 20
            "Grass",            // 21
            "Grate",            // 22
            "Hex",              // 23
            "Honey",            // 24
            "Hound",            // 25
            "Insul",            // 26
            "Mudst",            // 27
            "Net3",             // 28
            "Plast",            // 29
            "Plasti",           // 30
            "Sacncr",           // 31
            "Stars",            // 32
            "Steel",            // 33
            "Swamp",            // 34
            "Trans",            // 35
            "Triang",           // 36
            "Zigzag",           // 37
            "Ar_b816",          // 38
            "Ar_b816c",         // 39
            "Ar_b88",           // 40
            "Ar_brelm",         // 41
            "Ar_brstd",         // 42
            "Ar_conc",          // 43
            "Ar_hbone",         // 44
            "Ar_parq1",         // 45
            "Ar_rroo",         // 46
            "Ar_rroo_90",      // 47
            "Ar_rshke",         // 48
            "Ar_rshke_90",      // 49
            "Ar_rshke_180",     // 50
            "Ar_rshke_270",     // 51
            "Ar_sand",          // 52
            "Ansi31",           // 53
            "Ansi32",           // 54
            "Ansi33",           // 55
            "Ansi34",           // 56
            "Ansi35",           // 57
            "Ansi36",           // 58
            "Ansi37",           // 59
            "Ansi38",           // 60
            "Acad_iso02w100",   // 61
            "Acad_iso03w100",   // 62
            "Acad_iso04w100",   // 63
            "Acad_iso05w100",   // 64
            "Acad_iso06w100",   // 65
            "Acad_iso07w100",   // 66
            "Acad_iso08w100",   // 67
            "Acad_iso09w100",   // 68
            "Acad_iso10w100",   // 69
            "Acad_iso11w100",   // 70
            "Acad_iso12w100",   // 71
            "Acad_iso13w100",   // 72
            "Acad_iso14w100",   // 73
            "Acad_iso15w100"   // 74
};

