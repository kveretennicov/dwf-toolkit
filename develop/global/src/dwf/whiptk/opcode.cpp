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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/opcode.cpp 20    7/25/05 7:39p Gremmert $


#include "whiptk/pch.h"

#define TRACK_OPCODES_STATISTICS(opcode_index)    file.file_stats()->add_stat(opcode_index);

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Opcode::get_opcode(WT_File & file, WT_Boolean allow_sub_options)
{
    if (!file.have_read_first_opcode())
    {
        WD_CHECK (file.read(6, m_token));

        m_size = 6;
        m_token[m_size] = '\0';
        if (strcmp("(DWF V", (char *)m_token) &&
            strcmp("(W2D V", (char *)m_token))
            return WT_Result::Not_A_DWF_File_Error;

        if (!strcmp("(W2D V", (char *)m_token))
            file.heuristics().m_w2d_channel = WD_True;
        m_type = Extended_ASCII;
        file.increment_paren_count();
        m_status = Finished;

        file.set_have_read_first_opcode(WD_True);
        return WT_Result::Success;
    }

    // This is another dangerous routine since we may have already been
    // working on assembling an opcode token when the input stream ran out
    // of data.  Thus we need to see if we are just picking up from where
    // we left off, or if we are starting anew.

    // If result says we are finished, then we must have been called to read
    // in the *next* opcode token, in which case we'll want to start the whole
    // process at the beginning step which is to eat whitespace.
    switch (m_status)
    {
    case Finished:
    case Starting:

        m_status = Eating_Whitespace;
        m_prior_paren_level = file.paren_count();

        // No break here

    case Eating_Whitespace:

        WD_CHECK (file.eat_whitespace());
        m_status = Determining_Opcode_Type;

        // No break here

    case Determining_Opcode_Type:

        WD_CHECK (file.read(m_token[0]));
        m_size = 1;

        switch (m_token[0])
        {
        case '(':
            m_type = Extended_ASCII;
            file.increment_paren_count();
            m_status = Accumulating_Extended_Opcode;

            break;
        case '{':
            m_type   = Extended_Binary;
            m_status = Accumulating_Extended_Opcode;

            break;
        case ')':
            if (allow_sub_options)
            {
                // There are no more options left for this opcode
                m_type = Null_Optioncode;
                file.put_back(m_token[0]);
                m_token[0] = '\0';
                m_size = 0;
                m_status = Finished; 
                return WT_Result::Success;
            }
            else
                return WT_Result::Corrupt_File_Error;   // A ')' is not a legal opcode

            break;
        default:
            if (allow_sub_options)
            {
                m_type = Unary_Optioncode;
                m_status = Accumulating_Extended_Opcode;
            }
            else
            {
              m_type   = Single_Byte;
                m_token[1] = '\0';
                m_status = Finished;
                return WT_Result::Success;
            }
        }   // switch (m_token[0])

        // No break here
    case Accumulating_Extended_Opcode:

        if (m_type == Extended_Binary)
        {
            // Extended binary
            // TODO: deal with Endian issues
            WD_CHECK (file.read(WD_EXTENDED_BINARY_OPCODE_SIZE + WD_EXTENDED_BINARY_OFFSET_SIZE,
                                &m_token[1]));

            m_status = Finished;
        }
        else
        {
            // m_type == Extended_ASCII || m_type == Unary_Optioncode

            WT_Byte    a_byte;

            while (1)
            {
                WD_CHECK (file.read(a_byte));

                if (is_legal_opcode_character(a_byte))
                {
                    m_token[m_size++] = a_byte;
                    if (m_size > WD_MAX_OPCODE_TOKEN_SIZE)
                    {
                        // Token is too long, so we have a corrupt file.
                        return WT_Result::Corrupt_File_Error;
                    }
                }
                else
                {
                    if (is_opcode_terminator(a_byte))
                    {
                        m_status = Finished;
                        m_token[m_size] = '\0';
                        file.put_back(a_byte);
                        return WT_Result::Success;
                    }
                    else
                    {
                        // Error, this file seems scrambled
                        return WT_Result::Corrupt_File_Error;
                    }
                }
            }
        }

    } // switch

    return WT_Result::Success;  // Cool!!
}

///////////////////////////////////////////////////////////////////////////
WT_Object * WT_Opcode::object_from_opcode(WT_Rendition & rend, WT_Result & result, WT_File & file) const
{
    WT_Object * tmp = WD_Null;

    result = WT_Result::Success;

    switch (type())
    {
    case Single_Byte:
        {
            TRACK_OPCODES_STATISTICS(m_token[0])

            switch (m_token[0])
            {
            case WD_SBBO_SET_COLOR_RGBA:    // 0x03 Ctrl-C
            case WD_SBAO_SET_COLOR_INDEXED: // 'C'
            case WD_SBBO_SET_COLOR_INDEXED: // 'c'
                tmp = new WT_Color;
                break;

            //TODO: Support bezier
            case WD_SBBO_BEZIER_16R:          //0x02 Ctrl-B
            case WD_SBAO_BEZIER_32R:          //0x42 'B'
            case WD_SBBO_BEZIER_32:           //0x62 'b'
                tmp = new WT_Unknown;
                break;

            case WD_SBBO_SET_FONT:   // 0x06 Ctrl-F
                tmp = new WT_Font;
                break;
            case WD_SBAO_SET_FILL_ON:   // 'F'
            case WD_SBAO_SET_FILL_OFF:  // 'f'
                tmp = new WT_Fill;
                break;
            case WD_SBBO_DRAW_GOURAUD_POLYTRIANGLE_16R: // 0x07 Ctrl-G
            case WD_SBBO_DRAW_GOURAUD_POLYTRIANGLE_32R: // 'g'
                tmp = new WT_Gouraud_Polytriangle;
                break;
            // IMP. The following opcodes represent WT_PolyMarker_Symbol for file versions less then 601 
            // and WT_Macro_Index for file versions 601 and greater
            case WD_SBAO_SET_MACRO_INDEX: // 'G'
                if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_MACRO_IS_SUPPORTED)                
                    tmp = new WT_Marker_Symbol;
                else
                    tmp = new WT_Macro_Index;
                break;
            case WD_SBBO_DRAW_CONTOUR_SET_16R:  // 0x0B Ctrl-K
            case WD_SBBO_DRAW_CONTOUR_SET_32R:  // 'k'
                tmp = new WT_Contour_Set;
                break;
            case WD_SBBO_DRAW_LINE_16R: // 0x0C Ctrl-L
            case WD_SBAO_DRAW_LINE:     // 'L'
            case WD_SBBO_DRAW_LINE_32R: // 'l'
                tmp = new WT_Polyline;
                break;
            case WD_SBBO_SET_LAYER: // 0xAC High-order-bit + Ctrl-L
                tmp = new WT_Layer;
                break;
            case WD_SBBO_SET_LINE_PATTERN:  // 0xCC
                tmp = new WT_Line_Pattern;
                break;

            // IMP. The following opcodes represent WT_PolyMarker for file versions less then 601 
            // and WT_Macro_Draw for file versions 601 and greater
            case WD_SBBO_DRAW_MACRO_DRAW_16R:   // 0x8D This SHOULD have been 0x0D, but since 0x0D is a "Whitespace" character (CR) we are stuck now with 0x8D
            case WD_SBAO_DRAW_MACRO_DRAW:       // 'M'
            case WD_SBBO_DRAW_MACRO_DRAW_32R:   // 'm'
                if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_MACRO_IS_SUPPORTED)                
                    tmp = new WT_Polymarker;
                else
                    tmp = new WT_Macro_Draw; 
                break;
            case WD_SBBO_SET_ORIGIN_32: // 'O'
                tmp = new WT_Origin;
                break;
            case WD_SBBO_DRAW_POLYLINE_POLYGON_16R: // 0x10 Ctrl-P
            case WD_SBAO_DRAW_POLYLINE_POLYGON:     // 'P'
            case WD_SBBO_DRAW_POLYLINE_POLYGON_32R: // 'p'
                if (rend.fill().fill())
                    tmp = new WT_Polygon;
                else
                    tmp = new WT_Polyline;
                break;
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_16R: // 0x11 Ctrl-Q
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_32R: // 'q'
                tmp = new WT_Gouraud_Polyline;
                break;
            case WD_SBAO_DRAW_ELLIPSE:          // 'E'
            case WD_SBBO_DRAW_ELLIPSE_32R:      // 'e'
            case WD_SBBO_DRAW_CIRCLE_16R:       // 0x12 Ctrl-R
            case WD_SBAO_DRAW_CIRCLE:           // 'R'
            case WD_SBBO_DRAW_CIRCLE_32R:       // 'r'
            case WD_SBBO_DRAW_CIRCULAR_ARC_32R: // 0x92 XTENDED-Ctrl-R
                if (rend.fill().fill())
                    tmp = new WT_Filled_Ellipse;
                else
                    tmp = new WT_Outline_Ellipse;
                break;
            // IMP. The following opcodes represent WT_PolyMarker_Size for file versions less then 601 
            // and WT_Macro_Scale for file versions 601 and greater
            case WD_SBAO_SET_MACRO_SCALE:   // 'S'
            case WD_SBBO_SET_MACRO_SCALE:   // 's'
                if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_MACRO_IS_SUPPORTED)                
                    tmp = new WT_Marker_Size;
                else
                    tmp = new WT_Macro_Scale;
                break;
            case WD_SBBO_DRAW_POLYTRIANGLE_16R: // 0x14 Ctrl-T
            case WD_SBAO_DRAW_POLYTRIANGLE:     // 'T'
            case WD_SBBO_DRAW_POLYTRIANGLE_32R: // 't'
                tmp = new WT_Polytriangle;
                break;
            case WD_SBAO_SET_VISIBILITY_ON:     // 'V'
            case WD_SBAO_SET_VISIBILITY_OFF:    // 'v'
                tmp = new WT_Visibility;
                break;
            case WD_SBBO_SET_LINE_WEIGHT:   // 0x17 Ctrl-W
                tmp = new WT_Line_Weight;
                break;
            case WD_SBBO_DRAW_TEXT_COMPLEX: // 0x18 Ctrl-X
            case WD_SBBO_DRAW_TEXT_BASIC:   // 'x'
                tmp = new WT_Text;
                break;
            case WD_SBBO_OBJECT_NODE_32:    // 'N'
            case WD_SBBO_OBJECT_NODE_16:    // 'n'
            case WD_SBBO_OBJECT_NODE_AUTO:  // 0x0E
                tmp = new WT_Object_Node;
                break;
            
            default:
                {
                    // Error, unrecognized opcode and thus can't skip.
                    // Corrupt file!
                    WD_Assert(WD_False);
                    result = WT_Result::Unsupported_DWF_Opcode;
                } break;
            } // switch on single byte

            // If so far so good (it's an opcode we know), but our 'tmp' is null,
            // then we couldn't create it, so we must be out of memory
            if (result == WT_Result::Success && tmp == WD_Null)
                result = WT_Result::Out_Of_Memory_Error;

        } break; // case single byte
    case Extended_ASCII:
        {
            // Performance Note:  if you add a new Extended ASCII opcode here
            // please make sure your "if" checks appear in the order of probability
            // that they will be found in the file.  For example, under the "D" case
            // we have '(DWF', '(Description', and '(DashPattern'.  Since 'dashpattern'
            // will occur more often than the other two, its strcmp test should be first
            // for speed.

            switch (m_token[1])
            {
            case 'A':
                if (!strcmp((char const *)m_token,"(Author"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_AUTHOR)
                    tmp = new WT_Author;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Alignment"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_ALIGNMENT)
                    tmp = new WT_Alignment();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } 
                else if (!strcmp((char const *)m_token,"(AttributeURL"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_ATTRIBUTE_URL)
                    tmp = new WT_Attribute_URL();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } 
                
                break;
            case 'B':
                if (!strcmp((char const *)m_token,"(Background"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_BACKGROUND)
                    tmp = new WT_Background;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(BlockMeaning"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_BLOCK_MEANING)
                    tmp = new WT_Block_Meaning;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(BlockRef"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_BLOCKREF)
                    tmp = new WT_BlockRef();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'C':
                if (!strcmp((char const *)m_token,"(Circle"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_CIRCLE)
                    if (rend.fill().fill())
                        tmp = new WT_Filled_Ellipse;
                    else
                        tmp = new WT_Outline_Ellipse;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Contour"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_CONTOUR)
                    tmp = new WT_Contour_Set;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Color"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_COLOR)
                    tmp = new WT_Color;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(ColorMap"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_COLOR_MAP)
                    tmp = new WT_Color_Map();  // No need to specify which version of the default colormap we want since we are going to erase it anyway.
                    if (tmp)
                        ((WT_Color_Map *)tmp)->clear();
                    else
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strncmp((char const *)m_token,"(Comment", 8))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_COMMENTS)
                    tmp = new WT_Comments;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(ContrastColor"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_CONTRAST_COLOR)
                    tmp = new WT_Contrast_Color;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Copyright"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_COPYRIGHT)
                    tmp = new WT_Copyright;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Creator"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_CREATOR)
                    tmp = new WT_Creator;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Created"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_CREATION_TIME)
                    tmp = new WT_Creation_Time;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(CodePage"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_CODE_PAGE)
                    tmp = new WT_Code_Page;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'D':
                if (!strcmp((char const *)m_token,"(DashPattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_DASH_PATTERN)
                    tmp = new WT_Dash_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(DWF V"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_DWF_HEADER)
                    tmp = new WT_DWF_Header;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Description"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_DESCRIPTION)
                    tmp = new WT_Description;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Directory"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DIRECTORY)
                    tmp = new WT_Directory;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Delineate"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DELINEATE)
                    tmp = new WT_Delineate;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'E':
                if (!strcmp((char const *)m_token,"(Ellipse"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_ELLIPSE)
                    if (rend.fill().fill())
                        tmp = new WT_Filled_Ellipse;
                    else
                        tmp = new WT_Outline_Ellipse;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                    break;
                }
                else if (!strcmp((char const *)m_token,"(Embed"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_EMBED)
                    tmp = new WT_Embed;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(EndOfDWF"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_END_OF_DWF)
                    tmp = new WT_End_Of_DWF;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Embedded_Font"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_EMBEDDED_FONT)
                    tmp = new WT_Embedded_Font;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Encryption"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_ENCRYPTION)
                    tmp = new WT_Encryption;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'F':
                if (!strcmp((char const *)m_token,"(Font"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_FONT)
                    tmp = new WT_Font;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(FillPattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_FILL_PATTERN)
                    tmp = new WT_Fill_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(FontExtension"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_FONT_EXTENSION)
                    tmp = new WT_Font_Extension;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'G':
                if (!strcmp((char const *)m_token,"(Gouraud"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_GOURAUD_POLYTRIANGLE)
                    tmp = new WT_Gouraud_Polytriangle;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(GourLine"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_GOURAUD_POLYLINE)
                    tmp = new WT_Gouraud_Polyline;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Group4PNGImage"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_PNG_GROUP4_IMAGE)
                    tmp = new WT_PNG_Group4_Image;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(GroupBegin"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_GROUP_BEGIN)
                    tmp = new WT_Group_Begin;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(GroupEnd"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_GROUP_END)
                    tmp = new WT_Group_End;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Guid"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_GUID)
                    tmp = new WT_Guid;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(GuidList"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_GUID_LIST)
                    tmp = new WT_Guid_List;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;

            case 'I':
                if (!strcmp((char const *)m_token,"(Image"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_IMAGE)
                    tmp = new WT_Image;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(InkedArea"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_INKED_AREA)
                    tmp = new WT_Inked_Area;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'K':
                if (!strcmp((char const *)m_token,"(Keywords"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_KEYWORDS)
                    tmp = new WT_Keywords;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'L':
                if (!strcmp((char const *)m_token,"(Layer"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_LAYER)
                    tmp = new WT_Layer;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(LinePattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_LINE_PATTERN)
                    tmp = new WT_Line_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(LinesOverwrite"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_MERGE_CONTROL)
                    tmp = new WT_Merge_Control;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(LineWeight"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_LINE_WEIGHT)
                    tmp = new WT_Line_Weight;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(LineStyle"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_LINE_STYLE)
                    tmp = new WT_Line_Style;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'M':
                if (!strcmp((char const *)m_token,"(Modified"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_MODIFICATION_TIME)
                    tmp = new WT_Modification_Time;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Macro"))
                {   
                    TRACK_OPCODES_STATISTICS(WD_EXAO_MACRO_DEFINITION)
                    tmp = new WT_Macro_Definition;
                    if(!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'N':
                if (!strcmp((char const *)m_token,"(NamedView"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_NAMED_VIEW)
                    tmp = new WT_Named_View;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Node"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_OBJECT_NODE)
                    tmp = new WT_Object_Node;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'O':
                if (!strcmp((char const *)m_token,"(Orientation"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_ORIENTATION)
                    tmp = new WT_Orientation;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if(!strcmp((char const *)m_token,"(Overpost"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_OVERPOST)
                    tmp = new WT_Overpost;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'P':
                if (!strcmp((char const *)m_token,"(PlotInfo"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_PLOT_INFO)
                    tmp = new WT_Plot_Info;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if(!strcmp((char const *)m_token,"(PlotOptimized"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_OPTIMIZED_FOR_PLOTTING)
                    tmp = new WT_Plot_Optimized;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if(!strcmp((char const *)m_token,"(Psswd"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_PASSWORD)
                    tmp = new WT_Password;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if(!strcmp((char const *)m_token,"(PenPattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_PEN_PATTERN)
                    tmp = new WT_Pen_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if(!strcmp((char const *)m_token,"(PenPat_Options"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_PEN_PATTERN)
                    tmp = new WT_PenPat_Options;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Projection"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_PROJECTION)
                    tmp = new WT_Projection;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;

            case 'S':
                if (!strcmp((char const *)m_token,"(SourceCreated"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_SOURCE_CREATION_TIME)
                    tmp = new WT_Source_Creation_Time;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(SourceModified"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_SOURCE_MODIFICATION_TIME)
                    tmp = new WT_Source_Modification_Time;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(SourceFilename"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SOURCE_FILENAME)
                    tmp = new WT_Source_Filename;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Subject"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_SUBJECT)
                    tmp = new WT_Subject;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(SignData"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SIGNDATA)
                    tmp = new WT_SignData;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'T':
                if (!strcmp((char const *)m_token,"(Text"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DRAW_TEXT)
                    tmp = new WT_Text;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(TextHAlign"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_TEXT_HALIGN)
                    tmp = new WT_Text_HAlign;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(TextVAlign"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_TEXT_VALIGN)
                    tmp = new WT_Text_VAlign;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(TextBackground"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_TEXT_BACKGROUND)
                    tmp = new WT_Text_Background;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Title"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_TITLE)
                    tmp = new WT_Title;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(TrustedFontList"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_TRUSTED_FONT_LIST)
                    tmp = new WT_Trusted_Font_List;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Time"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_FILETIME)
                    tmp = new WT_FileTime;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'U':
                if (!strcmp((char const *)m_token, "(URL"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_URL)
                    tmp = new WT_URL;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(Units"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_UNITS)
                    tmp = new WT_Units;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(UserData"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_USERDATA)
                    tmp = new WT_UserData;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(UserFillPattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_USER_FILL_PATTERN)
                    tmp = new WT_User_Fill_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(UserHatchPattern"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_USER_HATCH_PATTERN)
                    tmp = new WT_User_Hatch_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case 'V':
                if (!strcmp((char const *)m_token,"(Viewport"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_VIEWPORT)
                    tmp = new WT_Viewport;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                else if (!strcmp((char const *)m_token,"(View"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_SET_VIEW)
                    tmp = new WT_View;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }

                break;
            case 'W':
                if (!strcmp((char const *)m_token,"(W2D V"))
                {
                    TRACK_OPCODES_STATISTICS(WD_EXAO_DEFINE_DWF_HEADER)
                    tmp = new WT_DWF_Header;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            }  // Switch

            //it is an unknown opcode
            if (!tmp && result==WT_Result::Success)
            {
                TRACK_OPCODES_STATISTICS(WD_EXAO_UNKNOWN)
                tmp = new WT_Unknown;
            }

        } break;
    case Extended_Binary:
        {
            // For now, *all* extend Binary opcodes are "unknown"
            int    ext_bin_value;

            ext_bin_value = (m_token[6] << 8) + m_token[5];
            switch (ext_bin_value)
            {
            case WD_COLOR_MAP_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_SET_COLOR_MAP)
                    tmp = new WT_Color_Map();  // No need to specify which version of the default colormap we want since we are going to erase it anyway.
                    if (tmp)
                        ((WT_Color_Map *)tmp)->clear();
                    else
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_BITONAL_MAPPED_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_BITONAL_MAPPED)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_GROUP3X_MAPPED_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_GROUP3X_MAPPED)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_GROUP4X_MAPPED_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_GROUP4X_MAPPED)
                    tmp = new WT_PNG_Group4_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_INDEXED_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_INDEXED)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_MAPPED_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_MAPPED)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_RGB_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_RGB)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_RGBA_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_RGBA)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_JPEG_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_JPEG)
                    tmp = new WT_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_PNG_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_PNG)
                    tmp = new WT_PNG_Group4_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_IMAGE_GROUP4_BITONAL_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DRAW_IMAGE_GROUP4)
                    tmp = new WT_PNG_Group4_Image(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_EMBEDDED_FONT:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_EMBEDDED_FONT)
                    tmp = new WT_Embedded_Font;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_BLOCK_MEANING:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_BLOCK_MEANING)
                    tmp = new WT_Block_Meaning();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_ENCRYPTION:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_ENCRYPTION)
                    tmp = new WT_Encryption();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_ORIENTATION:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_ORIENTATION)
                    tmp = new WT_Orientation();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_ALIGNMENT:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_ALIGNMENT)
                    tmp = new WT_Alignment();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_FILETIME:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_FILETIME)
                    tmp = new WT_FileTime();
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_GRAPHICS_HDR:
            case WD_EXBO_OVERLAY_HDR:
            case WD_EXBO_REDLINE_HDR:
            case WD_EXBO_THUMBNAIL:
            case WD_EXBO_PREVIEW:
            case WD_EXBO_OVERLAY_PREVIEW:
            case WD_EXBO_FONT:
            case WD_EXBO_GRAPHICS:
            case WD_EXBO_OVERLAY:
            case WD_EXBO_REDLINE:
            case WD_EXBO_USER:
            case WD_EXBO_NULL:
            case WD_EXBO_GLOBAL_SHEET:
            case WD_EXBO_GLOBAL:
            case WD_EXBO_SIGNATURE:
            case WD_EXBO_BLOCKREF:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_BLOCKREF)
                    tmp = new WT_BlockRef((WT_BlockRef::WT_BlockRef_Format) ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_LZ_COMPRESSION_EXT_OPCODE:
            case WD_LZ_COMPRESSION_EXT_OPCODE_OBSOLETE:
            case WD_ZLIB_COMPRESSION_EXT_OPCODE:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_ADSK_COMPRESSION)
                    tmp = new WT_Compressed_Data_Moniker(ext_bin_value);
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            case WD_EXBO_PASSWORD:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_PASSWORD)
                    tmp = new WT_Password;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_GUID:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_GUID)
                    tmp = new WT_Guid;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_GUID_LIST:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_GUID_LIST)
                    tmp = new WT_Guid_List;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_DIRECTORY:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_DIRECTORY)
                    tmp = new WT_Directory;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_USERDATA:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_USERDATA)
                    tmp = new WT_UserData;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_SIGNDATA:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_SIGNDATA)
                    tmp = new WT_SignData;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_PEN_PATTERN:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_PEN_PATTERN)
                    tmp = new WT_Pen_Pattern;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_MACRO_DEFINITION:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_MACRO_DEFINITION)
                   tmp = new WT_Macro_Definition;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_TEXT_BACKGROUND:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_TEXT_BACKGROUND)
                   tmp = new WT_Text_Background;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_TEXT_VALIGN:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_TEXT_VALIGN)
                   tmp = new WT_Text_VAlign;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_TEXT_HALIGN:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_TEXT_HALIGN)
                   tmp = new WT_Text_HAlign;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_OVERPOST:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_OVERPOST)
                   tmp = new WT_Overpost;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_DELINEATE:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_DELINEATE)
                   tmp = new WT_Delineate;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_SET_USER_FILL_PATTERN:            
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_SET_USER_FILL_PATTERN)
                   tmp = new WT_User_Fill_Pattern;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error;
                }
                break;
            case WD_EXBO_SET_USER_HATCH_PATTERN:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_SET_USER_HATCH_PATTERN)
                   tmp = new WT_User_Hatch_Pattern;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error; 
                }
                break;
            case WD_EXBO_SET_CONTRAST_COLOR:
                {
                   TRACK_OPCODES_STATISTICS(WD_EXBO_SET_CONTRAST_COLOR)
                   tmp = new WT_Contrast_Color;
                   if(!tmp)
                       result = WT_Result::Out_Of_Memory_Error; 
                }
                break;
            default:
                {
                    TRACK_OPCODES_STATISTICS(WD_EXBO_UNKNOWN)
                    tmp = new WT_Unknown;
                    if (!tmp)
                        result = WT_Result::Out_Of_Memory_Error;
                } break;
            } // switch
        } break;
    default:
        {
            // Error, unknown opcode type.
            result = WT_Result::Internal_Error;
        } break;
    } // switch on opcode type

    // Note, this condition should never happen.  Above, we do all of the
    // out of memory checks, and thus 'result' will indicate so.  Otherwise,
    // tmp should have contents.  So if this test is true, we're toast
    if (!tmp && result == WT_Result::Success)
        result = WT_Result::Internal_Error;
    return tmp;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Opcode::is_legal_opcode_character(WT_Byte byte) const
{
    if (byte >= '!' && byte <= 'z' && byte != '(' && byte != ')')
        return WD_True;

    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Opcode::is_opcode_terminator(WT_Byte byte) const
{
    if (WU_is_whitespace(byte) || byte == '(' || byte == ')')
        return WD_True;

    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Opcode::skip_past_matching_paren(WT_File & file) const
{
    return file.skip_past_matching_paren(matching_paren_level());
}

void WT_Opcode::set_token(const char cToken)
{
	m_token[0] = cToken;
	m_size = 1;

}
