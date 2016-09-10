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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/color.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
void WT_Color::set(
    WT_Color_Index index,
    const WT_Color_Map& color_map )
{
    m_index = index;
    if( color_map.size() > index )
        m_rgba = color_map.map()[index];
    else
        throw WT_Result::Toolkit_Usage_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID  WT_Color::object_id() const
{
    return Color_ID;
}

int WT_Color::map_to_index(
    WT_RGBA32 rgba,
    WT_Color_Map_Mode mode,
    WT_Color_Map const & cmap )
{
    // calc heidi color
    // cube index
    long lr,lg,lb;
    short sr,sg,sb;
    lr = ( rgba.m_rgb.r * 6 ) << 8;
    lg = ( rgba.m_rgb.g * 6 ) << 8;
    lb = ( rgba.m_rgb.b * 6 ) << 8;
    lr = WT_Endian::adjust(lr);
    lg = WT_Endian::adjust(lg);
    lb = WT_Endian::adjust(lb);
    sr = (short)(( lr >> 16 ) & 0x0000FFFF);
    sg = (short)(( lg >> 16 ) & 0x0000FFFF);
    sb = (short)(( lb >> 16 ) & 0x0000FFFF);
    int index = sb + (sg*6) + (sr*6*6);
    WT_RGBA32* pMap = cmap.map();
    switch( mode )
    {
    case Search_666_Color_Cube_Exact:
        if( index < cmap.size()
            && pMap[index] == rgba )
            return index;
        break;
    case Search_666_Color_Cube_Nearest:
        return index;
    case Search_666_Color_Cube_Exact_Then_Color_Map_Exact:
        if( index < cmap.size()
            && pMap[index] == rgba )
            return index;
        index = cmap.closest_index(rgba);
        if( index < cmap.size()
            && pMap[index] == rgba )
            return index;
        break;
    case Search_666_Color_Cube_Exact_Then_Color_Map_Nearest:
        if( index < cmap.size()
            && pMap[index] == rgba )
            return index;
        return cmap.closest_index(rgba);
    default:;// no mapping
    }
    return WD_NO_COLOR_INDEX;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Color::serialize( WT_File& file ) const
{
    WD_CHECK( file.dump_delayed_drawable() );

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WT_Color_Index index = m_index;
    if( m_mode != No_Mapping
        && index == WD_NO_COLOR_INDEX
        && file.heuristics().allow_indexed_colors()
        && file.desired_rendition().color_map().size()>index )
    {   // map this rgba
        // to an index
        WT_Color_Map m =
            file.desired_rendition().color_map();
        index = map_to_index(m_rgba,m_mode,m);
    }

    if( index != WD_NO_COLOR_INDEX )
    {   // serialize color as index
        WD_CHECK(
            file.desired_rendition().sync(
                file,
                WT_Rendition::Color_Map_Bit) );
        if( file.heuristics().allow_binary_data() )
        {   WD_CHECK( file.write((WT_Byte) 'c') );
            return file.write((WT_Byte)index);
        }
        else
        {   WD_CHECK( file.write_tab_level() );
            WD_CHECK( file.write("C ") );
            return file.write_ascii(index);
        }
    }
    else
        // serialize color as a full RGBA
        if( file.heuristics().allow_binary_data() )
        {   WD_CHECK( file.write((WT_Byte)0x03) );
            return file.write(m_rgba);
        }
        else
        {   WD_CHECK( file.write_tab_level() );
            WD_CHECK( file.write("(Color ") );
            WD_CHECK( file.write_ascii(m_rgba) );
            return file.write(")");
        }
}
#else
WT_Result WT_Color::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Color::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Color_ID &&
        m_rgba.m_whole == ((WT_Color const &)attrib).m_rgba.m_whole &&
        m_index == ((WT_Color const &)attrib).m_index)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Color::sync(WT_File & file) const
{
    WD_Assert(  (file.file_mode() == WT_File::File_Write) ||
                (file.file_mode() == WT_File::Block_Append) ||
                (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().color())
    {
        file.rendition().color() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Color::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        switch (opcode.token()[0])
        {
        case 'C':
            // Indexed color, in ASCII

            WD_CHECK (file.read_ascii(m_index));
            if (m_index < 0)
                return WT_Result::Corrupt_File_Error;

            // Note: We don't know which colormap this indexed color is
            // going to refer to.  Normally it will refer to the color map
            // in the File's rendition, but how do we know what the user intends?
            // So, if the index is in range of the File's map, use that RGBA,
            // otherwise just set the rgba to a clear-black (undefined) color
            // that the user's process routine should fix.
            if (m_index < file.rendition().color_map().size())
              m_rgba  = file.rendition().color_map().map()[m_index];
            else
              m_rgba  = WT_RGBA32(0,0,0,0); // Clear black-- an undefined color!

            break;
        case 'c':
            // Indexed color, in binary

            WT_Byte index;

            WD_CHECK (file.read(index));
            m_index = index;

            // Note: We don't know which colormap this indexed color is
            // going to refer to.  Normally it will refer to the color map
            // in the File's rendition, but how do we know what the user intends?
            // So, if the index is in range of the File's map, use that RGBA,
            // otherwise just set the rgba to a clear-black (undefined) color
            // that the user's process routine should fix.
            if (m_index < file.rendition().color_map().size())
              m_rgba  = file.rendition().color_map().map()[m_index];
            else
              m_rgba  = WT_RGBA32(0,0,0,0); // Clear black-- an undefined color!

            break;
        case 0x03: // Ctrl-C
            // RGBA color, in binary
            WD_CHECK (file.read(m_rgba));
            m_index = WD_NO_COLOR_INDEX;

            break;
        default:

            // Error, this opcode is not legal for this object.
            return WT_Result::Opcode_Not_Valid_For_This_Object;
            // break;
        } // switch (opcode.token()[0])

        break;
    case WT_Opcode::Extended_ASCII:
        // RGBA color, extended ASCII
        switch (m_stage)
        {
        case Getting_Color:
            WD_CHECK(file.read_ascii(*this));
            m_index = WD_NO_COLOR_INDEX;
            m_stage = Getting_Close_Paren;

        // No break here
        case Getting_Close_Paren:
            WD_CHECK(opcode.skip_past_matching_paren(file));
            m_stage = Getting_Color; // we finished, so reset for next time.
        } // switch (m_stage)

        break;
     default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch


    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 'C':
                {
                // Indexed color, in ASCII
                WT_Integer32 index;
                WD_CHECK (file.read_ascii(index));
                } break;

            case 'c':
                {
                // Indexed color, in binary
                WT_Byte index;
                WD_CHECK (file.read(index));
                } break;

            case 0x03: // Ctrl-C
                {
                // RGBA color, in binary
                WT_RGBA32 rgba;
                WD_CHECK (file.read(rgba));
                } break;

            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            }
        } break;
    case WT_Opcode::Extended_ASCII:
        {
            WD_CHECK (opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color::process(WT_File & file)
{
    WD_Assert(file.color_action());
    return (file.color_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Color::default_process(WT_Color & item, WT_File & file)
{
    file.rendition().color() = item;
    return WT_Result::Success;
}
