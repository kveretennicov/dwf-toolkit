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
// $Header: /Components/Internal/DWF Toolkit/v7/develop/global/src/dwf/whiptk/backgrnd.cpp 2     12/02/04 1:42p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Background::object_id() const
{
    return Background_ID;
}

///////////////////////////////////////////////////////////////////////////
void WT_Background::set(WT_Color const & color)
{
    m_color = color;
}

///////////////////////////////////////////////////////////////////////////
const WT_Color& WT_Background::color() const
{
    return m_color;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Background::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Success; //silently ignore this one for DWF packages

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (file.heuristics().allow_indexed_colors() && m_color.index() != WD_NO_COLOR_INDEX)
    {
        // We can use a color index.

        WD_Assert(file.desired_rendition().color_map().size() > m_color.index());

        WD_CHECK (file.desired_rendition().sync(file, WT_Rendition::Color_Map_Bit));

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(Background "));
        WD_CHECK (file.write_ascii((WT_Integer32) m_color.index()));
    }
    else
    {
        // We have to use a full RGBA color.
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(Background ")               );
        WD_CHECK (file.write_ascii((WT_Integer32) m_color.rgba().m_rgb.r ));
        WD_CHECK (file.write((WT_Byte)',')                 );
        WD_CHECK (file.write_ascii((WT_Integer32) m_color.rgba().m_rgb.g ));
        WD_CHECK (file.write((WT_Byte)',')                 );
        WD_CHECK (file.write_ascii((WT_Integer32) m_color.rgba().m_rgb.b ));
        WD_CHECK (file.write((WT_Byte)',')                 );
        WD_CHECK (file.write_ascii((WT_Integer32) m_color.rgba().m_rgb.a ));
    }

    return file.write(")");
}
#else
WT_Result WT_Background::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Background::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Background_ID &&
        m_color == ((WT_Background const &)attrib).m_color)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Background::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if (*this != file.rendition().rendering_options().background() )
    {
      file.rendition().rendering_options().background() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Background::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Background::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Getting_Color:

                WD_CHECK (file.read_ascii(m_color));
                m_stage = Getting_Close_Paren;
                // No break

            case Getting_Close_Paren:

              WD_CHECK (opcode.skip_past_matching_paren(file));
                m_stage = Getting_Color;
                break;

            default:
                return WT_Result::Internal_Error;
            }
        } break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Background::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        WD_CHECK(opcode.skip_past_matching_paren(file));
        break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch


    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Background::process(WT_File & file)
{
    WD_Assert(file.background_action());
    return (file.background_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Background::default_process(WT_Background & item, WT_File & file)
{
    file.rendition().rendering_options().background() = item;
    return WT_Result::Success;
}
