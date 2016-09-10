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

// EMap MapGuide fill pattern set for Heidi, version 1.0.
// call by, for example, hr.set_face_pattern(mg_fillPatterns[UserFillPattern::Shapefill12]);

static WT_Byte const shapefill12BitmapData [8] =
    {0xFF, 0xEF, 0xFF, 0xFE, 0xFF, 0xEF, 0xFF, 0xFE};
static WT_Byte const shapefill13BitmapData [8] =
    {0xFF, 0xEE, 0xFF, 0xBB, 0xFF, 0xEE, 0xFF, 0xBB};
static WT_Byte const shapefill14BitmapData [8] =
    {0xEE, 0xBB, 0xEE, 0xBB, 0xEE, 0xBB, 0xEE, 0xBB};
static WT_Byte const shapefill15BitmapData [8] =
    {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
static WT_Byte const shapefill16BitmapData [8] =
    {0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11};
static WT_Byte const shapefill17BitmapData [8] =
    {0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00, 0x11};
static WT_Byte const shapefill18BitmapData [8] =
    {0x00, 0x10, 0x00, 0x01, 0x00, 0x10, 0x00, 0x01};
static WT_Byte const shapefill19BitmapData [8] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static WT_Byte const shapefill20BitmapData [8] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static WT_Byte const shapefill21BitmapData [8] =
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF};
static WT_Byte const shapefill22BitmapData [8] =
    {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF};
static WT_Byte const shapefill23BitmapData [8] =
    {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};
static WT_Byte const shapefill24BitmapData [8] =
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
static WT_Byte const shapefill25BitmapData [8] =
    {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
static WT_Byte const shapefill26BitmapData [8] =
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
static WT_Byte const shapefill27BitmapData [8] =
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
static WT_Byte const shapefill28BitmapData [8] =
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
static WT_Byte const shapefill29BitmapData [8] =
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static WT_Byte const shapefill30BitmapData [8] =
    {0x81, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0};
static WT_Byte const shapefill31BitmapData [8] =
    {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
static WT_Byte const shapefill32BitmapData [8] =
    {0x99, 0x33, 0x66, 0xCC, 0x99, 0x33, 0x66, 0xCC};
static WT_Byte const shapefill33BitmapData [8] =
    {0xEE, 0xDD, 0xBB, 0x77, 0xEE, 0xDD, 0xBB, 0x77};
static WT_Byte const shapefill34BitmapData [8] =
    {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
static WT_Byte const shapefill35BitmapData [8] =
    {0x81, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03};
static WT_Byte const shapefill36BitmapData [8] =
    {0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11};
static WT_Byte const shapefill37BitmapData [8] =
    {0x99, 0xCC, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33};
static WT_Byte const shapefill38BitmapData [8] =
    {0x77, 0xBB, 0xDD, 0xEE, 0x77, 0xBB, 0xDD, 0xEE};
static WT_Byte const shapefill39BitmapData [8] =
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF};
static WT_Byte const shapefill40BitmapData [8] =
    {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF};
static WT_Byte const shapefill41BitmapData [8] =
    {0x11, 0x11, 0x11, 0xFF, 0x11, 0x11, 0x11, 0xFF};
static WT_Byte const shapefill42BitmapData [8] =
    {0x33, 0x33, 0xFF, 0xFF, 0x33, 0x33, 0xFF, 0xFF};
static WT_Byte const shapefill43BitmapData [8] =
    {0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF};
static WT_Byte const shapefill44BitmapData [8] =
    {0x80, 0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41};
static WT_Byte const shapefill45BitmapData [8] =
    {0x81, 0xC3, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0xC3};
static WT_Byte const shapefill46BitmapData [8] =
    {0x88, 0x55, 0x22, 0x55, 0x88, 0x55, 0x22, 0x55};
static WT_Byte const shapefill47BitmapData [8] =
    {0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33};
static WT_Byte const shapefill48BitmapData [8] =
    {0xB1, 0x30, 0x03, 0x1B, 0xD8, 0xC0, 0x0C, 0x8D};
static WT_Byte const shapefill49BitmapData [8] =
    {0x20, 0x02, 0x10, 0x80, 0x04, 0x40, 0x08, 0x01};
static WT_Byte const shapefill50BitmapData [8] =
    {0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55};
static WT_Byte const shapefill51BitmapData [8] =
    {0x00, 0x10, 0x20, 0x40, 0x00, 0x04, 0x02, 0x01};
static WT_Byte const shapefill52BitmapData [8] =
    {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01};
static WT_Byte const shapefill53BitmapData [8] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static WT_Byte const shapefill54BitmapData [8] =
    {0x80, 0x80, 0x80, 0x41, 0x22, 0x9C, 0x22, 0x41};
static WT_Byte const shapefill55BitmapData [8] =
    {0x8E, 0x44, 0xE8, 0xF1, 0xE2, 0x44, 0x2E, 0x1F};
static WT_Byte const shapefill56BitmapData [8] =
    {0x20, 0x20, 0x50, 0xAA, 0x02, 0x02, 0x05, 0xAA};
static WT_Byte const shapefill57BitmapData [8] =
    {0x40, 0xA0, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
static WT_Byte const shapefill58BitmapData [8] =
    {0x0D, 0x0D, 0x0D, 0x0D, 0xFD, 0xFD, 0x00, 0xFD};
static WT_Byte const shapefill59BitmapData [8] =
    {0x10, 0x10, 0x10, 0xFF, 0x01, 0x01, 0x01, 0xFF};
static WT_Byte const shapefill60BitmapData [8] =
    {0x00, 0x01, 0x00, 0x11, 0x00, 0x01, 0x00, 0x55};
static WT_Byte const shapefill61BitmapData [8] =
    {0x20, 0x40, 0x80, 0x01, 0x83, 0x44, 0x38, 0x10};
static WT_Byte const shapefill62BitmapData [8] =
    {0x00, 0x55, 0x00, 0x11, 0x82, 0x44, 0x28, 0x11};
static WT_Byte const shapefill63BitmapData [8] =
    {0x00, 0x00, 0x50, 0x20, 0x00, 0x00, 0x05, 0x02};
static WT_Byte const shapefill64BitmapData [8] =
    {0x80, 0x80, 0x40, 0x30, 0x0C, 0x12, 0x21, 0xC0};
static WT_Byte const shapefill65BitmapData [8] =
    {0xC7, 0x28, 0x10, 0x10, 0x7C, 0x82, 0x01, 0x01};
static WT_Byte const shapefill66BitmapData [8] =
    {0x10, 0x20, 0x40, 0xFF, 0x55, 0x2A, 0x04, 0x08};
static WT_Byte const shapefill67BitmapData [8] =
    {0x1F, 0x1F, 0x19, 0xEE, 0xF1, 0xF1, 0x91, 0xEE};
static WT_Byte const shapefill68BitmapData [8] =
    {0x10, 0x28, 0x54, 0xAA, 0x54, 0x28, 0x10, 0x00};
static WT_Byte const shapefill69BitmapData [8] =
    {0x22, 0x49, 0x9C, 0x39, 0x92, 0x44, 0x28, 0x14};
static WT_Byte const shapefill70BitmapData [8] =
    {0x00, 0x06, 0x06, 0x06, 0x00, 0x60, 0x60, 0x60};
static WT_Byte const shapefill71BitmapData [8] =
    {0x8F, 0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88};


static const WT_Byte* Helper_Fill_Patterns[] = {
         shapefill12BitmapData ,
         shapefill13BitmapData ,
         shapefill14BitmapData ,
         shapefill15BitmapData ,
         shapefill16BitmapData ,
         shapefill17BitmapData ,
         shapefill18BitmapData ,
         shapefill19BitmapData ,
         shapefill20BitmapData ,
         shapefill21BitmapData ,
         shapefill22BitmapData ,
         shapefill23BitmapData ,
         shapefill24BitmapData ,
         shapefill25BitmapData ,
         shapefill26BitmapData ,
         shapefill27BitmapData ,
         shapefill28BitmapData ,
         shapefill29BitmapData ,
         shapefill30BitmapData ,
         shapefill31BitmapData ,
         shapefill32BitmapData ,
         shapefill33BitmapData ,
         shapefill34BitmapData ,
         shapefill35BitmapData ,
         shapefill36BitmapData ,
         shapefill37BitmapData ,
         shapefill38BitmapData ,
         shapefill39BitmapData ,
         shapefill40BitmapData ,
         shapefill41BitmapData ,
         shapefill42BitmapData ,
         shapefill43BitmapData ,
         shapefill44BitmapData ,
         shapefill45BitmapData ,
         shapefill46BitmapData ,
         shapefill47BitmapData ,
         shapefill48BitmapData ,
         shapefill49BitmapData ,
         shapefill50BitmapData ,
         shapefill51BitmapData ,
         shapefill52BitmapData ,
         shapefill53BitmapData ,
         shapefill54BitmapData ,
         shapefill55BitmapData ,
         shapefill56BitmapData ,
         shapefill57BitmapData ,
         shapefill58BitmapData ,
         shapefill59BitmapData ,
         shapefill60BitmapData ,
         shapefill61BitmapData ,
         shapefill62BitmapData ,
         shapefill63BitmapData ,
         shapefill64BitmapData ,
         shapefill65BitmapData ,
         shapefill66BitmapData ,
         shapefill67BitmapData ,
         shapefill68BitmapData ,
         shapefill69BitmapData ,
         shapefill70BitmapData ,
         shapefill71BitmapData 
};

static const char* Helper_Fill_Pattern_Names[] = {
         "SHAPEFILL12" ,
         "SHAPEFILL13" ,
         "SHAPEFILL14" ,
         "SHAPEFILL15" ,
         "SHAPEFILL16" ,
         "SHAPEFILL17" ,
         "SHAPEFILL18" ,
         "SHAPEFILL19" ,
         "SHAPEFILL20" ,
         "SHAPEFILL21" ,
         "SHAPEFILL22" ,
         "SHAPEFILL23" ,
         "SHAPEFILL24" ,
         "SHAPEFILL25" ,
         "SHAPEFILL26" ,
         "SHAPEFILL27" ,
         "SHAPEFILL28" ,
         "SHAPEFILL29" ,
         "SHAPEFILL30" ,
         "SHAPEFILL31" ,
         "SHAPEFILL32" ,
         "SHAPEFILL33" ,
         "SHAPEFILL34" ,
         "SHAPEFILL35" ,
         "SHAPEFILL36" ,
         "SHAPEFILL37" ,
         "SHAPEFILL38" ,
         "SHAPEFILL39" ,
         "SHAPEFILL40" ,
         "SHAPEFILL41" ,
         "SHAPEFILL42" ,
         "SHAPEFILL43" ,
         "SHAPEFILL44" ,
         "SHAPEFILL45" ,
         "SHAPEFILL46" ,
         "SHAPEFILL47" ,
         "SHAPEFILL48" ,
         "SHAPEFILL49" ,
         "SHAPEFILL50" ,
         "SHAPEFILL51" ,
         "SHAPEFILL52" ,
         "SHAPEFILL53" ,
         "SHAPEFILL54" ,
         "SHAPEFILL55" ,
         "SHAPEFILL56" ,
         "SHAPEFILL57" ,
         "SHAPEFILL58" ,
         "SHAPEFILL59" ,
         "SHAPEFILL60" ,
         "SHAPEFILL61" ,
         "SHAPEFILL62" ,
         "SHAPEFILL63" ,
         "SHAPEFILL64" ,
         "SHAPEFILL65" ,
         "SHAPEFILL66" ,
         "SHAPEFILL67" ,
         "SHAPEFILL68" ,
         "SHAPEFILL69" ,
         "SHAPEFILL70" ,
         "SHAPEFILL71" 
};

struct Helper_Fill_Pattern {
    public:
        enum Enum {
            Shapefill12,      //  0
            Shapefill13,      //  1
            Shapefill14,      //  2
            Shapefill15,      //  3
            Shapefill16,      //  4
            Shapefill17,      //  5
            Shapefill18,      //  6
            Shapefill19,      //  7
            Shapefill20,      //  8
            Shapefill21,      //  9
            Shapefill22,      // 10
            Shapefill23,      // 11
            Shapefill24,      // 12
            Shapefill25,      // 13
            Shapefill26,      // 14
            Shapefill27,      // 15
            Shapefill28,      // 16
            Shapefill29,      // 17
            Shapefill30,      // 18
            Shapefill31,      // 19
            Shapefill32,      // 20
            Shapefill33,      // 21
            Shapefill34,      // 22
            Shapefill35,      // 23
            Shapefill36,      // 24
            Shapefill37,      // 25
            Shapefill38,      // 26
            Shapefill39,      // 27
            Shapefill40,      // 28
            Shapefill41,      // 29
            Shapefill42,      // 30
            Shapefill43,      // 31
            Shapefill44,      // 32
            Shapefill45,      // 33
            Shapefill46,      // 34
            Shapefill47,      // 35
            Shapefill48,      // 36
            Shapefill49,      // 37
            Shapefill50,      // 38
            Shapefill51,      // 39
            Shapefill52,      // 40
            Shapefill53,      // 41
            Shapefill54,      // 42
            Shapefill55,      // 43
            Shapefill56,      // 44
            Shapefill57,      // 45
            Shapefill58,      // 46
            Shapefill59,      // 47
            Shapefill60,      // 48
            Shapefill61,      // 49
            Shapefill62,      // 50
            Shapefill63,      // 51
            Shapefill64,      // 52
            Shapefill65,      // 53
            Shapefill66,      // 54
            Shapefill67,      // 55
            Shapefill68,      // 56
            Shapefill69,      // 57
            Shapefill70,      // 58
            Shapefill71,      // 59
            count
        };
};
