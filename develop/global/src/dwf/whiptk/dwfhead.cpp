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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/dwfhead.cpp 1     9/12/04 8:52p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_DWF_Header::object_id() const
{
    return DWF_Header_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_DWF_Header::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_DWF_Header::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    char    header[16];

    if (file.heuristics().target_version() < WHIP40_DWF_FILE_VERSION)
    {
        // This section avoids hitting a bug in older WHIP 3.1 viewers.
        // If a new application writes a DWF file in the older 3.1 format
        // and if the drawing uses true-type text without first defining a
        // font (as would happen if the application were storing invisible
        // true-type text alongside visible stroked text vectors), then
        // the old WHIP 3.1 viewer will crash.
        // The workaround we use here is to force a font to be sync'ed if
        // true-type is emmitted.  We do this by making sure that the
        // desired rendition wont match the current rendition.
        file.rendition().font() = WT_Font(  WT_String("undefined_font"), // name,
                                            WD_False,                    // bold,
                                            WD_False,                    // italic,
                                            WD_False,                    // underlined,
                                            0,                           // charset,
                                            0,                           // pitch,
                                            0,                           // family,
                                            1024,                        // height,
                                            0,                           // rotation,
                                            1024,                        // width_scale,
                                            1024,                        // spacing,
                                            0,                           // oblique,
                                            0);                          // flags);

        file.desired_rendition().font() = WT_Font(  WT_String("courier"), // name,
                                                    WD_False,             // bold,
                                                    WD_False,             // italic,
                                                    WD_False,             // underlined,
                                                    0,                    // charset,
                                                    0,                    // pitch,
                                                    0,                    // family,
                                                    1024,                 // height,
                                                    0,                    // rotation,
                                                    1024,                 // width_scale,
                                                    1024,                 // spacing,
                                                    0,                    // oblique,
                                                    0);                   // flags);
    }

    if (file.heuristics().target_version() < REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        WD_CHECK (file.write("(DWF V"));
    else
        WD_CHECK (file.write("(W2D V"));

    header[0] = (char) ((file.heuristics().target_version() / 1000)     ) + '0';
    header[1] = (char) ((file.heuristics().target_version() / 100 ) % 10) + '0';
    header[2] = (char) '.';
    header[3] = (char) ((file.heuristics().target_version() / 10  ) % 10) + '0';
    header[4] = (char) ((file.heuristics().target_version()       ) % 10) + '0';
    header[5] = ')';
    header[6] = '\0';

    return file.write(header);
}
#else
WT_Result WT_DWF_Header::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_DWF_Header::materialize(WT_Opcode const &, WT_File & file)
{
    WT_Byte        tmp_buf[8];

    WD_CHECK (file.read(6, tmp_buf));

    tmp_buf[6] = '\0';
    if (tmp_buf[0] < '0' || tmp_buf[0] > '9' ||
        tmp_buf[1] < '0' || tmp_buf[1] > '9' ||
        tmp_buf[2] != '.' ||
        tmp_buf[3] < '0' || tmp_buf[3] > '9' ||
        tmp_buf[4] < '0' || tmp_buf[4] > '9' ||
        tmp_buf[5] != ')'                        )
        return WT_Result::Not_A_DWF_File_Error;

    file.rendition().drawing_info().set_major_revision((tmp_buf[0] - '0') * 10 + (tmp_buf[1] - '0'));
    file.rendition().drawing_info().set_minor_revision((tmp_buf[3] - '0') * 10 + (tmp_buf[4] - '0'));

    file.decrement_paren_count();


    // The default colormap changed with the REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED.
    // If we see we are reading in an older file, then the colormap in the renditions
    // were initialized incorrectly and we need to reset them to the old style colormap.
    if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_DEFAULT_COLORMAP_WAS_CHANGED)
    {
        file.rendition().color_map() = WT_Color_Map(file.rendition().drawing_info().decimal_revision());
    }

    // This stream toolkit cannot open DWF 6.0 package files.  From 6.0 onwards, we open W2D streams only
    if (!file.heuristics().m_w2d_channel && file.rendition().drawing_info().decimal_revision() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
    {
        return WT_Result::DWF_Package_Format;
    }

    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_DWF_Header::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_DWF_Header::process(WT_File & file)
{
    WD_Assert(file.dwf_header_action());
    return (file.dwf_header_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_DWF_Header::default_process(WT_DWF_Header &, WT_File &)
{
    return WT_Result::Success;
}
