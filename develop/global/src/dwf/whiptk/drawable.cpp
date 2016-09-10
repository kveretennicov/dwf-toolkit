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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/drawable.cpp 1     9/12/04 8:52p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Drawable::object_type() const
{
    return Drawable;
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Box WT_Drawable::bounds (WT_File * file)
{
    if (m_bounds_set == WD_False)
    {
        m_bounds.m_min = WT_Logical_Point(0x7FFFFFFF,0x7FFFFFFF);
        m_bounds.m_max = WT_Logical_Point(0x80000000,0x80000000);
        update_bounds(file);
        m_bounds_set = WD_True;
    }
    return m_bounds;
}

///////////////////////////////////////////////////////////////////////////
void WT_Drawable::update_bounds(const WT_Logical_Point& in_pt, WT_File * file)
{
    bool use_lineweights = file && file->rendition().line_weight().weight_value()>0;

    int lw_delta = use_lineweights ? (int)(file->rendition().line_weight().weight_value()/2) : 0;

    for (int lw_stage=0; lw_stage< (use_lineweights ? 2 : 1); lw_stage++)
    {
        WT_Logical_Point pt;

        switch(lw_stage)
        {
        case 0:
            if (!use_lineweights)
                pt = in_pt;
            else
            {
                pt.m_x = (int)WD_MIN((double)in_pt.m_x + (double)lw_delta, 2147483647.0);
                pt.m_y = (int)WD_MIN((double)in_pt.m_y + (double)lw_delta, 2147483647.0);
            }
            break;
        case 1:
            {
                pt.m_x = (int)WD_MAX((double)in_pt.m_x - (double)lw_delta, -2147483648.0);
                pt.m_y = (int)WD_MAX((double)in_pt.m_y - (double)lw_delta, -2147483648.0);
                break;
            }
        }

        if (pt.m_x < m_bounds.m_min.m_x)
            m_bounds.m_min.m_x = pt.m_x;
        if (pt.m_x > m_bounds.m_max.m_x)
            m_bounds.m_max.m_x = pt.m_x;
        if (pt.m_y < m_bounds.m_min.m_y)
            m_bounds.m_min.m_y = pt.m_y;
        if (pt.m_y > m_bounds.m_max.m_y)
            m_bounds.m_max.m_y = pt.m_y;
    }
}

///////////////////////////////////////////////////////////////////////////
void WT_Drawable::update_bounds(const WT_Point_Set_Data& set, WT_File * file)
{
    update_bounds(set.points(), set.count(), file);
}

///////////////////////////////////////////////////////////////////////////
void WT_Drawable::update_bounds(const WT_Logical_Point* pts, int count, WT_File * file)
{
    for (int i=0;i<count; i++)
    {
        update_bounds(pts[i], file);
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Drawable::dump(WT_File &) const
{
    // We shouldn't ever get called here since this
    // method should have been overloaded by any drawable
    // that had allowed itself to be delayed.
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Drawable::merge(WT_Drawable const &)
{
    // We shouldn't ever get called here since this
    // method should have been overloaded by any drawable
    // that had allowed itself to be delayed.
    throw WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Drawable::delay(WT_File &) const
{
    // We shouldn't ever get called here since this
    // method should have been overloaded by any drawable
    // that had allowed itself to be delayed.
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void WT_Drawable::update_bounds(WT_File *)
{}
