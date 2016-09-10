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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/dwginfo.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Drawing_Info::sync(WT_File & file)
{
    if (m_changed & Author_Bit)
    {
        author().serialize(file);
    }
    if (m_changed & Comments_Bit)
    {
        comments().serialize(file);
    }
    if (m_changed & Creator_Bit)
    {
        creator().serialize(file);
    }
    if (m_changed & Creation_Time_Bit)
    {
        creation_time().serialize(file);
    }
    if (m_changed & Description_Bit)
    {
        description().serialize(file);
    }
    if (m_changed & Modification_Time_Bit)
    {
        modification_time().serialize(file);
    }
    if (m_changed & Source_Filename_Bit)
    {
        source_filename().serialize(file);
    }
    if (m_changed & Source_Creation_Time_Bit)
    {
        source_creation_time().serialize(file);
    }
    if (m_changed & Source_Modification_Time_Bit)
    {
        source_modification_time().serialize(file);
    }
    if (m_changed & Named_View_List_Bit)
    {
        named_view_list().serialize(file);
    }
    if (m_changed & Units_Bit)
    {
        units().serialize(file);
    }
    if (m_changed & Copyright_Bit)
    {
        copyright().serialize(file);
    }
    if (m_changed & Keywords_Bit)
    {
        keywords().serialize(file);
    }
    if (m_changed & Title_Bit)
    {
        title().serialize(file);
    }
    if (m_changed & Subject_Bit)
    {
        subject().serialize(file);
    }

    m_changed = 0;
    return WT_Result::Success;
}
#else
WT_Result WT_Drawing_Info::sync(WT_File &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Drawing_Info::default_process(WT_Drawing_Info &, WT_File &)
{
    return WT_Result::Success;
}
