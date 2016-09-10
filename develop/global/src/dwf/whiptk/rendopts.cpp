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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/rendopts.cpp 1     9/12/04 8:56p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Rendering_Options::sync_parts(WT_File & file, WT_Integer32 needed)
{
    WT_Result               result;

    while (needed)
    {
        WT_Integer32   this_case = needed & -needed;  // Get lowest bit that is set to true

        needed &= ~this_case;
        switch (this_case)
        {
        case View_Bit:
                result = m_view.sync(file);
                break;
        case Background_Bit:
                result = m_background.sync(file);
                break;
        case Plot_Optimized_Bit:
                result = m_plot_optimized.sync(file);
                break;
        case Inked_Area_Bit:
                result = m_inked_area.sync(file);
                break;
        case PenPat_Options_Bit:
                result = m_penpat_options.sync(file);
                break;
        } // switch

        if (result != WT_Result::Success)
            return result;
    } // while

    return WT_Result::Success;
}
#else
WT_Result WT_Rendering_Options::sync_parts(WT_File &, WT_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()
