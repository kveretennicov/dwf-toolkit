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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/pattern_scale.cpp 3     7/06/05 4:24p Bangiav $
#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Pattern_Scale::serialize(
    WT_Object const & parent,
    WT_File & file ) const
{

    WD_CHECK( file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK( file.write_tab_level());
    if (parent.object_id() == WT_Object::Fill_Pattern_ID ||
        parent.object_id() == WT_Object::User_Fill_Pattern_ID)
        WD_CHECK( file.write("(FillPatternScale "));
    else
        WD_CHECK( file.write("(LinePatternScale "));
    WD_CHECK( file.write_ascii(m_value));
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Pattern_Scale::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Pattern_Scale::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read_ascii(m_value) );
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}
