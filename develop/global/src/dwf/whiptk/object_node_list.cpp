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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/object_node_list.cpp 1     9/12/04 8:55p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Object_Node_List::object_id() const
{
    return WT_Object::Object_Node_List_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Object_Node_List::object_type() const
{
    return WT_Object::Definition;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result    WT_Object_Node_List::serialize (WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // Return internal error.  This method should never be called
    //  as there is no object_node_list opcode
    return WT_Result::Internal_Error;
}
#else
WT_Result    WT_Object_Node_List::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Object_Node_List::materialize (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no object_node_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Object_Node_List::skip_operand (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no object_node_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Object_Node_List::process(WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no object_node_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void  WT_Object_Node_List::add_object_node (WT_Object_Node const & object_node)
{
    // Make a copy of object_node

    WT_Object_Node *     object_node_copy = new WT_Object_Node (object_node);

    add_front(object_node_copy );
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node * WT_Object_Node_List::find_object_node_from_index  (WT_Integer32 object_node_num)
{
    if (is_empty())
        return WD_Null;

    // find associated view
    WT_Object_Node *         current_object_node = (WT_Object_Node *) get_head();

    while (current_object_node != WD_Null)
    {
        // NOTE: If more than one named view has the same name, we return the first instance
        if (current_object_node->object_node_num()== object_node_num)
            return current_object_node;
        current_object_node = (WT_Object_Node *) current_object_node->next();
    }
    // Didn't find object_node in the list
    return WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node * WT_Object_Node_List::find_object_node_from_name  (WT_String object_node_name)
{
    if (is_empty() || object_node_name.length() == 0)
        return WD_Null;

    // find associated view
    WT_Object_Node *         current_object_node = (WT_Object_Node *) get_head();

    while (current_object_node != WD_Null)
    {
        // NOTE: If more than one named view has the same name, we return the first instance
        if (current_object_node->object_node_name()== object_node_name)
            return current_object_node;
        current_object_node = (WT_Object_Node *) current_object_node->next();
    }
    // Didn't find object_node in the list
    return WD_Null;
}
