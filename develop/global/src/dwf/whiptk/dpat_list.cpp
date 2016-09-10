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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/dpat_list.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

WT_Dash_Pattern* WT_Dash_Pattern_List::add_pattern( WT_Dash_Pattern const & r )
{
    add_front(new WT_Dash_Pattern(r));
    return (WT_Dash_Pattern*)get_head();
}

WT_Dash_Pattern* WT_Dash_Pattern_List::find_pattern( WT_Integer32 i )
{
    WT_Item *head;
    for( head = (WT_Dash_Pattern*)get_head();
        head != WD_Null;
        head = head->next())
    {
        if( ((WT_Dash_Pattern*)head)->number() == i )
            return (WT_Dash_Pattern*)head;
    }
    return (WT_Dash_Pattern*)&WT_Dash_Pattern::kNull;
}
