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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/colormap.cpp 3     5/09/05 12:42a Evansg $

#include "whiptk/pch.h"

#include "whiptk/palette.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Color_Map::object_id() const
{
    return Color_Map_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map::WT_Color_Map(int file_revision_being_used) throw(WT_Result)
    : m_size(256)
    , m_incarnation(-1)
    , m_stage(Getting_Count)
    , m_map(WD_Null)
{
    m_map = new WT_RGBA32[m_size];
    if (m_map == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    // Load the default Autocad Palette, if the user hasn't chosen a
    // different one.

    // Note that before REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED
    // we had used a different colormap.  When constructing a default map,
    // you must supply the desired revision of the DWF spec to which you'd
    // like to work with.

    if (file_revision_being_used < REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED)
    {
        for (int loop = 0; loop < m_size; loop++)
        {
            m_map[loop] = WD_Old_Default_Palette[loop];
        }
    }
    else
    {
        for (int loop = 0; loop < m_size; loop++)
        {
            m_map[loop] = WD_New_Default_Palette[loop];
        }
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map::WT_Color_Map(int count, WT_RGBA32 const * map, WT_File & file) throw(WT_Result)
    : m_size(count)
    , m_incarnation(file.next_incarnation())
    , m_stage(Getting_Count)
    , m_map (WD_Null)
{
    WD_Assert(count >= 0);
    WD_Assert(count <= 256);

    m_map = new WT_RGBA32[count];
    if (m_map == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    WD_COPY_MEMORY(map, count * sizeof(WT_RGBA32), m_map);
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map::WT_Color_Map(int count, WT_RGB const * map, WT_File & file) throw(WT_Result)
    : m_size(count)
    , m_incarnation(file.next_incarnation())
    , m_stage(Getting_Count)
    , m_map(WD_Null)
{
    WD_Assert(count >= 0);
    WD_Assert(count <= 256);

    m_map = new WT_RGBA32[count];
    if (m_map == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    for (int loop = 0; loop < m_size; loop++)
    {
        m_map[loop] = map[loop];
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map::WT_Color_Map(WT_Color_Map const & cmap) throw(WT_Result)
    : WT_Attribute()
  , m_size(cmap.size())
  , m_incarnation(cmap.m_incarnation)
    , m_stage(Getting_Count)
  , m_map(WD_Null)
{
    m_map = new WT_RGBA32[m_size];
    if (m_map == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    for (int loop = 0; loop < m_size; loop++)
    {
        m_map[loop] = cmap.map()[loop];
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map::~WT_Color_Map()
{
    delete []m_map;
}

///////////////////////////////////////////////////////////////////////////
void WT_Color_Map::clear()
{
    m_size = 0;
    delete []m_map;
    m_map = WD_Null;
    m_incarnation = -2;
    m_stage = Getting_Count;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::set(int count, WT_RGB const * map, WT_File & file)
{
    WD_Assert(count >= 0);
    WD_Assert(count <= 256);

    delete []m_map;

    m_size = count;
    m_incarnation = file.next_incarnation();
    m_map = new WT_RGBA32[count];
    if (m_map == NULL)
        return WT_Result::Out_Of_Memory_Error;
    for (int loop = 0; loop < m_size; loop++)
    {
        m_map[loop] = map[loop];
    }

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::set(int count, WT_RGBA32 const * map, WT_File & file)
{
    WD_Assert(count >= 0);
    WD_Assert(count <= 256);

    delete []m_map;

    m_size = count;
    m_incarnation = file.next_incarnation();
    m_map = new WT_RGBA32[count];
    if (m_map == NULL)
        return WT_Result::Out_Of_Memory_Error;
    for (int loop = 0; loop < m_size; loop++)
    {
        m_map[loop] = map[loop];
    }

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Color_Map const & WT_Color_Map::operator=(WT_Color_Map const & cmap) throw(WT_Result)
{
    delete []m_map;

    m_stage = Getting_Count;
    m_size = cmap.size();
    m_incarnation = cmap.m_incarnation;
    m_map = new WT_RGBA32[m_size];
    if (m_map == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    if (m_map)
    {
        for (int loop = 0; loop < m_size; loop++)
        {
            m_map[loop] = cmap.map()[loop];
        }
    }
    else
    {
        // Ran out of memory
        m_size = 0;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Color_Map::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_Assert(m_size > 0 && m_size <= 256);

    if (file.heuristics().allow_binary_data())
    {
        // Extended Binary opcode
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (2 +            // Space for Opcode
                                             1 +            // Space for Num colors in map
                                             (m_size * 4) + // Space for N color definitions
                                             1)));          // Space for closing brace

        WD_CHECK (file.write((WT_Unsigned_Integer16) WD_COLOR_MAP_EXT_OPCODE));

        WD_CHECK (serialize_just_colors(file));
        return file.write((WT_Byte) '}');
    }
    else
    {
        // Extended ASCII opcode
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(ColorMap "));
        WD_CHECK (file.write_ascii(m_size));
        for (int loop = 0; loop < m_size; loop++)
        {
            if (!(loop % 4))
            {
                WD_CHECK (file.write_tab_level());
                WD_CHECK (file.write((WT_Byte) '\t'));
            }
            else
                WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii(m_map[loop]));
        }

        return file.write((WT_Byte) ')');
    }
}
#else
WT_Result WT_Color_Map::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Color_Map::serialize_just_colors(WT_File & file) const
{
    WD_Assert(m_size > 0 && m_size <= 256);

    // If we have a full 256 colors, we can't write this number into a byte,
    // so we write the value 0 instead which represents 256 colors.
    if (m_size == 256)
        WD_CHECK (file.write((WT_Byte) 0));
    else
        WD_CHECK (file.write((WT_Byte) m_size));

    for (int loop = 0; loop < m_size; loop++)
        WD_CHECK (file.write(m_map[loop]));

    return WT_Result::Success;
}
#else
WT_Result WT_Color_Map::serialize_just_colors(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Color_Map::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Color_Map_ID &&
        m_size == ((WT_Color_Map const &)attrib).m_size)
    {
        if (m_incarnation == ((WT_Color_Map const &)attrib).m_incarnation)
        {
            return WD_True;
        }
        for (int loop = 0; loop < m_size; loop++)
        {
            // TODO: we don't want to be comparing the entire map each time we output an index
            // so we need to use incarnation comparisons rather than piecewise compares.
            if (m_map[loop].m_whole != ((WT_Color_Map const &)attrib).m_map[loop].m_whole)
            {
                return WD_False;
            }
        }
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Color_Map::sync(WT_File & file) const
{
    WD_Assert(  (file.file_mode() == WT_File::File_Write) ||
                (file.file_mode() == WT_File::Block_Append) ||
                (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().color_map())
    {
        // TODO: this copy is incorrect due to the pointer to an allocated color map.
        file.rendition().color_map().set(m_size,m_map,file);
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Color_Map::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Boolean  using_binary;

    m_incarnation = file.next_incarnation();

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        using_binary = WD_False;

        break;
    case WT_Opcode::Extended_Binary:
        using_binary = WD_True;

        break;
    case WT_Opcode::Single_Byte:
    default:
            // Error, this opcode is not legal for this object.
            return WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch

    WD_CHECK(materialize_just_colors (opcode, using_binary, WD_True, file));

    // In DWF versions before 00.25, the background color index was incorrectly written
    // and is bogus.  Also, the background opcode was sent *before* the colormap opcode.
    // To correct for this we will now call the background color process routine with
    // an index of 0, which in most cases is the correct background color for Acad
    // generated dwfs.
    if (file.rendition().drawing_info().decimal_revision() < WHIP10_DWF_FILE_VERSION)
    {
        WT_Background    patch_background(WT_Color(0, *this));

        WD_CHECK(patch_background.process(file));
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::materialize_just_colors(WT_Opcode const & opcode, WT_Boolean using_binary, WT_Boolean get_close_brace, WT_File & file)
{
    // Be carefull not to confuse the "int" m_size with the "byte" count.
    // If count is zero, it means that m_size is 256.
    // If m_size is zero, it means we haven't read the value from the file yet.
    switch (m_stage)
    {
    case Getting_Count:

        if (using_binary)
        {
          WT_Byte    count;

            WD_CHECK (file.read(count));
            // The value 256 doesn't fit into a byte, so 0 represents 256.
            if (count == 0)
                m_size = 256;
            else
                m_size = count;
        }
        else
        {
            WD_CHECK (file.read_ascii(m_size));
            if (m_size < 0 || m_size > 65535)
                return WT_Result::Corrupt_File_Error;
        }

        m_incarnation = file.next_incarnation();

        WD_Assert(m_map == WD_Null);

        m_map = new WT_RGBA32[m_size];
        if (!m_map)
            return WT_Result::Out_Of_Memory_Error;

        m_stage = Getting_Colors;

        // No break here

    case Getting_Colors:

        if (using_binary)
            WD_CHECK(file.read(m_size, m_map));
        else
            WD_CHECK(file.read_ascii(m_size, m_map));

        if (get_close_brace)
            m_stage = Getting_Close_Brace;
        else
        {
            m_stage = Getting_Count;  // We finished, so reset for next time.
            break;
        }
        // No break here

    case Getting_Close_Brace:

        if (using_binary)
        {
            WT_Byte a_brace;

            WD_CHECK (file.read(a_brace));
            if (a_brace != '}')
                return WT_Result::Corrupt_File_Error;
        }
        else
        {
            WD_CHECK (opcode.skip_past_matching_paren(file));
        }

        m_stage = Getting_Count;  // We finished, so reset for next time.
        break;

    default:
        return WT_Result::Internal_Error;
    }  // switch (m_stage)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            WD_CHECK(opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
        {
            WT_Integer32    size;
            WT_Byte    *        size_ptr = (WT_Byte *)&size;

            size_ptr[0] = (opcode.token())[1];
            size_ptr[1] = (opcode.token())[2];
            size_ptr[2] = (opcode.token())[3];
            size_ptr[3] = (opcode.token())[4];

            file.skip(size - WD_EXTENDED_BINARY_OPCODE_SIZE);
        } break;
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

/// Returns the index of the matching color in the map to the given color, or -1 if not found.
int WT_Color_Map::exact_index(WT_RGBA32 const & rgba) const
{
    for (int index = 0; index < size(); index++)
    {
        long red_diff = rgba.m_rgb.r - map((WT_Byte)index).m_rgb.r;
        long grn_diff = rgba.m_rgb.g - map((WT_Byte)index).m_rgb.g;
        long blu_diff = rgba.m_rgb.b - map((WT_Byte)index).m_rgb.b;
        long alp_diff = rgba.m_rgb.a - map((WT_Byte)index).m_rgb.a;
        long error    = red_diff * red_diff +
                        grn_diff * grn_diff +
                        blu_diff * blu_diff +
                        alp_diff * alp_diff;

        if (error == 0)
        {
            return index;
        }
    } // for

    return WD_NO_COLOR_INDEX;
}

/// Returns the index of the matching color in the map to the given color, or -1 if not found.
int WT_Color_Map::exact_index(WT_Color const & desired) const
{
    // First check to see if what we were given is the actual answer.
    if ( desired.index() != WD_NO_COLOR_INDEX    &&
         desired.index() <  size()               &&
         desired.rgba()  == map((WT_Byte)desired.index()) )
        return desired.index();

    for (int index = 0; index < size(); index++)
    {
        long red_diff = desired.rgba().m_rgb.r - map((WT_Byte)index).m_rgb.r;
        long grn_diff = desired.rgba().m_rgb.g - map((WT_Byte)index).m_rgb.g;
        long blu_diff = desired.rgba().m_rgb.b - map((WT_Byte)index).m_rgb.b;
        long alp_diff = desired.rgba().m_rgb.a - map((WT_Byte)index).m_rgb.a;
        long error    = red_diff * red_diff +
                        grn_diff * grn_diff +
                        blu_diff * blu_diff +
                        alp_diff * alp_diff;

        if (error == 0)
        {
            return index;
        }
    } // for

    return WD_NO_COLOR_INDEX;
}

///////////////////////////////////////////////////////////////////////////
int WT_Color_Map::closest_index(WT_Color const & desired) const
{
    // If we have no colors, then we can't match
    if (size() < 1)
        return WD_NO_COLOR_INDEX;

    int idx = exact_index( desired );
    if (idx != WD_NO_COLOR_INDEX)
    {
        return idx;
    }

    // Now search the entire list for the best match
    int   best_match = 0;
    long  best_match_error =  524288; // Twice the maximum possible error;

    for (int index = 0; index < size(); index++)
    {
        long red_diff = desired.rgba().m_rgb.r - map((WT_Byte)index).m_rgb.r;
        long grn_diff = desired.rgba().m_rgb.g - map((WT_Byte)index).m_rgb.g;
        long blu_diff = desired.rgba().m_rgb.b - map((WT_Byte)index).m_rgb.b;
        long alp_diff = desired.rgba().m_rgb.a - map((WT_Byte)index).m_rgb.a;
        long error    = red_diff * red_diff +
                        grn_diff * grn_diff +
                        blu_diff * blu_diff +
                        alp_diff * alp_diff;
        if (error < best_match_error)
        {
            best_match_error = error;
            best_match = index;
        }
    } // for

    return best_match;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::process(WT_File & file)
{
    WD_Assert(file.color_map_action());
    return (file.color_map_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color_Map::default_process(WT_Color_Map & item, WT_File & file)
{
    file.rendition().color_map() = item;
    return WT_Result::Success;
}
