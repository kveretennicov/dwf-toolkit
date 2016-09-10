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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/penpat_options.cpp 3     5/09/05 12:42a Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_PenPat_Options::object_id() const
{
    return WT_Object::PenPat_Options_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_PenPat_Options::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // ASCII output
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(PenPat_Options "));

    if(get_scale_pen_width())
        WD_CHECK (file.write((WT_Byte) '1'));
    else
        WD_CHECK (file.write((WT_Byte) '0'));
    WD_CHECK (file.write(" "));

    if(get_map_colors_to_gray_scale())
        WD_CHECK (file.write((WT_Byte) '1'));
    else
        WD_CHECK (file.write((WT_Byte) '0'));
    WD_CHECK (file.write(" "));

    if(get_use_alternate_fill_rule())
        WD_CHECK (file.write((WT_Byte) '1'));
    else
        WD_CHECK (file.write((WT_Byte) '0'));
    WD_CHECK (file.write(" "));

    if(get_use_error_diffusion_for_DWF_Rasters())
        WD_CHECK (file.write((WT_Byte) '1'));
    else
        WD_CHECK (file.write((WT_Byte) '0'));
    WD_CHECK (file.write(" "));

    WD_CHECK (file.write((WT_Byte) ')'));

    return WT_Result::Success;
}
#else
WT_Result WT_PenPat_Options::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PenPat_Options::materialize(WT_Opcode const & opcode,
                                     WT_File & file)
{
    if (opcode.type() == WT_Opcode::Extended_ASCII)
    {
        // ASCII
        WT_Integer32 iTmp = 0;

        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.eat_whitespace());
            m_stage = Getting_Scale_Pen_Width;

        case Getting_Scale_Pen_Width:
            iTmp = 0;
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(iTmp));
            if( iTmp != 0 )
                m_scale_pen_width = WD_True;
            else
                m_scale_pen_width = WD_False;

            m_stage = Getting_Map_Colors_To_Gray_Scale;

            // No break
        case Getting_Map_Colors_To_Gray_Scale:
            iTmp = 0;
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(iTmp));
            if( iTmp != 0 )
                m_map_colors_to_gray_scale = WD_True;
            else
                m_map_colors_to_gray_scale = WD_False;

            m_stage = Getting_Use_Alternate_Fill_Rule;

            // No break
        case Getting_Use_Alternate_Fill_Rule:
            iTmp = 0;
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(iTmp));
            if( iTmp != 0 )
                m_use_alternate_fill_rule = WD_True;
            else
                m_use_alternate_fill_rule = WD_False;

            m_stage = Getting_Use_Error_Diffusion_For_DWF_Rasters;

            // No break
        case Getting_Use_Error_Diffusion_For_DWF_Rasters:
            iTmp = 0;
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(iTmp));
            if( iTmp != 0 )
                m_use_error_diffusion_for_DWF_Rasters = WD_True;
            else
                m_use_error_diffusion_for_DWF_Rasters = WD_False;

            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(opcode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
        } // switch (m_stage)
    } // ASCII
    else {
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_PenPat_Options const & WT_PenPat_Options::operator=(WT_PenPat_Options const & options)
{

    m_scale_pen_width = options.get_scale_pen_width();;
    m_map_colors_to_gray_scale = options.get_map_colors_to_gray_scale();
    m_use_alternate_fill_rule = options.get_use_alternate_fill_rule();
    m_use_error_diffusion_for_DWF_Rasters = options.get_use_error_diffusion_for_DWF_Rasters();

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PenPat_Options::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PenPat_Options::process(WT_File & file)
{
    WD_Assert(file.penpat_options_action());
    return (file.penpat_options_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_PenPat_Options::default_process(WT_PenPat_Options & item, WT_File & file)
{
    file.rendition().rendering_options().penpat_options() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_PenPat_Options::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().rendering_options().penpat_options() )
    {
        file.rendition().rendering_options().penpat_options() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_PenPat_Options::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_PenPat_Options::operator== (WT_Attribute const & attrib) const
{
    if(attrib.object_id() != PenPat_Options_ID)
        return WD_False;

    const WT_PenPat_Options & options = (const WT_PenPat_Options &)attrib;

    if(m_scale_pen_width != options.get_scale_pen_width()
        || m_map_colors_to_gray_scale != options.get_map_colors_to_gray_scale()
        || m_use_alternate_fill_rule != options.get_use_alternate_fill_rule()
        || m_use_error_diffusion_for_DWF_Rasters != options.get_use_error_diffusion_for_DWF_Rasters())
        return WD_False;

    return WD_True;
}
