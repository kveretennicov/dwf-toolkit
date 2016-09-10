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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/origin.cpp 1     9/12/04 8:55p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Origin::object_id() const
{
    return WT_Object::Origin_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Origin::object_type() const
{
    return WT_Object::Drawable;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Origin::serialize(WT_File & file, WT_Boolean update_current_point) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_Assert(file.heuristics().allow_binary_data());

    WD_CHECK (file.write((WT_Byte) 'O'));
    WD_CHECK (file.write(1, &m_origin));

    if (update_current_point)
        file.update_current_point (m_origin);

    return WT_Result::Success;
}
#else
WT_Result WT_Origin::serialize(WT_File &, WT_Boolean) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Origin::materialize(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() != WT_Opcode::Single_Byte)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    WD_CHECK (file.read (1, &m_origin));

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Origin::skip_operand(WT_Opcode const &, WT_File & file)
{
    WD_CHECK (file.skip (sizeof (WT_Logical_Point)));

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Origin::process(WT_File & file)
{
    WD_Assert (file.origin_action());
    return (file.origin_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Origin::default_process(WT_Origin & item, WT_File & file)
{
    file.update_current_point (item.origin());

    return WT_Result::Success;
}
