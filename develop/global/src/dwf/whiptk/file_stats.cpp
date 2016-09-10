//  Copyright (c) 1996-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/file_stats.cpp 9     7/05/05 8:56p Bangiav $


#include "whiptk/pch.h"

// ==========================================================================
// The WT_File_Stats class tracks what opcodes are being parsed by the reader
// and keeps info such as how big those opcodes are.
// Description strings for stats reporting...

static char const * g_DWF_Opcode_Descriptions[WD_MAX_OPCODE_ID + 1] = {
        ""                                     // 0x00
        ,""                                    // 0x01
        ,"SBBO_BEZIER_16R"                     // 0x02
        ,"SBBO_SET_COLOR_RGBA"                 // 0x03 // Ctrl-C
        ,""                                    // 0x04
        ,""                                    // 0x05
        ,"SBBO_SET_FONT"                       // 0x06 // Ctrl-F
        ,"SBBO_DRAW_GOURAUD_POLYTRIANGLE_16R"  // 0x07 // Ctrl-G
        ,""                                    // 0x08
        ,""                                    // 0x09
        ,""                                    // 0x0A
        ,"SBBO_DRAW_CONTOUR_SET_16R"           // 0x0B // Ctrl-K
        ,"SBBO_DRAW_LINE_16R"                  // 0x0C // Ctrl-L
        ,""                                    // 0x0D
        ,"SBBO_OBJECT_NODE_AUTO"               // 0x0E // Ctrl-N
        ,""                                    // 0x0F
        ,"SBBO_DRAW_POLYLINE_POLYGON_16R"      // 0x10 // Ctrl-P
        ,"SBBO_DRAW_GOURAUD_POLYLINE_16R"      // 0x11 // Ctrl-Q
        ,"SBBO_DRAW_CIRCLE_16R"                // 0x12 // Ctrl-R
        ,""                                    // 0x13
        ,"SBBO_DRAW_POLYTRIANGLE_16R"          // 0x14 // Ctrl-T
        ,""                                    // 0x15
        ,""                                    // 0x16
        ,"SBBO_SET_LINE_WEIGHT"                // 0x17 // Ctrl-W
        ,"SBBO_DRAW_TEXT_COMPLEX"              // 0x18 // Ctrl-X
        ,""                                    // 0x19
        ,""                                    // 0x1A
        ,""                                    // 0X1B
        ,""                                    // 0X1C
        ,""                                    // 0X1D
        ,""                                    // 0X1E
        ,""                                    // 0X1F
        ,""                                    // 0X20
        ,""                                    // 0X21
        ,""                                    // 0X22
        ,""                                    // 0X23
        ,""                                    // 0X24
        ,""                                    // 0X25
        ,""                                    // 0X26
        ,""                                    // 0X27
        ,""                                    // 0X28
        ,""                                    // 0X29
        ,""                                    // 0X2A
        ,""                                    // 0X2B
        ,""                                    // 0X2C
        ,""                                    // 0X2D
        ,""                                    // 0X2E
        ,""                                    // 0X2F
        ,""                                    // 0X30
        ,""                                    // 0X31
        ,""                                    // 0X32
        ,""                                    // 0X33
        ,""                                    // 0X34
        ,""                                    // 0X35
        ,""                                    // 0X36
        ,""                                    // 0X37
        ,""                                    // 0X38
        ,""                                    // 0X39
        ,""                                    // 0X3A
        ,""                                    // 0X3B
        ,""                                    // 0X3C
        ,""                                    // 0X3D
        ,""                                    // 0X3E
        ,""                                    // 0X3F
        ,""                                    // 0X40
        ,""                                    // 0X41
        ,"SBAO_BEZIER_32R"                     // 'B'  // 0X42
        ,"SBAO_SET_COLOR_INDEXED"              // 'C'  // 0x43
        ,""                                    // 0X44
        ,"SBAO_DRAW_ELLIPSE"                   // 'E'  // 0x45
        ,"SBAO_SET_FILL_ON"                    // 'F'  // 0x46
        ,"SBAO_SET_MACRO_INDEX"                // 'G'  // 0X47
        ,""                                    // 0X48
        ,""                                    // 0X49
        ,""                                    // 0X4A
        ,""                                    // 0X4B
        ,"SBAO_DRAW_LINE"                      // 'L'  // 0x4C
        ,"SBAO_DRAW_MACRO_DRAW"                // 'M'  // 0x4D
        ,"SBBO_OBJECT_NODE_32"                 // 'N'  // 0X4E
        ,"SBBO_SET_ORIGIN_32"                  // 'O'  // 0x4F
        ,"SBAO_DRAW_POLYLINE_POLYGON"          // 'P'  // 0x50
        ,""                                    // 0X51
        ,"SBAO_DRAW_CIRCLE"                    // 'R'  // 0x52
        ,"SBAO_SET_MACRO_SIZE"                 // 'S'  // 0x53
        ,"SBAO_DRAW_POLYTRIANGLE"              // 'T'  // 0x54
        ,""                                    // 0X55
        ,"SBAO_SET_VISIBILITY_ON"              // 'V'  // 0x56
        ,""                                    // 0X57
        ,""                                    // 0X58
        ,""                                    // 0X59
        ,""                                    // 0X5A
        ,""                                    // 0X5B
        ,""                                    // 0X5C
        ,""                                    // 0X5D
        ,""                                    // 0X5E
        ,""                                    // 0X5F
        ,""                                    // 0X60
        ,""                                    // 0X61
        ,"SBBO_BEZIER_32"                      // 'b'  // 0X62
        ,"SBBO_SET_COLOR_INDEXED"              // 'c'  // 0x63
        ,""                                    // 0X64
        ,"SBBO_DRAW_ELLIPSE_32R"               // 'e'  // 0x65
        ,"SBAO_SET_FILL_OFF"                   // 'f'  // 0x66
        ,"SBBO_DRAW_GOURAUD_POLYTRIANGLE_32R"  // 'g'  // 0x67
        ,""                                    // 0X68
        ,""                                    // 'i'  // 0X69
        ,""                                    // 0X6A
        ,"SBBO_DRAW_CONTOUR_SET_32R"           // 'k'  // 0x6B
        ,"SBBO_DRAW_LINE_32R"                  // 'l'  // 0x6C
        ,"SBBO_DRAW_MACRO_DRAW_32R"            // 'm'  // 0x6D
        ,"SBBO_OBJECT_NODE_16"                 // 'n'  // 0X6E
        ,""                                    // 0X6F
        ,"SBBO_DRAW_POLYLINE_POLYGON_32R"      // 'p'  // 0x70
        ,"SBBO_DRAW_GOURAUD_POLYLINE_32R"      // 'q'  // 0x71
        ,"SBBO_DRAW_CIRCLE_32R"                // 'r'  // 0x72
        ,"SBBO_SET_MACRO_SIZE"                 // 's'  // 0x73
        ,"SBBO_DRAW_POLYTRIANGLE_32R"          // 't'  // 0x74
        ,""                                    // 0X75
        ,"SBAO_SET_VISIBILITY_OFF"             // 'v'  // 0x76
        ,""                                    // 0x77
        ,"SBBO_DRAW_TEXT_BASIC"                // 'x'  // 0x78
        ,""                                    // 0X79
        ,""                                    // 0X7A
        ,""                                    // 0X7B
        ,""                                    // 0X7C
        ,""                                    // 0X7D
        ,""                                    // 0X7E
        ,""                                    // 0X7F
        ,""                                    // 0X80
        ,""                                    // 0X81
        ,""                                    // 0X82
        ,""                                    // 0X83
        ,""                                    // 0X84
        ,""                                    // 0X85
        ,""                                    // 0X86
        ,""                                    // 0X87
        ,""                                    // 0X88
        ,""                                    // 0X89
        ,""                                    // 0X8A
        ,""                                    // 0X8B
        ,""                                    // 0X8C
        ,"SBBO_DRAW_MACRO_DRAW_16R"            // 0x8D // This SHOULD have been 0x0D, but we are stuck now with 0x8D
        ,""                                    // 0X8E
        ,""                                    // 0X8F
        ,""                                    // 0X90
        ,""                                    // 0X91
        ,"SBBO_DRAW_CIRCULAR_ARC_32R"          // 0x92 // XTENDED-Ctrl-R
        ,""                                    // 0X93
        ,""                                    // 0X94
        ,""                                    // 0X95
        ,""                                    // 0X96
        ,""                                    // 0X97
        ,""                                    // 0X98
        ,""                                    // 0X99
        ,""                                    // 0X9A
        ,""                                    // 0X9B
        ,""                                    // 0X9C
        ,""                                    // 0X9D
        ,""                                    // 0X9E
        ,""                                    // 0X9F
        ,""                                    // 0XA0
        ,""                                    // 0XA1
        ,""                                    // 0XA2
        ,""                                    // 0XA3
        ,""                                    // 0XA4
        ,""                                    // 0XA5
        ,""                                    // 0XA6
        ,""                                    // 0XA7
        ,""                                    // 0XA8
        ,""                                    // 0XA9
        ,""                                    // 0XAA
        ,""                                    // 0XAB
        ,"SBBO_SET_LAYER"                      // 0xAC // High-order-bit + Ctrl-L
        ,""                                    // 0XAD
        ,""                                    // 0XAE
        ,""                                    // 0XAF
        ,""                                    // 0XB0
        ,""                                    // 0XB1
        ,""                                    // 0XB2
        ,""                                    // 0XB3
        ,""                                    // 0XB4
        ,""                                    // 0XB5
        ,""                                    // 0XB6
        ,""                                    // 0XB7
        ,""                                    // 0XB8
        ,""                                    // 0XB9
        ,""                                    // 0XBA
        ,""                                    // 0XBB
        ,""                                    // 0XBC
        ,""                                    // 0XBD
        ,""                                    // 0XBE
        ,""                                    // 0XBF
        ,""                                    // 0XC0
        ,""                                    // 0XC1
        ,""                                    // 0XC2
        ,""                                    // 0XC3
        ,""                                    // 0XC4
        ,""                                    // 0XC5
        ,""                                    // 0XC6
        ,""                                    // 0XC7
        ,""                                    // 0XC8
        ,""                                    // 0XC9
        ,""                                    // 0XCA
        ,""                                    // 0XCB
        ,"SBBO_SET_LINE_PATTERN"               // 0xCC // Dbl-extnd + Ctrl-L
        ,""                                    // 0XCD
        ,""                                    // 0XCE
        ,""                                    // 0XCF
        ,""                                    // 0XD0
        ,""                                    // 0XD1
        ,""                                    // 0XD2
        ,""                                    // 0XD3
        ,""                                    // 0XD4
        ,""                                    // 0XD5
        ,""                                    // 0XD6
        ,""                                    // 0XD7
        ,""                                    // 0XD8
        ,""                                    // 0XD9
        ,""                                    // 0XDA
        ,""                                    // 0XDB
        ,""                                    // 0XDC
        ,""                                    // 0XDD
        ,""                                    // 0XDE
        ,""                                    // 0XDF
        ,""                                    // 0XE0
        ,""                                    // 0XE1
        ,""                                    // 0XE2
        ,""                                    // 0XE3
        ,""                                    // 0XE4
        ,""                                    // 0XE5
        ,""                                    // 0XE6
        ,""                                    // 0XE7
        ,""                                    // 0XE8
        ,""                                    // 0XE9
        ,""                                    // 0XEA
        ,""                                    // 0XEB
        ,""                                    // 0XEC
        ,""                                    // 0XED
        ,""                                    // 0XEE
        ,""                                    // 0XEF
        ,""                                    // 0XF0
        ,""                                    // 0XF1
        ,""                                    // 0XF2
        ,""                                    // 0XF3
        ,""                                    // 0XF4
        ,""                                    // 0XF5
        ,""                                    // 0XF6
        ,""                                    // 0XF7
        ,""                                    // 0XF8
        ,""                                    // 0XF9
        ,""                                    // 0XFA
        ,""                                    // 0XFB
        ,""                                    // 0XFC
        ,""                                    // 0XFD
        ,""                                    // 0XFE
        ,""                                    // 0XFF
        ,"EXAO_DEFINE_AUTHOR"                  // 256
        ,"EXAO_SET_BACKGROUND"                 // 257
        ,"EXAO_DRAW_CIRCLE"                    // 258
        ,"EXAO_DRAW_CONTOUR"                   // 259
        ,"EXAO_SET_COLOR"                      // 260
        ,"EXAO_SET_COLOR_MAP"                  // 261
        ,"EXAO_DEFINE_COMMENTS"                // 262
        ,"EXAO_DEFINE_COPYRIGHT"               // 263
        ,"EXAO_DEFINE_CREATOR"                 // 264
        ,"EXAO_DEFINE_CREATION_TIME"           // 265
        ,"EXAO_SET_CODE_PAGE"                  // 266
        ,"EXAO_SET_DASH_PATTERN"               // 267
        ,"EXAO_DEFINE_DWF_HEADER"              // 268
        ,"EXAO_DEFINE_DESCRIPTION"             // 269
        ,"EXAO_DRAW_ELLIPSE"                   // 270
        ,"EXAO_DEFINE_EMBED"                   // 271
        ,"EXAO_DEFINE_END_OF_DWF"              // 272
        ,"EXAO_SET_FONT"                       // 273
        ,"EXAO_DRAW_IMAGE"                     // 274
        ,"EXAO_DEFINE_KEYWORDS"                // 275
        ,"EXAO_SET_LAYER"                      // 276
        ,"EXAO_SET_LINE_PATTERN"               // 277
        ,"EXAO_SET_LINE_WEIGHT"                // 278
        ,"EXAO_SET_LINE_STYLE"                 // 279
        ,"EXAO_DEFINE_MODIFICATION_TIME"       // 280
        ,"EXAO_DEFINE_NAMED_VIEW"              // 281
        ,"EXAO_DEFINE_PLOT_INFO"               // 282
        ,"EXAO_SET_PROJECTION"                 // 283
        ,"EXAO_DEFINE_SOURCE_CREATION_TIME"    // 284
        ,"EXAO_DEFINE_SOURCE_MODIFICATION_TIME" //  285
        ,"EXAO_SOURCE_FILENAME"                // 286
        ,"EXAO_DRAW_TEXT"                      // 287
        ,"EXAO_SET_URL"                        // 288
        ,"EXAO_DEFINE_UNITS"                   // 289
        ,"EXAO_SET_VIEWPORT"                   // 290
        ,"EXAO_SET_VIEW"                       // 291
        ,"EXAO_UNKNOWN"                        // 292
        ,"EXBO_SET_COLOR_MAP"                  // 293
        ,"EXBO_DRAW_IMAGE_BITONAL_MAPPED"      // 294
        ,"EXBO_DRAW_IMAGE_GROUP3X_MAPPED"      // 295
        ,"EXBO_DRAW_IMAGE_INDEXED"             // 296
        ,"EXBO_DRAW_IMAGE_MAPPED"              // 297
        ,"EXBO_DRAW_IMAGE_RGB"                 // 298
        ,"EXBO_DRAW_IMAGE_RGBA"                // 299
        ,"EXBO_DRAW_IMAGE_JPEG"                // 300
        ,"EXBO_ADSK_COMPRESSION"               // 301
        ,"EXBO_UNKNOWN"                        // 302
        ,"EXAO_DEFINE_TITLE"                   // 303
        ,"EXAO_DEFINE_SUBJECT"                 // 304
        ,"EXBO_DRAW_IMAGE_PNG"                 // 305
        ,"EXBO_DRAW_IMAGE_GROUP4"              // 306
        ,"EXAO_DRAW_PNG_GROUP4_IMAGE"          // 307
        ,"EXAO_SET_MERGE_CONTROL"              // 308
        ,""                                    // 309
        ,""                                    // 310
        ,""                                    // 311
        ,"EXAO_SET_OPTIMIZED_FOR_PLOTTING"     // 312
        ,"EXAO_SET_GROUP_BEGIN"                // 313
        ,"EXAO_SET_GROUP_END"                  // 314
        ,"EXAO_SET_FILL_PATTERN"               // 315
        ,"EXAO_SET_INKED_AREA"                 // 316
        ,"EXBO_DRAW_IMAGE_GROUP4X_MAPPED"      // 317
        ,"EXBO_EMBEDDED_FONT"                  // 318
        ,"EXAO_EMBEDDED_FONT"                  // 319
        ,"EXAO_TRUSTED_FONT_LIST"              // 320
        ,"EXAO_BLOCK_MEANING"                  // 321
        ,"EXBO_BLOCK_MEANING"                  // 322
        ,"EXBO_ENCRYPTION"                     // 323
        ,"EXAO_ENCRYPTION"                     // 324
        ,"WD_EXBO_ORIENTATION"                 // 325
        ,"EXAO_ORIENTATION"                    // 326
        ,"WD_EXBO_ALIGNMENT"                   // 327
        ,"EXAO_ALIGNMENT"                      // 328
        ,"EXAO_PASSWORD"                       // 329
        ,"WD_EXBO_GUID"                        // 330
        ,"WD_EXBO_PASSWORD"                    // 331
        ,"EXAO_GUID"                           // 332
        ,"WD_EXBO_FILETIME"                    // 333
        ,"EXAO_FILETIME"                       // 334
        ,"EXBO_GRAPHICS_HDR"                   // 335
        ,"EXBO_OVERLAY_HDR"                    // 336
        ,"EXBO_REDLINE_HDR"                    // 337
        ,"EXBO_THUMBNAIL"                      // 338
        ,"EXBO_PREVIEW"                        // 339
        ,"EXBO_OVERLAY_PREVIEW"                // 340
        ,"EXBO_FONT"                           // 341
        ,"EXBO_GRAPHICS"                       // 342
        ,"EXBO_OVERLAY"                        // 343
        ,"EXBO_REDLINE"                        // 344
        ,"EXBO_USER"                           // 345
        ,"EXBO_NULL"                           // 346
        ,"EXBO_GLOBAL_SHEET"                   // 347
        ,"EXBO_GLOBAL"                         // 348
        ,"EXBO_SIGNATURE"                      // 349
        ,"EXBO_BLOCKREF"                       // 350
        ,"EXAO_BLOCKREF"                       // 351
        ,"EXBO_DIRECTORY"                      // 352
        ,"EXAO_DIRECTORY"                      // 353
        ,"EXBO_USERDATA"                       // 354
        ,"EXAO_USERDATA"                       // 355
        ,"EXBO_PEN_PATTERN"                    // 356
        ,"EXAO_PEN_PATTERN"                    // 357
        ,"EXBO_SIGNDATA"                       // 358
        ,"EXAO_SIGNDATA"                       // 359
        ,"EXBO_GUID_LIST"                      // 360
        ,"EXAO_GUID_LIST"                      // 361
        ,"EXAO_SET_FONT_EXTENSION"             // 362
        ,"EXAO_PENPAT_OPTIONS"                 // 363
        ,"EXAO_GOURAUD_POLYTRIANGLE"           // 364
        ,"EXAO_DRAWING_INFO"                   // 365
        ,"EXAO_OBJECT_NODE"                    // 366
        ,"EXAO_GOURAUD_POLYLINE"               // 367
        ,"EXAO_BEZIER"                         // 368
        ,"EXBO_MACRO_DEFINITION"               // 369
        ,"EXAO_MACRO_DEFINITION"               // 370
        ,"EXBO_TEXT_HALIGN"                    // 371
        ,"EXAO_TEXT_HALIGN"                    // 372
        ,"EXBO_TEXT_VALIGN"                    // 373
        ,"EXAO_TEXT_VALIGN"                    // 374
        ,"EXBO_TEXT_BACKGROUND"                // 375
        ,"EXAO_TEXT_BACKGROUND"                // 376
        ,"EXBO_OVERPOST"                       // 377        
        ,"EXAO_OVERPOST"                       // 378 
        ,"EXA0_DELINEATE"                      // 379
        ,"EXBO_DELINEATE"                      // 380
        ,"EXAO_SET_USER_FILL_PATTERN"          // 381
        ,"EXBO_SET_USER_FILL_PATTERN"          // 382
        ,"EXAO_SET_USER_HATCH_PATTERN"         // 383
        ,"EXBO_SET_USER_HATCH_PATTERN"         // 384
        ,"EXAO_SET_CONTRAST_COLOR"             // 385
        ,"EXBO_SET_CONTRAST_COLOR"             // 386
        ,"EXAO_SET_ATTRIBUTE_URL"              // 387
    };

///////////////////////////////////////////////////////////////////////////
WT_File_Stats::WT_File_Stats(WT_File const * file)
        : m_last_opcode_start_position(0)
        , m_last_opcode_index(0)
        , m_main_file(file)
{
        for (int loop = 0; loop <= WD_MAX_OPCODE_ID; loop++)
        {
            m_opcode_count[loop]        =
            m_opcode_total_bytes[loop]  = 0;
        }
}

///////////////////////////////////////////////////////////////////////////
char const * WT_File_Stats::descriptions(int index) const
{
    return g_DWF_Opcode_Descriptions[index<0?m_last_opcode_index:index];
}
