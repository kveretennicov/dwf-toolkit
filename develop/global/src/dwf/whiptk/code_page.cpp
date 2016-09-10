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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/code_page.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Code_Page::object_id() const
{
    return Code_Page_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Code_Page::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(CodePage "));
    WD_CHECK (file.write_ascii(m_page_number));
    WD_CHECK (file.write(")"));

    return WT_Result::Success;
}
#else
WT_Result WT_Code_Page::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Code_Page::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Code_Page_ID)
        if (number() == ((WT_Code_Page const &)attrib).number())
            return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Code_Page::sync(WT_File & file) const
{

    WD_Assert(  (file.file_mode() == WT_File::File_Write) ||
                (file.file_mode() == WT_File::Block_Append) ||
                (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().code_page())
    {
        file.rendition().code_page() = *this;
        return serialize(file);
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Code_Page::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Code_Page::materialize(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Getting_Page;
        // No Break Here

    case Getting_Page:
        WD_CHECK (file.read_ascii(m_page_number));
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK (opcode.skip_past_matching_paren(file));
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Code_Page::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Code_Page::process(WT_File & file)
{
    WD_Assert (file.code_page_action());
    return (file.code_page_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Code_Page::default_process(WT_Code_Page & item, WT_File & file)
{
    file.rendition().code_page() = item;
    return WT_Result::Success;
}
