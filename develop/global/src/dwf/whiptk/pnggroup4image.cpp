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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/pnggroup4image.cpp 1     9/12/04 8:56p Evansg $


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif

///////////////////////////////////////////////////////////////////////////
WT_PNG_Group4_Image::WT_PNG_Group4_Image(
    WT_Unsigned_Integer16     rows,
    WT_Unsigned_Integer16       columns,
    WT_PNG_Group4_Image_Format  format,
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
             WD_Assert (m_format == Group4X_Mapped);  // Shouldn't be passing in a colormap if we don't have
                                                      // a mappable image type

            // Note that the "default" colormap changed at version REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED
            m_color_map = new WT_Color_Map(); // It doesn't matter what version of the map we start with
                                              // since we're going to replace it.
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
WT_Result WT_PNG_Group4_Image::set(WT_Color_Map const* pColorMap)
{
    if(!pColorMap)
        return WT_Result::Internal_Error;
    
    WD_Assert (m_format == Group4X_Mapped);  // Shouldn't be passing in a colormap if we don't have
                                             // a mappable image type

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
WT_Object::WT_ID    WT_PNG_Group4_Image::object_id() const
{
    return PNG_Group4_Image_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_PNG_Group4_Image::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    WD_Assert(m_rows     > 0);
    WD_Assert(m_columns  > 0);
    WD_Assert(m_data);

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
    case Group4X_Mapped:
        WD_Assert(m_color_map); // There had better be a colormap attached
        if (!m_color_map)
            return WT_Result::File_Write_Error;

        WD_Assert(m_color_map->size() == 2);
        colormap_size = 1 +                         // For the colormap size byte
                        m_color_map->size() * 4;    // The colormap itself

        break;
    case Group4:
    case PNG:
        WD_Assert(!m_color_map); // There should not be any colormap attached
        break;
    default:
        return WT_Result::Internal_Error;
    } // switch

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
        ((WT_PNG_Group4_Image *)this)->transform(file.heuristics().transform());

    if (file.heuristics().allow_binary_data())
    {
        ((WT_PNG_Group4_Image *)this)->relativize(file);

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
        case Group4X_Mapped:
        case Group4:
        case PNG:
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
        WD_CHECK (file.write("(Group4PNGImage "));

        switch (format())
        {
        case Group4X_Mapped:
                                WD_CHECK (file.write_quoted_string("group 4X", WD_True));
                                break;
        case Group4:
                                WD_CHECK (file.write_quoted_string("Group4", WD_True));
                                break;
        case PNG:
                                WD_CHECK (file.write_quoted_string("PNG", WD_True));
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

        if ((colormap_size != 0) && (format() == Group4X_Mapped))
        {
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii((WT_Unsigned_Integer16)colormap_size));
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
WT_Result WT_PNG_Group4_Image::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PNG_Group4_Image::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Unsigned_Integer16 colormap_size = 0;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Columns;
            // No break;

        case Getting_Columns:
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
            if (m_format == Group4X_Mapped )
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
            else {
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

            switch (m_format)
            {
            case Group4X_Mapped:
            case Group4:
            case PNG:
                if (!m_data)
                {
                    m_data = new WT_Byte[m_data_size];
                    if (!m_data)
                        return WT_Result::Out_Of_Memory_Error;
                    m_local_data_copy = WD_True;
                }
                WD_CHECK (file.read(m_data_size, m_data));

                break;
            default:
                return WT_Result::Corrupt_File_Error;
            } // switch (m_format)

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
            if (!strcmp(format_name, "group 4X"))
                m_format = Group4X_Mapped;
            else if (!strcmp(format_name, "Group4"))
                m_format = Group4;
            else if (!strcmp(format_name, "PNG"))
                m_format = PNG;
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
            if (m_format != Group4X_Mapped )
            {
                m_stage = Getting_Pre_Data_Size_Whitespace;
                goto Getting_Pre_Data_Size_Whitespace_Hop;
            }
            m_stage = Getting_Color_Map_Size;

        case Getting_Color_Map_Size:
            WD_CHECK (file.read_ascii(colormap_size));
            m_stage = Getting_Color_Map_Opcode;

            // No break
        case Getting_Color_Map_Opcode:

            if(colormap_size != 0) {
            WD_CHECK (m_colormap_opcode.get_opcode(file));

            m_color_map = new WT_Color_Map();   // Normally we would have to specify
                                                // a version number with the colormap
                                                // constructor since the default map used depends on the file version we are dealing with.
                                                // However, since we are going to "overwrite" the default map, it doesn't matter which one we get.
            if (!m_color_map)
                return WT_Result::Out_Of_Memory_Error;
            m_local_color_map_copy = WD_True;
            m_color_map->clear();
            }
            m_stage = Getting_Color_Map;

            // No break
        case Getting_Color_Map:
            if(colormap_size != 0) {
            WD_CHECK (m_color_map->materialize(m_colormap_opcode, file));
            }
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

            if (!m_data)
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read_hex(m_data_size, m_data));

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


    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_PNG_Group4_Image::update_bounds(WT_File *)
{
    WT_Drawable::update_bounds(m_min_corner);
    WT_Drawable::update_bounds(m_max_corner);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PNG_Group4_Image::skip_operand(WT_Opcode const &, WT_File &)
{
    //TODO: implement
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void WT_PNG_Group4_Image::transform(WT_Transform const & transform)
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
void WT_PNG_Group4_Image::relativize(WT_File & file)
{
    if (!m_relativized)
    {
        m_min_corner = file.update_current_point(m_min_corner);
        m_max_corner = file.update_current_point(m_max_corner);
        m_relativized = WD_True;
    } // If (!relativized)
}
#else
void WT_PNG_Group4_Image::relativize(WT_File &)
{
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_PNG_Group4_Image::de_relativize(WT_File & file)
{
    if (m_relativized)
    {
        m_min_corner = file.de_update_current_point(m_min_corner);
        m_max_corner = file.de_update_current_point(m_max_corner);
        m_relativized = WD_False;
    } // If (!relativized)
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PNG_Group4_Image::process(WT_File & file)
{
    WD_Assert (file.image_action());
    return (file.png_group4_image_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PNG_Group4_Image::default_process(WT_PNG_Group4_Image &, WT_File &)
{
    return WT_Result::Success;
}
