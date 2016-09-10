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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/heuristics.cpp 1     9/12/04 8:53p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_File_Heuristics::WT_File_Heuristics()
: m_allow_data_compression (WD_False)
, m_allow_indexed_colors (WD_True)
, m_allow_binary_data (WD_True)
, m_allow_drawable_merging (WD_True)
, m_apply_transform(WD_False)
, m_apply_transform_to_units_matrix(WD_True)
, m_target_version(0)
, m_user_data(WD_Null)
, m_deferred_delete(WD_False)
, m_broken_plotinfo(WD_False)
, m_w2d_channel(WD_False)
{ }

///////////////////////////////////////////////////////////////////////////
WT_File_Heuristics::~WT_File_Heuristics()
{ }


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_File_Heuristics::set_target_version(int target, WT_File *file)
{
    m_target_version = target;

    // Need to reset the File's renditions to have the proper default values for the
    // requested version.

    if (file)
        file->rendition().color_map() = WT_Color_Map(target);
}
#else
void WT_File_Heuristics::set_target_version(int, WT_File *)
{}
#endif  // DESIRED_CODE()
