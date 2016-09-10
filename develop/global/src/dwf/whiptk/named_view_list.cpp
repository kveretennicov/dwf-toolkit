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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/named_view_list.cpp 1     9/12/04 8:55p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Named_View_List::object_id() const
{
    return WT_Object::Named_View_List_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View_List::~WT_Named_View_List()
{
    remove_all();
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Named_View_List::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View_List const &
    WT_Named_View_List::operator=(WT_Named_View_List const & list)
throw(WT_Result)
{
    remove_all();

    if (list.is_empty())
        return *this;

    WT_Named_View *current =
        (WT_Named_View *) list.get_head();

    while (current != WD_Null)
    {
        WT_Named_View *new_item;
        new_item = current->copy();
        if (new_item == WD_Null)
            throw (WT_Result::Out_Of_Memory_Error );

        add_front(new_item);
        current = (WT_Named_View *) current->next();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result    WT_Named_View_List::serialize (WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // Have each named view in the list serialize itself.
    if (!is_empty())
    {
        // One or more in list
        WT_Named_View *     current_named_view = (WT_Named_View *) get_head();

        do
        {
            WD_CHECK (current_named_view->serialize (file));
            current_named_view = (WT_Named_View *) current_named_view->next();
        } while (current_named_view->next() != WD_Null);

    }
    return WT_Result::Success;
}
#else
WT_Result    WT_Named_View_List::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Named_View_List::materialize (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no named_view_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Named_View_List::skip_operand (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no named_view_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_Named_View_List::process(WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no named_view_list opcode
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void  WT_Named_View_List::add_named_view (WT_Named_View & named_view)
{
    // Make a copy of the named view
    WT_Named_View *     named_view_copy = named_view.copy();

    add_tail( named_view_copy);
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View * WT_Named_View_List::find_named_view_from_name (char const * name)
{
    return find_named_view_from_name(WT_String((int)strlen(name),name));
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View * WT_Named_View_List::find_named_view_from_name (WT_Unsigned_Integer16 const * name)
{
    return find_named_view_from_name(WT_String(WT_String::wcslen(name),name));
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View * WT_Named_View_List::find_named_view_from_name (WT_String const & name)
{
    if (is_empty())
        return WD_Null;

    // find associated view
    WT_Named_View *         current_named_view = (WT_Named_View *) get_head();

    while (current_named_view != WD_Null)
    {
        // NOTE: If more than one named view has the same name, we return the first instance
        if (current_named_view->name() == name)
            return current_named_view;
        current_named_view = (WT_Named_View *) current_named_view->next();
    }
    // Didn't find a view in the list
    return WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Named_View_List::default_process(WT_Named_View_List &, WT_File &)
{
    return WT_Result::Success;
}
