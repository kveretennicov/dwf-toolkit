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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/layer_list.cpp 1     9/12/04 8:54p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Layer_List::object_id() const
{
    return WT_Object::Layer_List_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Layer_List::object_type() const
{
    return WT_Object::Definition;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result    WT_Layer_List::serialize (WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // Return internal error.  This method should never be called
    //  as there is no layer_list opcode
    return WT_Result::Internal_Error;
}
#else
WT_Result    WT_Layer_List::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Layer_List::materialize (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no layer_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Layer_List::skip_operand (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no layer_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Layer_List::process(WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no layer_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void  WT_Layer_List::add_layer (WT_Layer const & layer)
{
    // Make a copy of layer

    WT_Layer *     layer_copy = new WT_Layer (layer);
    add_front(layer_copy);
}

///////////////////////////////////////////////////////////////////////////
WT_Layer * WT_Layer_List::find_layer_from_index  (WT_Integer32 layer_num)
{
    if (is_empty())
        return WD_Null;

    // find associated view
    WT_Layer *         current_layer = (WT_Layer *) get_head();

    while (current_layer != WD_Null)
    {
        // NOTE: If more than one named view has the same name, we return the first instance
        if (current_layer->layer_num()== layer_num)
            return current_layer;
        current_layer = (WT_Layer *) current_layer->next();
    }
    // Didn't find layer in the list
    return WD_Null;
}
