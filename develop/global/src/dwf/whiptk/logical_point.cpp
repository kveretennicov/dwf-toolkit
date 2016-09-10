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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/logical_point.cpp 1     9/12/04 8:54p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Logical_Point::operator== (WT_Logical_Point_16 const & lp) const
{
    if (m_x == (WT_Integer32)lp.m_x &&
        m_y == (WT_Integer32)lp.m_y)
        return WD_True;
    else
        return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Point const & WT_Logical_Point::operator= (WT_Logical_Point_16 const & in)
{
    m_x = (WT_Integer32) in.m_x;
    m_y = (WT_Integer32) in.m_y;
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Point const WT_Logical_Point::operator* (WT_Transform const & trans) const
{
    WT_Logical_Point new_point;

    // new_point.m_x = (WT_Integer32) ( ((double)m_x + trans.m_translate.m_x) * trans.m_x_scale );
    // new_point.m_y = (WT_Integer32) ( ((double)m_y + trans.m_translate.m_y) * trans.m_y_scale );

    switch (trans.rotation())
    {
        case 0:
            new_point.m_x = (WT_Integer32) (((double)m_x * trans.m_x_scale) + trans.m_translate.m_x);
            new_point.m_y = (WT_Integer32) (((double)m_y * trans.m_y_scale) + trans.m_translate.m_y);
            break;
        case 90:
            new_point.m_x = 0x7FFFFFFF - ((WT_Integer32) (((double)m_y * trans.m_y_scale) + trans.m_translate.m_y));
            new_point.m_y = (WT_Integer32) (((double)m_x * trans.m_x_scale) + trans.m_translate.m_x);
            break;
        case 180:
            new_point.m_x = 0x7FFFFFFF - ((WT_Integer32) (((double)m_x * trans.m_x_scale) + trans.m_translate.m_x));
            new_point.m_y = 0x7FFFFFFF - ((WT_Integer32) (((double)m_y * trans.m_y_scale) + trans.m_translate.m_y));
            break;
        case 270:
            new_point.m_x = (WT_Integer32) (((double)m_y * trans.m_y_scale) + trans.m_translate.m_y);
            new_point.m_y = 0x7FFFFFFF - ((WT_Integer32) (((double)m_x * trans.m_x_scale) + trans.m_translate.m_x));
            break;
        default:
            throw WT_Result::Internal_Error;
    }

    return new_point;
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Point const WT_Logical_Point::operator*= (WT_Transform const & trans)
{
    // Need a temp copy since with some rotations will be mucking with intermediate terms
    WT_Logical_Point    orig_point(m_x, m_y);

    *this = orig_point * trans;
    return *this;
}
