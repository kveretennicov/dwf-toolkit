//  Copyright (c) 2001-2006 by Autodesk, Inc.
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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/blockref_list.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"

#include "whiptk/blockref_list.h"

///////////////////////////////////////////////////////////////////////////
WT_BlockRef_List const &
    WT_BlockRef_List::operator=(WT_BlockRef_List const & list)
{
    remove_all();

    if (list.is_empty())
        return *this;

    WT_BlockRef *current =
        (WT_BlockRef *) list.get_head();

    while (current != WD_Null)
    {
        WT_BlockRef new_item(*current);
        add(new_item);
        current = (WT_BlockRef *) current->next();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean
    WT_BlockRef_List::operator==(WT_BlockRef_List const & list) const
{
    if (count() != list.count())
        return WD_False;

    WT_BlockRef *current_me = (WT_BlockRef *) get_head();
    WT_BlockRef *current_other =
        (WT_BlockRef *) list.get_head();

    while (current_me)
    {
        if ( current_me != current_other )
            return WD_False;

        current_me = (WT_BlockRef *) current_me->next();
        current_other = (WT_BlockRef *) current_other->next();
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
void WT_BlockRef_List::add(WT_BlockRef & item)
{
    WT_BlockRef *item_local =
        new WT_BlockRef(item);
    WT_Item_List::add_tail(item_local);
}

///////////////////////////////////////////////////////////////////////////
WT_BlockRef_List::~WT_BlockRef_List()
{
    remove_all();
}
