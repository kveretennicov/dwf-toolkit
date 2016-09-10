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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/XAML/XamlPointset.cpp#1 $


#include "XAML/pch.h"

#define WD_MAXIMUM_POINT_SET_SIZE    (256 + 65535)

WT_XAML_Point_Set_Data::WT_XAML_Point_Set_Data( vector<WT_Point2D>& points ) throw (WT_Result)
: m_count(0)
, m_allocated(0)
, m_points(NULL)
, m_relativized(false)
{
    m_count = (WT_Integer32) points.size();
    
    WD_Assert(m_count <= WD_MAXIMUM_POINT_SET_SIZE);
    WD_Assert(m_count > 0);

    if (m_count > WD_MAXIMUM_POINT_SET_SIZE)
        m_count = WD_MAXIMUM_POINT_SET_SIZE;

    m_allocated = m_count + 3;    // Make room for some extra points so we don't need to realloc to do a merge
    m_points = new WT_Point2D[m_allocated];
    if (!m_points)
        throw WT_Result::Out_Of_Memory_Error;

    vector<WT_Point2D>::const_iterator iter = points.begin();
    WT_Integer32 i=0; 
    for(; iter != points.end(); iter++)
    {
        m_points[i++] = *iter;
    }
}


///////////////////////////////////////////////////////////////////////////
WT_XAML_Point_Set_Data::WT_XAML_Point_Set_Data(
    int                         count,
    WT_Logical_Point const *    points)
    : m_count(count)
    , m_allocated(0)
    , m_points(NULL)
    , m_relativized(false)
{
    set(count, points);
}

///////////////////////////////////////////////////////////////////////////
WT_XAML_Point_Set_Data::WT_XAML_Point_Set_Data(
    int                         count,
    WT_Point2D const *          points,
    bool                        copy)
    : m_count(count)
    , m_allocated(0)
    , m_points(NULL)
    , m_relativized(false)
{
    set(count, points, copy);
}


///////////////////////////////////////////////////////////////////////////
WT_XAML_Point_Set_Data const &
WT_XAML_Point_Set_Data::set(
    int                         count,
    WT_Logical_Point const *    points)
{
    m_relativized = false;
    m_count = count;

    WD_Assert(count <= WD_MAXIMUM_POINT_SET_SIZE);
    WD_Assert(count > 0);

    if (count > WD_MAXIMUM_POINT_SET_SIZE)
        count = WD_MAXIMUM_POINT_SET_SIZE;

    if (m_allocated && (m_allocated < count) )
    {
        delete[] m_points;
        m_allocated = 0;
        m_points = NULL;
    }

    if (!m_points)
    {
        m_allocated = count + 3;    // Make room for some extra points so we don't need to realloc to do a merge
        m_points = new WT_Point2D[m_allocated];
        if (!m_points)
            throw WT_Result::Out_Of_Memory_Error;
    }

    for(int i=0;i<count;i++)
    {
        m_points[i].m_x = points[i].m_x;
        m_points[i].m_y = points[i].m_y;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_XAML_Point_Set_Data const &
WT_XAML_Point_Set_Data::set(
    int                         count,
    WT_Point2D const *          points,
    bool                        copy)
{
    m_relativized = false;
    m_count = count;

    WD_Assert(count <= WD_MAXIMUM_POINT_SET_SIZE);
    WD_Assert(count > 0);

    if (count > WD_MAXIMUM_POINT_SET_SIZE)
        count = WD_MAXIMUM_POINT_SET_SIZE;

    if ( ( m_allocated > 0 ) && (m_allocated < count) )
    {
        delete[] m_points;
        m_allocated = 0;
        m_points = NULL;
    }

    if (!copy)
    {
        if ( m_allocated > 0 )
        {
            delete[] m_points;
            m_allocated = 0;
        }
        m_points = (WT_Point2D *) points;
    }
    else
    {
        if (!m_points)
        {
            m_allocated = count + 3;    // Make room for some extra points so we don't need to realloc to do a merge
            m_points = new WT_Point2D[m_allocated];
            if (!m_points)
                throw WT_Result::Out_Of_Memory_Error;
        }
        WD_COPY_MEMORY((void *)points, count * sizeof(WT_Point2D), m_points);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_XAML_Point_Set_Data::~WT_XAML_Point_Set_Data()
{
    if (m_allocated)
        delete []m_points;
}


///////////////////////////////////////////////////////////////////////////
void WT_XAML_Point_Set_Data::clear()
{
    if (m_allocated)
        delete []m_points;
    m_points = NULL;
    m_allocated = 0;
    m_count = 0;
    m_relativized = false;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_XAML_Point_Set_Data::relativize(WT_File & file)
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File&>(file);

    if (!m_relativized)
    {
        WT_Point2D *    source = m_points;
        WT_Point2D *    dest = NULL;
        WT_Point2D *    new_points = NULL;

        if (!m_allocated)
        {
            dest = new_points = new WT_Point2D[m_count];
            if (!new_points)
                throw WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_count; loop++)
        {
            *dest++ = rFile.update_current_point(*source++);
        }

        if (!m_allocated)
        {
            m_points = new_points;
            m_allocated = m_count;
        }

        m_relativized = true;
    } // If (!relativized)
}
#else
void WT_XAML_Point_Set_Data::relativize(WT_File &)
{}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_XAML_Point_Set_Data::de_relativize(WT_File & file)
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File&>(file);

    if (m_relativized)
    {
        WT_Point2D *    source = m_points;
        WT_Point2D *    dest = NULL;
        WT_Point2D *    new_points = NULL;

        if (!m_allocated)
        {
            dest = new_points = new WT_Point2D[m_count];
            if (!new_points)
                throw WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_count; loop++)
        {
            *dest++ = rFile.de_update_current_point(*source++);
        }

        if (!m_allocated)
        {
            m_points = new_points;
            m_allocated = m_count;
        }

        m_relativized = false;
    } // If (!relativized)
}

///////////////////////////////////////////////////////////////////////////
bool WT_XAML_Point_Set_Data::operator== (WT_XAML_Point_Set_Data const & set) const
{
    if (m_count != set.m_count ||
        m_relativized != set.m_relativized )
        return false;
    for (int loop = 0; loop < m_count; loop++)
        if (!( m_points[loop] == set.m_points[loop] ))
            return false;
    return true;
}

