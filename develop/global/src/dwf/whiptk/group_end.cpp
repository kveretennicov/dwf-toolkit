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
// $Header:
#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Group_End::object_id() const
{
    return Group_End_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Group_End::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Group_End::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // We've never seen this layer before.
    // So define it and add it to the list of defined layers.
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(GroupEnd)"));

    return WT_Result::Success;
}
#else
WT_Result WT_Group_End::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_End::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
        case WT_Opcode::Extended_ASCII:
        {
            WD_CHECK (opcode.skip_past_matching_paren(file));
        }
        break; //end of ascii

        default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        }
        break;
    }

    m_materialized = WD_True;

    //Set the current object_node rendition to node 'zero'

    //Have we seen node zero before?

    WT_Object_Node* node = file.object_node_list().find_object_node_from_index(0);
    if (!node)
    {
        WT_Object_Node new_node(file, 0, "");

        //we haven't, so create a unique number for it
        file.object_node_list().add_object_node(new_node);
        file.rendition().object_node().set(new_node);
    }
    else
    {
        file.rendition().object_node().set(*node);
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_End::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() == WT_Opcode::Extended_ASCII)
        return opcode.skip_past_matching_paren(file);
    else
        return WT_Result::Opcode_Not_Valid_For_This_Object;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_End::process(WT_File & file)
{
    WD_Assert (file.group_end_action());
    return (file.group_end_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_End::default_process(WT_Group_End &, WT_File &)
{
    return WT_Result::Success;
}
