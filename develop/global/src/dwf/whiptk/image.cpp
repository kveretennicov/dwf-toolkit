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
// $Header: /Components/Internal/DWF Toolkit/v7.2/develop/global/src/dwf/whiptk/image.cpp 6     10/31/05 8:39a Hainese $


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif

///////////////////////////////////////////////////////////////////////////
WT_Image::WT_Image(
    WT_Unsigned_Integer16     rows,
    WT_Unsigned_Integer16       columns,
    WT_Image_Format             format,
    WT_Integer32                identifier,
    WT_Color_Map const *        color_map,
    WT_Integer32                data_size,
    WT_Byte *                   data,
    WT_Logical_Point const &    min_corner,
    WT_Logical_Point const &    max_corner,
    WT_Boolean                  copy,
    WT_Integer32                dpi) throw(WT_Result)
    : m_rows(rows)
    , m_columns(columns)
    , m_format((WT_Byte)format)
    , m_identifier(identifier)
    , m_dpi(dpi)
    , m_color_map(WD_Null)
    , m_data_size(data_size)
    , m_data(WD_Null)
    , m_min_corner(min_corner)
    , m_max_corner(max_corner)
    , m_transformed(WD_False)
    , m_relativized(WD_False)
    , m_local_data_copy(copy)
    , m_local_color_map_copy(copy)
    , m_stage(Getting_Columns)
{
    WD_Assert (rows && columns && data_size && data);

    if (copy)
    {
        if (color_map)
        {
            WD_Assert (m_format == Mapped || m_format == Group3X_Mapped || m_format == Bitonal_Mapped);  // Shouldn't be passing in a colormap if we don't have a mappable image type

            // Note that the "default" colormap changed at version REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED
            m_color_map = new WT_Color_Map(); // It doesn't matter what version of the map we start with since we're going to replace it.
            if (m_color_map)
                *m_color_map = *color_map;  // Overloaded = operator copies the entire map
            else
                throw WT_Result::Out_Of_Memory_Error;
        }

        if (m_local_data_copy)
        {
            m_data = new WT_Byte[data_size];

            if (m_data)
                memcpy(m_data, data, data_size);
            else
                throw WT_Result::Out_Of_Memory_Error;
        } // if (copy)
    }  // if (copy)
    else
    {
        m_color_map = (WT_Color_Map *) color_map;
        m_data = data;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Image::object_id() const
{
    return Image_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::set(WT_Color_Map const* pColorMap)
{
    if(!pColorMap)
        return WT_Result::Internal_Error;
    
    WD_Assert (m_format == Mapped || m_format == Group3X_Mapped || m_format == Bitonal_Mapped);  // Shouldn't be passing in a colormap 
                                                                                                 //if we don't have a mappable image type

    if((m_local_color_map_copy) && (m_color_map != NULL))
        delete m_color_map;

    // Note that the "default" colormap changed at version REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED
    m_color_map = new WT_Color_Map(); // It doesn't matter what version of the map we start with
                                        // since we're going to replace it.    
    if (!m_color_map)
        return WT_Result::Out_Of_Memory_Error;
    
    m_local_color_map_copy = WD_True;
    m_color_map->clear();
    *m_color_map = *pColorMap;  // Overloaded = operator copies the entire map       

    return WT_Result::Success;

}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Image::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    WD_Assert(m_rows     > 0);
    WD_Assert(m_columns  > 0);
    WD_Assert(m_data);
    // Warned the programmer, now it's time to stop the user.
    if ( m_rows <= 0 || m_columns <= 0 || m_data == NULL ) {
        return WT_Result::Toolkit_Usage_Error;
    }

    int colormap_size = 0;

    // Make sure we have a legal image
    WT_Integer32    parts_to_sync = //  WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::BlockRef_Bit        |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                    //  WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit;

    switch (m_format)
    {
    case Bitonal_Mapped:
        // Need to convert this bitonal image into a Group3X_Mapped image before
        // serializing since DWF doesn't support bitonal directly.
        WD_CHECK ( ((WT_Image *) this)->convert_bitonal_to_group_3X());

        // No break
    case Group3X_Mapped:
        WD_Assert(m_color_map); // There had better be a colormap attached
        if (!m_color_map)
            return WT_Result::File_Write_Error;

        WD_Assert(m_color_map->size() == 2);
        colormap_size = 1 +                         // For the colormap size byte
                        m_color_map->size() * 4;    // The colormap itself

        break;
    case Mapped:
        WD_Assert(m_color_map); // There had better be a colormap attached
        if (!m_color_map)
            return WT_Result::File_Write_Error;

        colormap_size = 1 +                         // For the colormap size byte
                        m_color_map->size() * 4;    // The colormap itself

        break;
    case Indexed:
        parts_to_sync |= WT_Rendition::Color_Map_Bit;
        // No break;
    case RGB:
    case RGBA:
    case JPEG:
        WD_Assert(!m_color_map); // There should not be any colormap attached
        break;
    default:
        return WT_Result::Internal_Error;
    } // switch

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
        ((WT_Image *)this)->transform(file.heuristics().transform());

    if (file.heuristics().allow_binary_data())
    {
        ((WT_Image *)this)->relativize(file);

        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer16) + // Num columns
                                             sizeof(WT_Unsigned_Integer16) + // Num rows
                                             sizeof(WT_Logical_Point) +      // Lower left logical point
                                             sizeof(WT_Logical_Point) +      // Upper right logical point
                                             sizeof(WT_Integer32) +          // Image identifier
                                             colormap_size        +          // Space taken by the image's colormap, 0 in most cases
                                             sizeof(WT_Integer32) +          // the data size
                                             m_data_size +                   // the image data
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16) format() ));
        WD_CHECK (file.write(columns()));
        WD_CHECK (file.write(rows()));
        WD_CHECK (file.write(1, &min_corner()));
        WD_CHECK (file.write(1, &max_corner()));
        WD_CHECK (file.write(identifier()));

        if (colormap_size)
            WD_CHECK(m_color_map->serialize_just_colors(file));

        WD_CHECK (file.write(m_data_size));

        // TODO: put in a switch here based on format so that color endianism is accounted for
        switch (m_format)
        {
        case RGBA:
            {
                int num_pixels = rows() * columns();

                for (int pixel = 0; pixel < num_pixels; pixel++)
                    WD_CHECK (file.write(((WT_RGBA32 *)m_data)[pixel]));

            } break;

        case Bitonal_Mapped:
        case Group3X_Mapped:
        case Indexed:
        case Mapped:
        case RGB:
        case JPEG:
            WD_CHECK (file.write(m_data_size, m_data));
            break;
        default:
            return WT_Result::Internal_Error;
        } // switch

        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else
    {
        // Extended ASCII output

        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Image "));

        switch (format())
        {
        case Bitonal_Mapped:
                                WD_CHECK (file.write_quoted_string("bitonal", WD_True));
                                break;
        case Group3X_Mapped:
                                WD_CHECK (file.write_quoted_string("group 3X", WD_True));
                                break;
        case Indexed:
                                WD_CHECK (file.write_quoted_string("indexed", WD_True));
                                break;
        case Mapped:
                                WD_CHECK (file.write_quoted_string("mapped", WD_True));
                                break;
        case RGB:
                                WD_CHECK (file.write_quoted_string("RGB", WD_True));
                                break;
        case RGBA:
                                WD_CHECK (file.write_quoted_string("RGBA", WD_True));
                                break;
        case JPEG:
                                WD_CHECK (file.write_quoted_string("JPEG", WD_True));
                                break;
        default:
            return WT_Result::Internal_Error;
        }

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(identifier()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(columns()));
        WD_CHECK (file.write((WT_Byte) ','));
        WD_CHECK (file.write_ascii(rows()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(1, &min_corner()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(1, &max_corner()));

        if (colormap_size)
        {
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK(m_color_map->serialize(file));
        }

        WD_CHECK (file.write(" ("));
        WD_CHECK (file.write_ascii(m_data_size));
        WD_CHECK (file.write((WT_Byte) ' '));

        // TODO: put in a switch here based on format so that color endianism is accounted for
        WD_CHECK (file.write_hex(m_data_size, m_data));

        WD_CHECK (file.write("))"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Image::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Columns;
            // No break;

        case Getting_Columns:
            if (m_format == Bitonal_Mapped)
                return WT_Result::Corrupt_File_Error;   // We don't directly support bitonal's in DWF, use group3x
            WD_CHECK (file.read(m_columns));
            m_stage = Getting_Rows;

            // No break
        case Getting_Rows:
            WD_CHECK (file.read(m_rows));
            m_stage = Getting_Min_Corner;

            // No break
        case Getting_Min_Corner:
            WD_CHECK (file.read(1, &m_min_corner));
            m_stage = Getting_Max_Corner;

            // No break
        case Getting_Max_Corner:
            WD_CHECK (file.read(1, &m_max_corner));
            m_stage = Getting_Identifier;

            m_relativized = WD_True;
            de_relativize(file);

            // No break
        case Getting_Identifier:
            WD_CHECK (file.read(m_identifier));
            if (m_format == Bitonal_Mapped  ||
                m_format == Group3X_Mapped  ||
                m_format == Mapped              )
            {
                m_color_map = new WT_Color_Map();   // Normally we would have to specify
                                                    // a version number with the colormap
                                                    // constructor since the default map used depends on the file version we are dealing with.
                                                    // However, since we are going to "overwrite" the default map, it doesn't matter which one we get.
                if (!m_color_map)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_color_map_copy = WD_True;
                m_color_map->clear();
                m_stage = Getting_Color_Map;
            }
            else
            {
                m_stage = Getting_Data_Size;
                goto Getting_Data_Size_Hop;
            }

        case Getting_Color_Map:
            WD_CHECK (m_color_map->materialize_just_colors(opcode, WD_True, WD_False, file));
            m_stage = Getting_Data_Size;

            // No break
Getting_Data_Size_Hop:
        case Getting_Data_Size:
            WD_CHECK (file.read(m_data_size));
            m_stage = Getting_Data;

            // No break

        case Getting_Data:

            // if there is data, read; else do not allocate space and attempt to read
            if (m_data_size>0)
            {
                switch (m_format)
                {
                case Bitonal_Mapped:
                case Group3X_Mapped:
                case Indexed:
                case Mapped:
                case RGB:
                case JPEG:
                    if (!m_data)
                    {
                        m_data = new WT_Byte[m_data_size];
                        if (!m_data)
                            return WT_Result::Out_Of_Memory_Error;
                        m_local_data_copy = WD_True;
                    }
                    WD_CHECK (file.read(m_data_size, m_data));

                    break;
                case RGBA:
                    {
                        if (!m_data)
                        {
                            m_data = new WT_Byte[m_data_size];
                            if (!m_data)
                                return WT_Result::Out_Of_Memory_Error;
                            m_local_data_copy = WD_True;
                        }
                        WD_CHECK (file.read(m_data_size, m_data));

                        // Need to remap the RGBA data into our native WT_RGBA32 format.
                        // We do this in-place with the one array.
                        long    num_pixels = rows() * columns();
                        WT_Byte *   raw_pos = (WT_Byte *) m_data;
                        WT_RGBA32 * cooked_pos = (WT_RGBA32 *) m_data;

                        for (long pixel = 0; pixel < num_pixels; pixel++)
                        {
                            WT_RGBA32 color (   raw_pos[0],  // R
                                                raw_pos[1],  // G
                                                raw_pos[2],  // B
                                                raw_pos[3]); // A
                            raw_pos += 4;
                            *cooked_pos++ = color;
                        }
                    }
                    break;
                default:
                    return WT_Result::Corrupt_File_Error;
                } // switch (m_format)
            }

            m_stage = Getting_Close;

            // No break
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
            m_stage = Getting_Format;
            // No break;

        case Getting_Format:

            char *  format_name;

            WD_CHECK (file.read(format_name, 40));

            if (!strcmp(format_name, "bitonal"))
                m_format = Bitonal_Mapped;
            else if (!strcmp(format_name, "group 3X"))
                m_format = Group3X_Mapped;
            else if (!strcmp(format_name, "indexed"))
                m_format = Indexed;
            else if (!strcmp(format_name, "mapped"))
                m_format = Mapped;
            else if (!strcmp(format_name, "RGB"))
                m_format = RGB;
            else if (!strcmp(format_name, "RGBA"))
                m_format = RGBA;
            else if (!strcmp(format_name, "JPEG"))
                m_format = JPEG;
            else
            {
                delete []format_name;
                return WT_Result::Unsupported_DWF_Extension_Error;
            }

            delete []format_name;

            m_stage = Getting_Identifier;

            // No break
        case Getting_Identifier:

            WD_CHECK (file.read_ascii(m_identifier));
            m_stage = Getting_Columns;

            // No break
        case Getting_Columns:

            if (m_format == Bitonal_Mapped)
                return WT_Result::Corrupt_File_Error;   // We don't yet directly support bitonal's in DWF, use group3x

            WD_CHECK (file.read_ascii(m_columns));
            m_stage = Getting_Col_Row_Comma;

            // No break
        case Getting_Col_Row_Comma:
            WT_Byte a_byte;

            WD_CHECK (file.read(a_byte));
            if (a_byte != ',')
                return WT_Result::Corrupt_File_Error;

            m_stage = Getting_Rows;

            // No break
        case Getting_Rows:

            WD_CHECK (file.read_ascii(m_rows));
            m_stage = Getting_Min_Corner;

            // No break
        case Getting_Min_Corner:
            WD_CHECK (file.read_ascii(m_min_corner));
            m_stage = Getting_Max_Corner;

            // No break
        case Getting_Max_Corner:
            WD_CHECK (file.read_ascii(m_max_corner));

            m_relativized = WD_False;

            // Only certain formats have colormaps
            if (m_format != Bitonal_Mapped &&
                m_format != Group3X_Mapped &&
                m_format != Mapped              )
            {
                m_stage = Getting_Pre_Data_Size_Whitespace;
                goto Getting_Pre_Data_Size_Whitespace_Hop;
            }

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
            m_stage = Getting_Pre_Data_Size_Whitespace;

            // No break
Getting_Pre_Data_Size_Whitespace_Hop:
        case Getting_Pre_Data_Size_Whitespace:
            WD_CHECK(file.eat_whitespace());
            m_stage = Getting_Pre_Data_Size_Open_Paren;

            // No break
        case Getting_Pre_Data_Size_Open_Paren:
            WT_Byte     a_paren;

            WD_CHECK(file.read(a_paren));

            if (a_paren != '(')
                return WT_Result::Corrupt_File_Error;

            file.increment_paren_count();
            m_stage = Getting_Data_Size;

            // No break
        case Getting_Data_Size:
            WD_CHECK (file.read_ascii(m_data_size));
            m_stage = Getting_Data;

            // No break
        case Getting_Data:

            // if there is data, read; else do not allocate space and attempt to read
            if (m_data_size>0)
            {
                if (!m_data)
                {
                    m_data = new WT_Byte[m_data_size];
                    if (!m_data)
                        return WT_Result::Out_Of_Memory_Error;
                    m_local_data_copy = WD_True;
                }
                WD_CHECK (file.read_hex(m_data_size, m_data));
            }

            m_stage = Getting_Close;

            // No break

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

    m_transformed = WD_False;
    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    if (m_format == Bitonal_Mapped)
        return WT_Result::Unsupported_DWF_Opcode;

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Image::update_bounds(WT_File *)
{
    WT_Drawable::update_bounds(m_min_corner);
    WT_Drawable::update_bounds(m_max_corner);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::skip_operand(WT_Opcode const &, WT_File &)
{
    //TODO: implement
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void WT_Image::transform(WT_Transform const & transform)
{
    if (!m_transformed)
    {
        WT_Logical_Point    orig_xformed_min_cnr;
        WT_Logical_Point    orig_xformed_max_cnr;

        orig_xformed_min_cnr = m_min_corner * transform;
        orig_xformed_max_cnr = m_max_corner * transform;

        // Note: the parent application is responsible for rotating the image
        // when Landscape flipping is on.

        switch (transform.rotation())
        {
            case 0:
                m_min_corner = orig_xformed_min_cnr;
                m_max_corner = orig_xformed_max_cnr;
                break;
            case 90:
                m_min_corner = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                m_max_corner = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                break;
            case 180:
                m_min_corner = orig_xformed_max_cnr;
                m_max_corner = orig_xformed_min_cnr;
                break;
            case 270:
                m_min_corner = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                m_max_corner = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                break;
            default:
                throw WT_Result::Internal_Error;
        }

        m_transformed = WD_True;

    } // If (!transformed)
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_Image::relativize(WT_File & file)
{
    if (!m_relativized)
    {
        m_min_corner = file.update_current_point(m_min_corner);
        m_max_corner = file.update_current_point(m_max_corner);
        m_relativized = WD_True;
    } // If (!relativized)
}
#else
void WT_Image::relativize(WT_File &)
{
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_Image::de_relativize(WT_File & file)
{
    if (m_relativized)
    {
        m_min_corner = file.de_update_current_point(m_min_corner);
        m_max_corner = file.de_update_current_point(m_max_corner);
        m_relativized = WD_False;
    } // If (!relativized)
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::process(WT_File & file)
{
    WD_Assert (file.image_action());
    return (file.image_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::default_process(WT_Image &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
// ==========================================
// Group 3X Data:
static WT_Huffman::Code    Group3   = {WT_Huffman::Group3,   2};
                        // Group3X  = {WT_Huffman::Group3X,  2},
                        // Literal  = {WT_Huffman::Literal,  2},
                        // Reserved = {WT_Huffman::Reserved, 2};


#if DESIRED_CODE(WHIP_OUTPUT)

static WT_Huffman huffman[] = {
      //  run      white   wl       black   bl
       {  0,     {{0x35,    8},    {0x37,   10}}},    //    00110101 |     0000110111
       {  1,     {{0x07,    6},    {0x02,    3}}},    //      000111 |            010
       {  2,     {{0x07,    4},    {0x03,    2}}},    //        0111 |             11
       {  3,     {{0x08,    4},    {0x02,    2}}},    //        1000 |             10
       {  4,     {{0x0B,    4},    {0x03,    3}}},    //        1011 |            011
       {  5,     {{0x0C,    4},    {0x03,    4}}},    //        1100 |           0011
       {  6,     {{0x0E,    4},    {0x02,    4}}},    //        1110 |           0010
       {  7,     {{0x0F,    4},    {0x03,    5}}},    //        1111 |          00011
       {  8,     {{0x13,    5},    {0x05,    6}}},    //       10011 |         000101
       {  9,     {{0x14,    5},    {0x04,    6}}},    //       10100 |         000100
       { 10,     {{0x07,    5},    {0x04,    7}}},    //       00111 |        0000100
       { 11,     {{0x08,    5},    {0x05,    7}}},    //       01000 |        0000101
       { 12,     {{0x08,    6},    {0x07,    7}}},    //      001000 |        0000111
       { 13,     {{0x03,    6},    {0x04,    8}}},    //      000011 |       00000100
       { 14,     {{0x34,    6},    {0x07,    8}}},    //      110100 |       00000111
       { 15,     {{0x35,    6},    {0x18,    9}}},    //      110101 |      000011000
       { 16,     {{0x2A,    6},    {0x17,   10}}},    //      101010 |     0000010111
       { 17,     {{0x2B,    6},    {0x18,   10}}},    //      101011 |     0000011000
       { 18,     {{0x27,    7},    {0x08,   10}}},    //     0100111 |     0000001000
       { 19,     {{0x0C,    7},    {0x67,   11}}},    //     0001100 |    00001100111
       { 20,     {{0x08,    7},    {0x68,   11}}},    //     0001000 |    00001101000
       { 21,     {{0x17,    7},    {0x6C,   11}}},    //     0010111 |    00001101100
       { 22,     {{0x03,    7},    {0x37,   11}}},    //     0000011 |    00000110111
       { 23,     {{0x04,    7},    {0x28,   11}}},    //     0000100 |    00000101000
       { 24,     {{0x28,    7},    {0x17,   11}}},    //     0101000 |    00000010111
       { 25,     {{0x2B,    7},    {0x18,   11}}},    //     0101011 |    00000011000
       { 26,     {{0x13,    7},    {0xCA,   12}}},    //     0010011 |   000011001010
       { 27,     {{0x24,    7},    {0xCB,   12}}},    //     0100100 |   000011001011
       { 28,     {{0x18,    7},    {0xCC,   12}}},    //     0011000 |   000011001100
       { 29,     {{0x02,    8},    {0xCD,   12}}},    //    00000010 |   000011001101
       { 30,     {{0x03,    8},    {0x68,   12}}},    //    00000011 |   000001101000
       { 31,     {{0x1A,    8},    {0x69,   12}}},    //    00011010 |   000001101001
       { 32,     {{0x1B,    8},    {0x6A,   12}}},    //    00011011 |   000001101010
       { 33,     {{0x12,    8},    {0x6B,   12}}},    //    00010010 |   000001101011
       { 34,     {{0x13,    8},    {0xD2,   12}}},    //    00010011 |   000011010010
       { 35,     {{0x14,    8},    {0xD3,   12}}},    //    00010100 |   000011010011
       { 36,     {{0x15,    8},    {0xD4,   12}}},    //    00010101 |   000011010100
       { 37,     {{0x16,    8},    {0xD5,   12}}},    //    00010110 |   000011010101
       { 38,     {{0x17,    8},    {0xD6,   12}}},    //    00010111 |   000011010110
       { 39,     {{0x28,    8},    {0xD7,   12}}},    //    00101000 |   000011010111
       { 40,     {{0x29,    8},    {0x6C,   12}}},    //    00101001 |   000001101100
       { 41,     {{0x2A,    8},    {0x6D,   12}}},    //    00101010 |   000001101101
       { 42,     {{0x2B,    8},    {0xDA,   12}}},    //    00101011 |   000011011010
       { 43,     {{0x2C,    8},    {0xDB,   12}}},    //    00101100 |   000011011011
       { 44,     {{0x2D,    8},    {0x54,   12}}},    //    00101101 |   000001010100
       { 45,     {{0x04,    8},    {0x55,   12}}},    //    00000100 |   000001010101
       { 46,     {{0x05,    8},    {0x56,   12}}},    //    00000101 |   000001010110
       { 47,     {{0x0A,    8},    {0x57,   12}}},    //    00001010 |   000001010111
       { 48,     {{0x0B,    8},    {0x64,   12}}},    //    00001011 |   000001100100
       { 49,     {{0x52,    8},    {0x65,   12}}},    //    01010010 |   000001100101
       { 50,     {{0x53,    8},    {0x52,   12}}},    //    01010011 |   000001010010
       { 51,     {{0x54,    8},    {0x53,   12}}},    //    01010100 |   000001010011
       { 52,     {{0x55,    8},    {0x24,   12}}},    //    01010101 |   000000100100
       { 53,     {{0x24,    8},    {0x37,   12}}},    //    00100100 |   000000110111
       { 54,     {{0x25,    8},    {0x38,   12}}},    //    00100101 |   000000111000
       { 55,     {{0x58,    8},    {0x27,   12}}},    //    01011000 |   000000100111
       { 56,     {{0x59,    8},    {0x28,   12}}},    //    01011001 |   000000101000
       { 57,     {{0x5A,    8},    {0x58,   12}}},    //    01011010 |   000001011000
       { 58,     {{0x5B,    8},    {0x59,   12}}},    //    01011011 |   000001011001
       { 59,     {{0x4A,    8},    {0x2B,   12}}},    //    01001010 |   000000101011
       { 60,     {{0x4B,    8},    {0x2C,   12}}},    //    01001011 |   000000101100
       { 61,     {{0x32,    8},    {0x5A,   12}}},    //    00110010 |   000001011010
       { 62,     {{0x33,    8},    {0x66,   12}}},    //    00110011 |   000001100110
       { 63,     {{0x34,    8},    {0x67,   12}}},    //    00110100 |   000001100111

        // N*64 codes located at index N+63
       { 64,     {{0x1B,    5},    {0x0F,   10}}},    //         11011 |     0000001111
       { 128,    {{0x12,    5},    {0xC8,   12}}},    //         10010 |   000011001000
       { 192,    {{0x17,    6},    {0xC9,   12}}},    //        010111 |   000011001001
       { 256,    {{0x37,    7},    {0x5B,   12}}},    //       0110111 |   000001011011
       { 320,    {{0x36,    8},    {0x33,   12}}},    //      00110110 |   000000110011
       { 384,    {{0x37,    8},    {0x34,   12}}},    //      00110111 |   000000110100
       { 448,    {{0x64,    8},    {0x35,   12}}},    //      01100100 |   000000110101
       { 512,    {{0x65,    8},    {0x6C,   13}}},    //      01100101 |  0000001101100
       { 576,    {{0x68,    8},    {0x6D,   13}}},    //      01101000 |  0000001101101
       { 640,    {{0x67,    8},    {0x4A,   13}}},    //      01100111 |  0000001001010
       { 704,    {{0xCC,    9},    {0x4B,   13}}},    //     011001100 |  0000001001011
       { 768,    {{0xCD,    9},    {0x4C,   13}}},    //     011001101 |  0000001001100
       { 832,    {{0xD2,    9},    {0x4D,   13}}},    //     011010010 |  0000001001101
       { 896,    {{0xD3,    9},    {0x72,   13}}},    //     011010011 |  0000001110010
       { 960,    {{0xD4,    9},    {0x73,   13}}},    //     011010100 |  0000001110011
       { 1024,   {{0xD5,    9},    {0x74,   13}}},    //     011010101 |  0000001110100
       { 1088,   {{0xD6,    9},    {0x75,   13}}},    //     011010110 |  0000001110101
       { 1152,   {{0xD7,    9},    {0x76,   13}}},    //     011010111 |  0000001110110
       { 1216,   {{0xD8,    9},    {0x77,   13}}},    //     011011000 |  0000001110111
       { 1280,   {{0xD9,    9},    {0x52,   13}}},    //     011011001 |  0000001010010
       { 1344,   {{0xDA,    9},    {0x53,   13}}},    //     011011010 |  0000001010011
       { 1408,   {{0xDB,    9},    {0x54,   13}}},    //     011011011 |  0000001010100
       { 1472,   {{0x98,    9},    {0x55,   13}}},    //     010011000 |  0000001010101
       { 1536,   {{0x99,    9},    {0x5A,   13}}},    //     010011001 |  0000001011010
       { 1600,   {{0x9A,    9},    {0x5B,   13}}},    //     010011010 |  0000001011011
       { 1664,   {{0x18,    6},    {0x64,   13}}},    //        011000 |  0000001100100
       { 1728,   {{0x9B,    9},    {0x65,   13}}},    //     010011011 |  0000001100101
       { 1792,   {{0x08,   11},    {0x08,   11}}},    //   00000001000 |    00000001000
       { 1856,   {{0x0C,   11},    {0x0C,   11}}},    //   00000001100 |    00000001100
       { 1920,   {{0x0D,   11},    {0x0D,   11}}},    //   00000001101 |    00000001101
       { 1984,   {{0x12,   12},    {0x12,   12}}},    //  000000010010 |   000000010010
       { 2048,   {{0x13,   12},    {0x13,   12}}},    //  000000010011 |   000000010011
       { 2112,   {{0x14,   12},    {0x14,   12}}},    //  000000010100 |   000000010100
       { 2176,   {{0x15,   12},    {0x15,   12}}},    //  000000010101 |   000000010101
       { 2240,   {{0x16,   12},    {0x16,   12}}},    //  000000010110 |   000000010110
       { 2304,   {{0x17,   12},    {0x17,   12}}},    //  000000010111 |   000000010111
       { 2368,   {{0x1C,   12},    {0x1C,   12}}},    //  000000011100 |   000000011100
       { 2432,   {{0x1D,   12},    {0x1D,   12}}},    //  000000011101 |   000000011101
       { 2496,   {{0x1E,   12},    {0x1E,   12}}},    //  000000011110 |   000000011110
       { 2560,   {{0x1F,   12},    {0x1F,   12}}},    //  000000011111 |   000000011111

       {   -1,   {{0x01,   12},    {0x01,   12}}},    //  000000000001 | 000000000001

       { -666,   {{0x01,    9},    {0x01,    9}}},    //     000000001 |    000000001
       { -666,   {{0x01,   10},    {0x01,   10}}},    //    0000000001 |   0000000001
       { -666,   {{0x01,   11},    {0x01,   11}}},    //   00000000001 |  00000000001
       { -666,   {{0x00,   12},    {0x00,   12}}}     //  000000000000 | 000000000000
};

#endif  // DESIRED_CODE()

static short decode_table[2][220][13] = {
    {
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 00
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 01
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  29,  -1,  -1,  -1,  -1,  -1},  // 02
     {-1,  -1,  -1,  -1,  -1,  13,  22,  30,  -1,  -1,  -1,  -1,  -1},  // 03
     {-1,  -1,  -1,  -1,  -1,  -1,  23,  45,  -1,  -1,  -1,  -1,  -1},  // 04
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  46,  -1,  -1,  -1,  -1,  -1},  // 05
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 06
     {-1,  -1,  -1,   2,  10,   1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 07
     {-1,  -1,  -1,   3,  11,  12,  20,  -1,  -1,  -1,1792,  -1,  -1},  // 08
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 09
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  47,  -1,  -1,  -1,  -1,  -1},  // 0A
     {-1,  -1,  -1,   4,  -1,  -1,  -1,  48,  -1,  -1,  -1,  -1,  -1},  // 0B
     {-1,  -1,  -1,   5,  -1,  -1,  19,  -1,  -1,  -1,1856,  -1,  -1},  // 0C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1920,  -1,  -1},  // 0D
     {-1,  -1,  -1,   6,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 0E
     {-1,  -1,  -1,   7,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 0F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 10
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 11
     {-1,  -1,  -1,  -1, 128,  -1,  -1,  33,  -1,  -1,  -1,1984,  -1},  // 12
     {-1,  -1,  -1,  -1,   8,  -1,  26,  34,  -1,  -1,  -1,2048,  -1},  // 13
     {-1,  -1,  -1,  -1,   9,  -1,  -1,  35,  -1,  -1,  -1,2112,  -1},  // 14
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  36,  -1,  -1,  -1,2176,  -1},  // 15
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  37,  -1,  -1,  -1,2240,  -1},  // 16
     {-1,  -1,  -1,  -1,  -1, 192,  21,  38,  -1,  -1,  -1,2304,  -1},  // 17
     {-1,  -1,  -1,  -1,  -1,1664,  28,  -1,  -1,  -1,  -1,  -1,  -1},  // 18
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 19
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  31,  -1,  -1,  -1,  -1,  -1},  // 1A
     {-1,  -1,  -1,  -1,  64,  -1,  -1,  32,  -1,  -1,  -1,  -1,  -1},  // 1B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2368,  -1},  // 1C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2432,  -1},  // 1D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2496,  -1},  // 1E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2560,  -1},  // 1F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 20
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 21
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 22
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 23
     {-1,  -1,  -1,  -1,  -1,  -1,  27,  53,  -1,  -1,  -1,  -1,  -1},  // 24
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  54,  -1,  -1,  -1,  -1,  -1},  // 25
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 26
     {-1,  -1,  -1,  -1,  -1,  -1,  18,  -1,  -1,  -1,  -1,  -1,  -1},  // 27
     {-1,  -1,  -1,  -1,  -1,  -1,  24,  39,  -1,  -1,  -1,  -1,  -1},  // 28
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,  -1,  -1,  -1},  // 29
     {-1,  -1,  -1,  -1,  -1,  16,  -1,  41,  -1,  -1,  -1,  -1,  -1},  // 2A
     {-1,  -1,  -1,  -1,  -1,  17,  25,  42,  -1,  -1,  -1,  -1,  -1},  // 2B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  43,  -1,  -1,  -1,  -1,  -1},  // 2C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  44,  -1,  -1,  -1,  -1,  -1},  // 2D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 30
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 31
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  61,  -1,  -1,  -1,  -1,  -1},  // 32
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  62,  -1,  -1,  -1,  -1,  -1},  // 33
     {-1,  -1,  -1,  -1,  -1,  14,  -1,  63,  -1,  -1,  -1,  -1,  -1},  // 34
     {-1,  -1,  -1,  -1,  -1,  15,  -1,   0,  -1,  -1,  -1,  -1,  -1},  // 35
     {-1,  -1,  -1,  -1,  -1,  -1,  -1, 320,  -1,  -1,  -1,  -1,  -1},  // 36
     {-1,  -1,  -1,  -1,  -1,  -1, 256, 384,  -1,  -1,  -1,  -1,  -1},  // 37
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 38
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 39
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 40
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 41
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 42
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 43
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 44
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 45
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 46
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 47
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 48
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 49
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  59,  -1,  -1,  -1,  -1,  -1},  // 4A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  60,  -1,  -1,  -1,  -1,  -1},  // 4B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 50
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 51
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  49,  -1,  -1,  -1,  -1,  -1},  // 52
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  50,  -1,  -1,  -1,  -1,  -1},  // 53
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  51,  -1,  -1,  -1,  -1,  -1},  // 54
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  52,  -1,  -1,  -1,  -1,  -1},  // 55
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 56
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 57
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  55,  -1,  -1,  -1,  -1,  -1},  // 58
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  56,  -1,  -1,  -1,  -1,  -1},  // 59
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  -1,  -1,  -1,  -1,  -1},  // 5A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  58,  -1,  -1,  -1,  -1,  -1},  // 5B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 60
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 61
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 62
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 63
     {-1,  -1,  -1,  -1,  -1,  -1,  -1, 448,  -1,  -1,  -1,  -1,  -1},  // 64
     {-1,  -1,  -1,  -1,  -1,  -1,  -1, 512,  -1,  -1,  -1,  -1,  -1},  // 65
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 66
     {-1,  -1,  -1,  -1,  -1,  -1,  -1, 640,  -1,  -1,  -1,  -1,  -1},  // 67
     {-1,  -1,  -1,  -1,  -1,  -1,  -1, 576,  -1,  -1,  -1,  -1,  -1},  // 68
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 69
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 70
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 71
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 72
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 73
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 74
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 75
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 76
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 77
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 78
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 79
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 80
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 81
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 82
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 83
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 84
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 85
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 86
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 87
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 88
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 89
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 90
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 91
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 92
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 93
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 94
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 95
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 96
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 97
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1472,  -1,  -1,  -1,  -1},  // 98
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1536,  -1,  -1,  -1,  -1},  // 99
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1600,  -1,  -1,  -1,  -1},  // 9A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1728,  -1,  -1,  -1,  -1},  // 9B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 704,  -1,  -1,  -1,  -1},  // CC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 768,  -1,  -1,  -1,  -1},  // CD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 832,  -1,  -1,  -1,  -1},  // D2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 896,  -1,  -1,  -1,  -1},  // D3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 960,  -1,  -1,  -1,  -1},  // D4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1024,  -1,  -1,  -1,  -1},  // D5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1088,  -1,  -1,  -1,  -1},  // D6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1152,  -1,  -1,  -1,  -1},  // D7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1216,  -1,  -1,  -1,  -1},  // D8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1280,  -1,  -1,  -1,  -1},  // D9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1344,  -1,  -1,  -1,  -1},  // DA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1408,  -1,  -1,  -1,  -1},  // DB
    },
    {
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 00
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 01
     {-1,   3,   1,   6,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 02
     {-1,   2,   4,   5,   7,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 03
     {-1,  -1,  -1,  -1,  -1,   9,  10,  13,  -1,  -1,  -1,  -1,  -1},  // 04
     {-1,  -1,  -1,  -1,  -1,   8,  11,  -1,  -1,  -1,  -1,  -1,  -1},  // 05
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 06
     {-1,  -1,  -1,  -1,  -1,  -1,  12,  14,  -1,  -1,  -1,  -1,  -1},  // 07
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  18,1792,  -1,  -1},  // 08
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 09
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 0A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 0B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1856,  -1,  -1},  // 0C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1920,  -1,  -1},  // 0D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 0E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  64,  -1,  -1,  -1},  // 0F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 10
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 11
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1984,  -1},  // 12
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2048,  -1},  // 13
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2112,  -1},  // 14
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2176,  -1},  // 15
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2240,  -1},  // 16
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  16,  24,2304,  -1},  // 17
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  15,  17,  25,  -1,  -1},  // 18
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 19
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 1A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 1B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2368,  -1},  // 1C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2432,  -1},  // 1D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2496,  -1},  // 1E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,2560,  -1},  // 1F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 20
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 21
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 22
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 23
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  52,  -1},  // 24
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 25
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 26
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  55,  -1},  // 27
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  23,  56,  -1},  // 28
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 29
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  59,  -1},  // 2B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  60,  -1},  // 2C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 30
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 31
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 32
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 320,  -1},  // 33
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 384,  -1},  // 34
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 448,  -1},  // 35
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 36
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,  22,  53,  -1},  // 37
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  54,  -1},  // 38
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 39
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 3F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 40
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 41
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 42
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 43
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 44
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 45
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 46
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 47
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 48
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 49
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 640},  // 4A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 704},  // 4B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 768},  // 4C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 832},  // 4D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 4F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 50
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 51
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  50,1280},  // 52
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  51,1344},  // 53
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  44,1408},  // 54
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  45,1472},  // 55
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  46,  -1},  // 56
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  47,  -1},  // 57
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  57,  -1},  // 58
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  58,  -1},  // 59
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  61,1536},  // 5A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 256,1600},  // 5B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 60
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 61
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 62
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 63
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  48,1664},  // 64
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  49,1728},  // 65
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  62,  -1},  // 66
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  19,  63,  -1},  // 67
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  20,  30,  -1},  // 68
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  31,  -1},  // 69
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  32,  -1},  // 6A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  33,  -1},  // 6B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  21,  40, 512},  // 6C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  41, 576},  // 6D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 6F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 70
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 71
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 896},  // 72
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 960},  // 73
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1024},  // 74
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1088},  // 75
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1152},  // 76
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,1216},  // 77
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 78
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 79
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 7F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 80
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 81
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 82
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 83
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 84
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 85
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 86
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 87
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 88
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 89
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 8F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 90
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 91
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 92
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 93
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 94
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 95
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 96
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 97
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 98
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 99
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9A
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9B
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9C
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9D
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9E
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 9F
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // A9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // AF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // B9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // BF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // C7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 128,  -1},  // C8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 192,  -1},  // C9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  26,  -1},  // CA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  27,  -1},  // CB
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  28,  -1},  // CC
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  29,  -1},  // CD
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CE
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // CF
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D0
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D1
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  34,  -1},  // D2
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  35,  -1},  // D3
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  36,  -1},  // D4
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  37,  -1},  // D5
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  38,  -1},  // D6
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  39,  -1},  // D7
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D8
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // D9
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  42,  -1},  // DA
     {-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  43,  -1}   // DB
    }
};

static char encode_table[256][2] = {
    {8, 0},  {7, 0},  {6, 0},  {6, 0},  {5, 0},  {5, 0},  {5, 0},  {5, 0},       // 00-07
    {4, 0},  {4, 0},  {4, 0},  {4, 0},  {4, 0},  {4, 0},  {4, 0},  {4, 0},       // 08-0F
    {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},       // 10-17
    {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},  {3, 0},       // 18-1F
    {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},       // 20-27
    {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},       // 28-2F
    {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},       // 30-37
    {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},  {2, 0},       // 38-3F
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 40-47
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 48-4F
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 50-57
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 58-5F
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 60-67
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 68-6F
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 70-77
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},       // 78-7F
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // 80-87
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // 88-8F
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // 90-97
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // 98-9F
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // A0-A7
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // A8-AF
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // B0-B7
    {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},  {0, 1},       // B8-BF
    {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},       // C0-C7
    {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},       // C8-CF
    {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},       // D0-D7
    {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},  {0, 2},       // D8-DF
    {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},       // E0-E7
    {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},  {0, 3},       // E8-EF
    {0, 4},  {0, 4},  {0, 4},  {0, 4},  {0, 4},  {0, 4},  {0, 4},  {0, 4},       // F0-F7
    {0, 5},  {0, 5},  {0, 5},  {0, 5},  {0, 6},  {0, 6},  {0, 7},  {0, 8}        // F8-FF
};


///////////////////////////////////////////////////////////////////////////
// ==========================================
// Group 3X Methods:
inline WT_Result WT_Image::add_code (WT_Huffman::Code const &    code)
{
    if ((m_data_size + 3) >= m_data_allocated) // The 2 accounts for the longest code (12 bits) that we could be adding below
    {
        // We ran out of data

        // Hold onto the old data
        WT_Byte *   old_data = m_data;

        // Allocate more space.
        m_data = new WT_Byte[m_data_allocated + 0xFFFF];
        if (!m_data)
            return WT_Result::Out_Of_Memory_Error;

        // Clear what will be the unused portions of the new array to zeros: this is required for other routines!
        memset (m_data + m_data_allocated, 0, 0xFFFF);

        // Now copy the old data into the new array
        memcpy(m_data, old_data, m_data_allocated);

        m_data_allocated += 0xFFFF;

        // Free the old data
        delete [] old_data;

    }

    WT_Byte *   current_dest_pos = m_data + m_data_size;
    int         value            = code.value;

    for (int bit_number = code.length - 1; bit_number >= 0; bit_number--)
    {
        *current_dest_pos |= ((value >> bit_number) & 0x01) << m_dst_bits_used;
        if (++m_dst_bits_used == 8)
        {
            current_dest_pos++;
            m_dst_bits_used = 0;
        }
    }

    m_data_size = (WT_Integer32)(current_dest_pos - m_data);
    WD_Assert(m_data_size == current_dest_pos - m_data); // Maximum 2GB

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
inline WT_Result WT_Image::output_group3x_code(int run_color, int run_length, WT_Boolean skip_trailing_zero)
{
    while (run_length >= 2560)
    {
        int     code_index = (2560/64)+63;
        WD_CHECK (add_code (huffman[code_index].code[run_color]));
        run_length -= 2560;     // huffman[code_index].run
    }

    if (run_length >= 64)
    {
        int     code_index = (run_length/64)+63;
        WD_CHECK (add_code (huffman[code_index].code[run_color]));
        run_length -= huffman[code_index].run;
    }

    if (skip_trailing_zero && run_length == 0)
        return WT_Result::Success;

    return add_code (huffman[run_length].code[run_color]);
}
#else
inline WT_Result WT_Image::output_group3x_code(int, int, WT_Boolean)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::convert_bitonal_to_group_3X ()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (m_format == Group3X_Mapped)
        return WT_Result::Success;  // Already done, don't need to do anything

    if (m_format != Bitonal_Mapped || !m_color_map || m_color_map->size() != 2)
        return WT_Result::Toolkit_Usage_Error;

    if (!m_data_size || !m_data)
        return WT_Result::Corrupt_File_Error;

    // Initialize
    WT_Byte *     save_source_data = m_data;
    WT_Byte *     source_bits = save_source_data;

    m_data_size = 0;
    m_dst_bits_used = 0;
    // Try and guess how big our output image will be and allocate that much memory to hold it.
    m_data_allocated = (int) ( (float) columns() * (float) rows() / 8.0f *    // Orig. number of bytes
                               (float) WD_EXPECTED_GROUP3X_FACTOR * 1.2f   ); // Conservative compression ration, not expected; try to minimize realloc's.
    m_data = new WT_Byte[m_data_allocated];
    if (!m_data)
        return WT_Result::Out_Of_Memory_Error;
    memset(m_data, 0, m_data_allocated);    // Note: this must be done!

    for (int raster = 0; raster < rows(); raster++)
    {
        int     run_color = WT_Huffman::Background;
        int     run_length;

        // specify type of encoding for this raster
        WD_CHECK (add_code (Group3));


        // While we haven't reached the end of this scan line, find the next run length
        int     columns_left = columns();
        int     src_bits_loaded = 0;
        int     byte_value = 0;

        while (columns_left > 0)
        {
            run_length = 0;
            while (columns_left > 0)
            {
                if (src_bits_loaded == 0)           // need another byte's worth of bits
                {
                    byte_value = *source_bits++;
                    src_bits_loaded = 8;
                }

                int     some_bits = encode_table[byte_value][run_color];

                if (some_bits == 0)                 // no more bits of the desired color
                    break;

                if (some_bits > src_bits_loaded)    // ignore "extra" bits (zeroes) from shifting
                    some_bits = src_bits_loaded;
                if (some_bits > columns_left)       // ignore bits from padded input
                    some_bits = columns_left;

                run_length += some_bits;            // keep these
                columns_left -= some_bits;

                src_bits_loaded -= some_bits;
                if (src_bits_loaded != 0)           // did we change color (or need more data)?
                {
                    byte_value <<= some_bits;       // prepare remaining bits for next run
                    byte_value &= 0x00FF;
                    break;
                }
            }

            // We either reached the end of the scan line, or our run of color ended
            // Now encode the run and send it to the destination
            WD_CHECK (output_group3x_code(run_color, run_length, columns_left == 0));

            run_color = 1 - run_color;
        } // while (there are still pixels in this scan line unprocessed)

        // source_bits already points at the next possible input byte, so we don't
        // need to do anything special to skip any pad bits (as src_bits_loaded is reset to
        // zero at the beginning of each raster).

    } // For (each raster)

    // We finished the entire image, but we need to pad the last code if it didn't fill a byte
    // Note that the memory should already have been allocated.
    if (m_dst_bits_used)
        m_data_size++;

    // Now delete the source data that we aren't using anymore;
    if (m_local_data_copy)
    {
        delete [] save_source_data;
    }

    m_local_data_copy = WD_True;
    m_format = Group3X_Mapped;

    // Tada! All done!
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
} // convert_bitonal_to_group3X

///////////////////////////////////////////////////////////////////////////
inline int WT_Image::get_next_bit()
{
    // TODO: use a data pointer rather than slow subscripts (no multiplication)
    int value = (m_source_data[m_src_byte_position] >> m_src_bits_used) & 0x0001;

    if (++m_src_bits_used == 8)
    {
        m_src_byte_position++;
        m_src_bits_used = 0;
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::convert_group_3X_to_mapped()
{
    if (m_format == Mapped)
        return WT_Result::Success;  // Already done, don't need to do anything

    if (m_format != Group3X_Mapped)
        return WT_Result::Toolkit_Usage_Error;

    // Initialize

    // Hold on to our orig. data in group3X format and prepare a new image array to hold the Mapped data
    m_source_data = m_data;
    m_data_size = rows() * columns();
    m_src_bits_used = 0;
    m_src_byte_position = 0;

    m_data = new WT_Byte [m_data_size];

    if (!m_data)
        return WT_Result::Out_Of_Memory_Error;

    WT_Byte * dest_pos = m_data;

#ifdef WD_NEED_DEBUG_PRINTF
    WT_Boolean  trace_active = WD_False;
#endif

    for (int raster = 0; raster < rows(); raster++)
    {
        // Get the initial two bits of the raster and use to see how this raster is encoded.
        int raster_encoding;

        raster_encoding =  get_next_bit() << 1;
        raster_encoding += get_next_bit();

        switch (raster_encoding)
        {
        case WT_Huffman::Literal:
            {
                // Read literal data out of this raster and expand into a byte per pixel.
                for (int cur_col = 0; cur_col < columns(); cur_col++)
                {
                    *dest_pos++ = (WT_Byte) get_next_bit();
                }
            } break;
        case WT_Huffman::Group3:
        case WT_Huffman::Group3X:
            {
                // Decode this raster as a set of Group3 Huffman run lengths
                int     cur_col = 0;
                int     color   = WT_Huffman::Background;
                int     count   = -1;

                while (cur_col < columns())
                {
                    int             code_length     = 0;
                    int             code            = 0;

                    while (count < 0)
                    {
                        code <<= 1;
                        code |= get_next_bit();

                        if (++code_length > 13 || code > 0xDB)
                            return WT_Result::Corrupt_File_Error;       // lost

                        count = decode_table[color][code][code_length-1];
                    }

                    // We have a run-length, fill in the destination image with a run of color
                    int     save_count = count;

                    cur_col += count;

                    if (cur_col > columns())
                        return WT_Result::Corrupt_File_Error;

                    while (count-- > 0)
                        *dest_pos++ = (WT_Byte) color;

                    if (save_count < 64)
                        color = 1 - color;
                } // while (cur_col < columns())


                if (raster_encoding == WT_Huffman::Group3X)
                {
                    // We need to XOR all the pixels with the previous raster
                    WT_Byte *   src_pos = m_data + ( (raster - 1) * columns() );
                    WT_Byte *   dst_pos = m_data + (  raster      * columns() );

                    for (cur_col = 0; cur_col < columns(); cur_col++)
                    {
                        *dst_pos++ ^= *src_pos++;
                    } // for (each column)
                } // If (Group3X)
            } break;
        default:
            return WT_Result::Corrupt_File_Error;

        } // switch (raster_encoding)

    } // for (each raster)


    // Now delete the source data that we aren't using anymore;
    if (m_local_data_copy)
    {
        delete [] m_source_data;
        m_source_data = WD_Null;
    }

    m_local_data_copy = WD_True;
    m_format = Mapped;

    return WT_Result::Success;
} // convert_group3x_to_mapped;

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Image::convert_group_3X_to_bitonal()
{
    if (m_format == Bitonal_Mapped)
        return WT_Result::Success;  // Already done, don't need to do anything

    if (m_format != Group3X_Mapped)
        return WT_Result::Toolkit_Usage_Error;

    // Initialize

    // Hold on to our orig. data in group3X format and prepare a new image array to hold the Mapped data
    m_source_data = m_data;
    int     row_bytes = (columns() + 7) / 8;
    m_data_size = rows() * row_bytes;
    m_src_bits_used = 0;
    m_src_byte_position = 0;

    m_data = new WT_Byte [m_data_size];

    if (!m_data)
        return WT_Result::Out_Of_Memory_Error;

    WT_Byte *   dest_pos = m_data;
    int         dest_bits_used = 0;

#ifdef WD_NEED_DEBUG_PRINTF
    WT_Boolean  trace_active = WD_False;
#endif

    for (int raster = 0; raster < rows(); raster++)
    {
        // Get the initial two bits of the raster and use to see how this raster is encoded.
        int raster_encoding;

        raster_encoding =  get_next_bit() << 1;
        raster_encoding += get_next_bit();

        switch (raster_encoding)
        {
        case WT_Huffman::Literal:
            {
                // Read literal data out of this raster and expand into a byte per pixel.
                for (int cur_col = 0; cur_col < columns(); cur_col++)
                {
                    *dest_pos++ = (WT_Byte) get_next_bit();
                }
            } break;
        case WT_Huffman::Group3:
        case WT_Huffman::Group3X:
            {
                // Decode this raster as a set of Group3 Huffman run lengths
                int     cur_col = 0;
                char    color   = 0x00;

                while (cur_col < columns())
                {
                    int             code_length     = 0;
                    int             code            = 0;
                    int             count           = -1;

                    do
                    {
                        code <<= 1;
                        code |= get_next_bit();

                        if (++code_length > 13 || code > 0xDB)
                            return WT_Result::Corrupt_File_Error;       // lost

                        count = decode_table[color & 1][code][code_length-1];
                    }
                    while (count < 0);

                    // We have a run-length, fill in the destination image with a run of color
                    int     save_count = count;

                    cur_col += count;

                    if (cur_col > columns())
                        return WT_Result::Corrupt_File_Error;

                    while (count >= 8-dest_bits_used)
                    {
                        *dest_pos++ = (char)(*dest_pos & (0xFF00 >> dest_bits_used) |
                                             color & (0x00FF >> dest_bits_used));
                        count -= (8-dest_bits_used);
                        dest_bits_used = 0;
                    }
                    if (count > 0)
                    {
                        *dest_pos = (char)(*dest_pos & (0xFF00 >> dest_bits_used) |
                                           color &  (0x00FF >> dest_bits_used));
                        dest_bits_used += count;
                    }

                    if (save_count < 64)
                        color = ~color;
                } // while (cur_col < columns())


                if (raster_encoding == WT_Huffman::Group3X)
                {
                    // We need to XOR all the pixels with the previous raster
                    WT_Byte *   src_pos = m_data + ( (raster - 1) * columns() );
                    WT_Byte *   dst_pos = m_data + (  raster      * columns() );

                    for (cur_col = 0; cur_col < columns(); cur_col++)
                    {
                        *dst_pos++ ^= *src_pos++;
                    } // for (each column)
                } // If (Group3X)
            } break;
        default:
            return WT_Result::Corrupt_File_Error;

        } // switch (raster_encoding)

        if (dest_bits_used != 0)
        {
            ++dest_pos;
            dest_bits_used = 0;
        }
    } // for (each raster)


    // Now delete the source data that we aren't using anymore;
    if (m_local_data_copy)
    {
        delete [] m_source_data;
        m_source_data = WD_Null;
    }

    m_local_data_copy = WD_True;
    m_format = Bitonal_Mapped;

    return WT_Result::Success;
} // convert_group3x_to_bitonal;
