//  Copyright (c) 2001-2006 by Autodesk, Inc.
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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/penpat.cpp 1     9/12/04 8:55p Evansg $


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif

///////////////////////////////////////////////////////////////////////////
WT_Pen_Pattern::WT_Pen_Pattern(
    WT_Pattern_ID                pattern_id,
    WT_Unsigned_Integer32        screening_percentage,
    WT_Boolean                   transparency_colormap_flag,
    WT_Color_Map const *         color_map,
    WT_Boolean                   copy) throw(WT_Result)
    : m_id(pattern_id)
    , m_screening_percentage(screening_percentage)
    , m_color_map(WD_Null)
    , m_local_color_map_copy(copy)
    , m_stage(Starting)
{
    if(transparency_colormap_flag) {
        if (color_map && copy)
        {
            m_color_map = new WT_Color_Map(*color_map);
            if (!m_color_map)
                throw WT_Result::Out_Of_Memory_Error;
        }
        else
        {
            m_color_map = (WT_Color_Map *) color_map;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Pen_Pattern& WT_Pen_Pattern::operator= ( const WT_Pen_Pattern& pen_pattern ) throw(WT_Result)
{
    m_id = pen_pattern.m_id;
    set_screening_percentage(pen_pattern.screening_percentage());

    bool transparency_colormap_flag = pen_pattern.color_map()!=NULL;
    if(transparency_colormap_flag )
    {
        if(pen_pattern.is_local_color_map_copy())
        {
            m_local_color_map_copy = WD_True;
            m_color_map = new WT_Color_Map(*pen_pattern.color_map());
            if (!m_color_map)
                throw WT_Result::Out_Of_Memory_Error;
        }
        else
        {
            m_local_color_map_copy = WD_False;
            m_color_map = (WT_Color_Map *)pen_pattern.color_map();
        }
    }
	else
	{
		m_local_color_map_copy = WD_False;
		m_color_map = WD_Null;
	}
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Pen_Pattern::WT_Pen_Pattern(WT_Pen_Pattern const & pattern) throw(WT_Result)
    : WT_Attribute()
{
    *this = pattern;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Pen_Pattern::object_id() const
{
    return Pen_Pattern_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Pen_Pattern::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Pen_Pattern::process(WT_File & file)
{
    WD_Assert(file.pen_pattern_action());
    return (file.pen_pattern_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Pen_Pattern::default_process(WT_Pen_Pattern & item, WT_File & file)
{
    file.rendition().pen_pattern() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Pen_Pattern::serialize(WT_File & file) const
{

    WD_CHECK (file.dump_delayed_drawable());

    int colormap_size = 0;

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    switch (m_id)
    {
    case Screening_Black:
    case Screening_Alternate:
    case Screening_Block:
    case Screening_Dots:
    case Screening_Big_Dots:
        if (file.heuristics().allow_binary_data())
        {
            if(m_color_map != NULL) {
                WD_Assert(m_color_map->size() == 2);
                colormap_size = 1 +         // For the colormap size byte
                m_color_map->size() * 4;    // The colormap itself
            }
            WD_CHECK (file.write((WT_Byte) '{'));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                                     sizeof(WT_Integer32) +          // for pattern id
                                                     sizeof(WT_Unsigned_Integer32) + // for screening percentage
                                                     sizeof(WT_Byte) +               // for transparency colormap flag
                                                     colormap_size        +          // Space taken by the image's colormap, 0 in most cases
                                                     sizeof(WT_Byte)                 // The closing "}"
                                                    )));
            }
            else {
                WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                                 sizeof(WT_Integer32) +              // for pattern id
                                                 sizeof(WT_Unsigned_Integer32) +     // for screening percentage
                                                 sizeof(WT_Byte) +                   // for transparency colormap flag
                                                 sizeof(WT_Byte)                     // The closing "}"
                                                )));
            }

            WD_CHECK (file.write((WT_Unsigned_Integer16) WD_EXBO_PEN_PATTERN ));
            WD_CHECK (file.write((WT_Integer32) m_id));
            WD_CHECK (file.write((WT_Unsigned_Integer32) screening_percentage()));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) '1'));
            }
            else {
                WD_CHECK (file.write((WT_Byte) '0'));
            }

            if (colormap_size && m_color_map != NULL)
                WD_CHECK(m_color_map->serialize_just_colors(file));

            WD_CHECK (file.write((WT_Byte) '}'));
        }
        else {
            WD_CHECK (file.write_tab_level());
            WD_CHECK (file.write("(PenPattern "));

            WD_CHECK (file.write_ascii((WT_Integer32) pen_pattern()));
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii((WT_Unsigned_Integer32) screening_percentage()));
            WD_CHECK (file.write((WT_Byte) ' '));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) '1'));
            }
            else {
                WD_CHECK (file.write((WT_Byte) '0'));
            }

            if (m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) ' '));
                WD_CHECK(m_color_map->serialize(file));
            }

            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write(")"));
        }
        break;

    case Dots_Big:
    case Dots_Medium:
    case Dots_Small:
    case Slant_Left_32x32:
    case Slant_Right_32x32:
    case Screen_15:
    case Screen_25:
    case Screen_20:
    case Screen_75:
    case Screen_50:
    case Screen_Thin_50:
    case Screen_Hatched_50:
    case Trellis:
    case ZigZag:
    case Diagonal:
    case Triangle:
    case Triangle_More:
    case Bricks:
    case Bricks_Big:
    case Squares:
    case Squares_3D:
    case Diamond_Plaid:
    case Ziggurat:
    case Diagonal_Thatch:
    case Zipper:
    case Slants:
    case Slants_More:
    case Diags:
    case Diags_More:
    case Marks:
    case Marks_More:
    case Diamonds_Thick:
    case Diamonds_Thin:

    case Screening_Black_0:
    case Screening_Black_10:
    case Screening_Black_15:
    case Screening_Black_20:
    case Screening_Black_35:
    case Screening_Black_40:
    case Screening_Black_45:
    case Screening_Black_50:
    case Screening_Black_55:
    case Screening_Black_60:
    case Screening_Black_65:
    case Screening_Black_70:
    case Screening_Black_75:
    case Screening_Black_80:
    case Screening_Black_85:
    case Screening_Black_90:
    case Screening_Black_100:

    case Screening_Alternate_5:
    case Screening_Alternate_10:
    case Screening_Alternate_15:
    case Screening_Alternate_20:
    case Screening_Alternate_25:
    case Screening_Alternate_35:
    case Screening_Alternate_40:
    case Screening_Alternate_45:
    case Screening_Alternate_50:
    case Screening_Alternate_65:
    case Screening_Alternate_70:
    case Screening_Alternate_80:
    case Screening_Alternate_85:
    case Screening_Alternate_90:
    case Screening_Alternate_95:

    case Screening_Block_10:
    case Screening_Block_20:
    case Screening_Block_25:
    case Screening_Block_30:
    case Screening_Block_40:
    case Screening_Block_50:
    case Screening_Block_60:
    case Screening_Block_65:
    case Screening_Block_70:
    case Screening_Block_75:
    case Screening_Block_80:
    case Screening_Block_85:
    case Screening_Block_90:
    case Screening_Block_95:

    case Screening_Dots_5:
    case Screening_Dots_10:
    case Screening_Dots_15:
    case Screening_Dots_20:
    case Screening_Dots_25:
    case Screening_Dots_30:
    case Screening_Dots_40:
    case Screening_Dots_50:
    case Screening_Dots_60:
    case Screening_Dots_70:
    case Screening_Dots_75:
    case Screening_Dots_85:
    case Screening_Dots_90:
    case Screening_Dots_95:

    case Screening_Big_Dots_5:
    case Screening_Big_Dots_10:
    case Screening_Big_Dots_20:
    case Screening_Big_Dots_30:
    case Screening_Big_Dots_40:
    case Screening_Big_Dots_50:
    case Screening_Big_Dots_60:
    case Screening_Big_Dots_65:
    case Screening_Big_Dots_70:
    case Screening_Big_Dots_75:
    case Screening_Big_Dots_80:
    case Screening_Big_Dots_85:
    case Screening_Big_Dots_90:
    case Screening_Big_Dots_95:
        if (file.heuristics().allow_binary_data())
        {
            if(m_color_map != NULL) {
                WD_Assert(m_color_map->size() == 2);
                colormap_size = 1 +                         // For the colormap size byte
                                    m_color_map->size() * 4;    // The colormap itself
            }
            WD_CHECK (file.write((WT_Byte) '{'));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                                     sizeof(WT_Integer32) +          // for pattern id
                                                     sizeof(WT_Byte) +               // for transparency colormap flag
                                                     colormap_size        +          // Space taken by the image's colormap, 0 in most cases
                                                     sizeof(WT_Byte)                 // The closing "}"
                                                    )));
            }
            else {
                WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                                 sizeof(WT_Integer32) +          // for pattern id
                                                 sizeof(WT_Byte) +               // for transparency colormap flag
                                                 sizeof(WT_Byte)                 // The closing "}"
                                                )));
            }

            WD_CHECK (file.write((WT_Unsigned_Integer16) WD_EXBO_PEN_PATTERN ));
            WD_CHECK (file.write((WT_Integer32) m_id));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) '1'));
            }
            else {
                WD_CHECK (file.write((WT_Byte) '0'));
            }

            if (colormap_size && m_color_map != NULL) {
                WD_CHECK(m_color_map->serialize_just_colors(file));
            }

            WD_CHECK (file.write((WT_Byte) '}'));
        }
        else {
            WD_CHECK (file.write_tab_level());
            WD_CHECK (file.write("(PenPattern "));

            WD_CHECK (file.write_ascii((WT_Integer32) pen_pattern()));
            WD_CHECK (file.write((WT_Byte) ' '));
            if(m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) '1'));
            }
            else {
                WD_CHECK (file.write((WT_Byte) '0'));
            }
            WD_CHECK (file.write((WT_Byte) ' '));

            if (m_color_map != NULL) {
                WD_CHECK (file.write((WT_Byte) ' '));
                WD_CHECK(m_color_map->serialize(file));
            }

            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write(")"));
        }
        break;

    default:
        return WT_Result::Internal_Error;
    } // switch


    return WT_Result::Success;
}
#else
WT_Result WT_Pen_Pattern::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Pen_Pattern::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Integer32 npattern_id = 0;
    WT_Byte a_byte;
    WT_Integer32 a_integer = 0;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Pattern_Id;
            // No break;

        case Getting_Pattern_Id:
            WD_CHECK (file.read(npattern_id));
            WD_Assert((npattern_id > 0) && (npattern_id < Count));
            if((npattern_id <= 0) || (npattern_id >= Count))
                return WT_Result::Internal_Error;

            m_id = (WT_Pattern_ID) npattern_id;
            if((m_id >= 1) && (m_id <6))
            {
                m_stage = Getting_Screening_Percentage;
            }
            else
            {
                m_stage = Getting_Transparency_ColorMap_Flag;
                goto Binary_Getting_Transparency_ColorMap_Flag_Hop;
            }

            // No break
        case Getting_Screening_Percentage:
            WD_CHECK (file.read(m_screening_percentage));
            m_stage = Getting_Transparency_ColorMap_Flag;

            // No break

Binary_Getting_Transparency_ColorMap_Flag_Hop:
        case Getting_Transparency_ColorMap_Flag:
            WD_CHECK (file.read(1, &a_byte));
            if(a_byte == '1')
                m_stage = Getting_Color_Map_Opcode;
            else if (a_byte == '0') {
                m_stage = Getting_Close;
                goto Binary_Getting_Close_Hop;
            }
            else
                return WT_Result::Internal_Error;

            // No break
        case Getting_Color_Map_Opcode:

            m_color_map = new WT_Color_Map();   // Normally we would have to specify
                                                    // a version number with the colormap
                                                    // constructor since the default map used depends on the file version we are dealing with.
                                                    // However, since we are going to "overwrite" the default map, it doesn't matter which one we get.
            if (!m_color_map)
                return WT_Result::Out_Of_Memory_Error;
            m_local_color_map_copy = WD_True;
            m_color_map->clear();
            m_stage = Getting_Color_Map;

            // No break
        case Getting_Color_Map:
            WD_CHECK (m_color_map->materialize_just_colors(opcode, WD_True, WD_False, file));
            m_stage = Getting_Close;

            // No break
Binary_Getting_Close_Hop:
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

                // Yeah!! We succeeded!
                break;  // Go below to clean up...
            default:
                return WT_Result::Internal_Error;  // Illegal stage.
        } // switch (m_stage) for Binary

        break;

    case WT_Opcode::Extended_ASCII:

        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Pattern_Id;
            // No break;

        case Getting_Pattern_Id:
            WD_CHECK (file.read_ascii(npattern_id));
            WD_Assert((npattern_id > 0) && (npattern_id < Count));
            if((npattern_id <= 0) || (npattern_id >= Count))
                return WT_Result::Internal_Error;

            m_id = (WT_Pattern_ID) npattern_id;
            if((m_id >= 1) && (m_id <6))
            {
                m_stage = Getting_Screening_Percentage;
            }
            else
            {
                m_stage = Getting_Transparency_ColorMap_Flag;
                goto ASCII_Getting_Transparency_ColorMap_Flag_Hop;
            }

            // No break
        case Getting_Screening_Percentage:
            WD_CHECK (file.read_ascii(m_screening_percentage));
            m_stage = Getting_Transparency_ColorMap_Flag;

            // No break

ASCII_Getting_Transparency_ColorMap_Flag_Hop:
        case Getting_Transparency_ColorMap_Flag:
            WD_CHECK (file.read_ascii(a_integer));
            if(a_integer == 1)
                m_stage = Getting_Color_Map_Opcode;
            else if (a_integer == 0) {
                m_stage = Getting_Close;
                goto ASCII_Getting_Close_Hop;
            }
            else
                return WT_Result::Internal_Error;

            m_stage = Getting_Color_Map_Opcode;

            // No break
        case Getting_Color_Map_Opcode:

            WD_CHECK (m_colormap_opcode.get_opcode(file));

            m_color_map = new WT_Color_Map();   // Normally we would have to specify
                                                // a version number with the colormap
                                                // constructor since the default map used depends on the file version we are dealing with.
                                                // However, since we are going to "overwrite" the default map, it doesn't matter which one we get.
            if (!m_color_map)
                return WT_Result::Out_Of_Memory_Error;
            m_local_color_map_copy = WD_True;
            m_color_map->clear();
            m_stage = Getting_Color_Map;

            // No break
        case Getting_Color_Map:
            WD_CHECK (m_color_map->materialize(m_colormap_opcode, file));
            m_stage = Getting_Close;

            // No break
ASCII_Getting_Close_Hop:
        case Getting_Close:

            // There will be two close parens: one for the data, one for the opcode
            WD_CHECK (opcode.skip_past_matching_paren(file));

            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.

        } // switch (m_stage) for ASCII

        break;

    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;  // Illegal Opcode Type
    } // switch (opcode type)

    m_stage = Starting;  // We finished, so reset stage to start.

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Pen_Pattern::operator== ( WT_Attribute const & atref ) const
{   if( atref.object_id() != Pen_Pattern_ID )
        return WD_False;

    WT_Pen_Pattern const & r =
        (WT_Pen_Pattern const &) atref;

    if(r.m_id != m_id)
        return WD_False;

    if(screening_percentage() != r.screening_percentage())
        return WD_False;

    if((m_color_map==NULL) !=
        (r.color_map()==NULL))
        return WD_False;

    if(m_color_map!=NULL) {
        if(*m_color_map != *(r.color_map()))
            return WD_False;
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Pen_Pattern::operator!= ( WT_Pen_Pattern const & r ) const
{
    return !((*this) == r);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Pen_Pattern::sync( WT_File& file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().pen_pattern() )
    {
        WD_CHECK (serialize(file));
        file.rendition().pen_pattern() = *this;
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Pen_Pattern::sync( WT_File& ) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()
