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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/font_extension.cpp 1     9/12/04 8:53p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Extension::object_id() const
{
    return Font_Extension_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Extension::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(FontExtension "));

    WD_CHECK(m_logfont_name.serialize(file,WD_True));
    WD_CHECK (file.write(" "));
    WD_CHECK(m_cannonical_name.serialize(file,WD_True));

    WD_CHECK (file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Font_Extension::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Font_Extension::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() != Font_Extension_ID)
        return WD_False;

    if ((m_logfont_name == ((WT_Font_Extension const &)attrib).m_logfont_name) &&
        (m_cannonical_name == ((WT_Font_Extension const &)attrib).m_cannonical_name))
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Extension::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().font_extension() )
    {
        file.rendition().font_extension() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Font_Extension::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Font_Extension::materialize(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch (m_stage)
    {
    case Eating_End_Whitespace:
    {
        break;
    }
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_LogFont_Name;
        // No Break here

    case Gathering_LogFont_Name:
        WD_CHECK(m_logfont_name.materialize(file));

        m_stage = Eating_Middle_Whitespace;
        // No Break here

    case Eating_Middle_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_Cannonical_Name;
        // No Break here

    case Gathering_Cannonical_Name:
        WD_CHECK (m_cannonical_name.materialize(file));
        m_stage = Eating_End_Whitespace;
        // No Break here
    }
    WD_CHECK ( opcode.skip_past_matching_paren(file) );

    m_materialized = WD_True;
    m_stage = Eating_Initial_Whitespace;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font_Extension::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font_Extension::process(WT_File & file)
{
    WD_Assert(file.font_extension_action());
    return (file.font_extension_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font_Extension::default_process(WT_Font_Extension & item, WT_File & file)
{
    file.rendition().font_extension() = item;
    return WT_Result::Success;
}
