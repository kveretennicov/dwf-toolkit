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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/endofdwf.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_End_Of_DWF::object_id() const
{
    return End_Of_DWF_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_End_Of_DWF::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_End_Of_DWF::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(EndOfDWF)"));
    return file.write(WD_NEWLINE);
}
#else
WT_Result WT_End_Of_DWF::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_End_Of_DWF::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WD_CHECK (skip_operand(opcode, file));

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_End_Of_DWF::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_End_Of_DWF::process(WT_File & file)
{
    WD_Assert(file.end_of_dwf_action());
    return (file.end_of_dwf_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_End_Of_DWF::default_process(WT_End_Of_DWF &, WT_File &)
{
    return WT_Result::End_Of_DWF_Opcode_Found;
}
