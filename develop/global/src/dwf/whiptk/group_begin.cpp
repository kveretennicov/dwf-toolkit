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
WT_Object::WT_ID WT_Group_Begin::object_id() const
{
    return Group_Begin_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Group_Begin::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Group_Begin::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // We've never seen this group before.
    // So define it and add it to the list of defined groups.
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(GroupBegin "));
    WD_CHECK (m_group_path.serialize(file));
    WD_CHECK (file.write((WT_Byte) ')'));

    return WT_Result::Success;
}
#else
WT_Result WT_Group_Begin::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_Begin::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
        case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
                case Gathering_Group_Path:
                    WD_CHECK (m_group_path.materialize(file));

                    // No Break Here
                case Eating_End_Whitespace:
                    WD_CHECK (opcode.skip_past_matching_paren(file));
                    // We finished, so get ready to start again...
                    m_stage = Gathering_Group_Path;
                    //m_stage = Gathering_Group_Id;
                    break;

                default:
                    return WT_Result::Internal_Error;
            }
        }
        break; //end of ascii

        default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        }
        break;
    }

    m_materialized = WD_True;

    //Set the current object_node rendition using this object to create a WT_Object_Node,
    //since viewers for version 6.0 and beyond won't be responding to Group tags anymore

    // Have we seen this group before?
    WT_Object_Node* node = file.object_node_list().find_object_node_from_name(m_group_path);
    if (!node)
    {
        WT_Integer32 object_node_num = 0;
        WT_Object_Node * current_object_node = (WT_Object_Node *) file.object_node_list().get_head();
        while (current_object_node != WD_Null)
        {
            // NOTE: If more than one named view has the same name, we return the first instance
            if (current_object_node->object_node_num() > object_node_num)
                object_node_num = current_object_node->object_node_num()+1;
            current_object_node = (WT_Object_Node *) current_object_node->next();
        }
        WT_Object_Node new_node(file, object_node_num, m_group_path);

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
WT_Result WT_Group_Begin::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() == WT_Opcode::Extended_ASCII)
        return opcode.skip_past_matching_paren(file);
    else
        return WT_Result::Opcode_Not_Valid_For_This_Object;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_Begin::process(WT_File & file)
{
    WD_Assert (file.group_begin_action());
    return (file.group_begin_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Group_Begin::default_process(WT_Group_Begin &, WT_File &)
{
    return WT_Result::Success;
}
