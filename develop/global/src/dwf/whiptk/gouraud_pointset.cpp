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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/gouraud_pointset.cpp 4     8/23/05 7:14p Bangiav $


#include "whiptk/pch.h"

#define WD_MAXIMUM_POINT_SET_SIZE    (256 + 65535)

///////////////////////////////////////////////////////////////////////////
WT_Gouraud_Point_Set_Data::WT_Gouraud_Point_Set_Data(
    int                           count,
    WT_Logical_Point const *      points,
    WT_RGBA32 const *             colors,
    WT_Boolean                    copy) throw(WT_Result)
    : WT_Point_Set_Data (count, points, copy)
    , m_colors_allocated(0)
{
    if (!copy)
    {
        m_colors = (WT_RGBA32 *) colors;
    }
    else
    {
        m_colors = new WT_RGBA32[count];
        if (m_colors == NULL)
            throw WT_Result::Out_Of_Memory_Error;
        
        // set the allocated count.
        m_colors_allocated = count;
        WD_COPY_MEMORY((void *)colors, count * sizeof(WT_RGBA32), m_colors);
    }
}


///////////////////////////////////////////////////////////////////////////
WT_Gouraud_Point_Set::WT_Gouraud_Point_Set(
    int                           count,
    WT_Logical_Point const *      points,
    WT_RGBA32 const *             colors,
    WT_Boolean                    copy) throw(WT_Result)
    : WT_Gouraud_Point_Set_Data (count, points, colors, copy)
    , m_read_mode (Materialize)
    , m_stage (Getting_Count)
{
    m_points_materialized  = 0; //inherited members cannot go in member initialization above
}

///////////////////////////////////////////////////////////////////////////
WT_Gouraud_Point_Set_Data::~WT_Gouraud_Point_Set_Data()
{
    if (m_colors_allocated)
    {
        delete []m_colors;
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Gouraud_Point_Set::serialize(WT_File &  file,
                                          WT_String  opcode_ascii,
                                          WT_Byte    opcode_32bit,
                                          WT_Byte    opcode_16bit) const
{

    // This method assumes that binary data is allowed.
    WD_Assert(m_count > 0);

    if (file.heuristics().apply_transform())
        ((WT_Point_Set *)this)->transform(file.heuristics().transform());

    WD_Assert(!m_relativized);

    if (   file.heuristics().allow_binary_data()
        && m_count <= WD_MAX_DWF_COUNT_VALUE)
    {
        WT_Logical_Point    first_point_absolute(m_points[0]);

        ((WT_Point_Set *)this)->relativize(file);

        WT_Boolean  use_16_bit_mode;

        if (remaining_points_fit_in_16_bits() &&
            (first_point_fits_in_16_bits() || m_count >= 3))
        {
            if (!first_point_fits_in_16_bits())
            {
                // We can save space by setting the first point as the origin,
                // and using relative coordinates for the whole point set.

                WT_Origin (first_point_absolute).serialize (file, WD_False);
                m_points[0] = WT_Logical_Point (0,0);       // new relative position
            }

            WD_CHECK (file.write(opcode_16bit));
            use_16_bit_mode = WD_True;
        }
        else
        {
            WD_CHECK (file.write(opcode_32bit));
            use_16_bit_mode = WD_False;
        }

        WD_CHECK(file.write_count(m_count));

        if (use_16_bit_mode)
        {
            for (int loop = 0; loop < m_count; loop++)
            {
                WD_CHECK (file.write((WT_Integer16) m_points[loop].m_x));
                WD_CHECK (file.write((WT_Integer16) m_points[loop].m_y));
                WD_CHECK (file.write(m_colors[loop]));
            }
            return WT_Result::Success;
        }
        else
        {
            for (int loop = 0; loop < m_count; ++loop)
            {
                WD_CHECK (file.write(1, &m_points[loop]));
                WD_CHECK (file.write(m_colors[loop]));
            }

            return WT_Result::Success;
        }
    }
    else
    {
        // ASCII only output
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write((WT_Byte) '('));
        WD_CHECK (file.write(opcode_ascii.ascii()));
        WD_CHECK (file.write((WT_Byte) ' '));

        WD_CHECK (file.write_ascii(m_count));

        for (int loop = 0; loop < m_count; ++loop)
        {
            if (!(loop % 3))
            {
                WD_CHECK (file.write_geom_tab_level());
                WD_CHECK (file.write("    "));
            }
            else
                WD_CHECK (file.write((WT_Byte) ' '));

            WD_CHECK (file.write_ascii(1, &m_points[loop]));
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii(m_colors[loop]));
        }
        WD_CHECK (file.write((WT_Byte) ')'));

        return WT_Result::Success;
    }
}
#else
WT_Result WT_Gouraud_Point_Set::serialize(WT_File &, WT_String, WT_Byte, WT_Byte) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
// Keep all the reading code in one place - read_pointset
// Materialize and Skip are simply different modes of reading
WT_Result WT_Gouraud_Point_Set::materialize(WT_File & file)
{
    m_read_mode = Materialize;
    return read_pointset(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::skip_operand(WT_File & file)
{
    m_read_mode = Skip;
    return read_pointset(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::read_pointset(WT_File & file)
{
    WT_Byte        count;

    switch (m_stage)
    {
        case Getting_Color:
        case Getting_Close_Paren:
        {
            break;
        }
        case Getting_Count:
            m_points_materialized = 0;

            // Find out how many vertices there will be.
            WD_CHECK (file.read(count));

            if (count == 0)
                m_count = -1;
            else
                m_count = count;

            if (m_count == -1)
            {
                WT_Unsigned_Integer16    tmp_short;
                WD_CHECK (file.read(tmp_short));

                m_count = 256 + tmp_short;
            }

            if (m_read_mode == Materialize)
            {
                WD_Assert(m_points == WD_Null);
                m_points = new WT_Logical_Point[m_count];
                if (!m_points)
                    return WT_Result::Out_Of_Memory_Error;

                m_allocated = m_count;

                WD_Assert(m_colors == WD_Null);
                m_colors = new WT_RGBA32[m_count];
                if (!m_colors)
                    return WT_Result::Out_Of_Memory_Error;

                m_colors_allocated = m_count;
            }

            m_stage = Getting_Point;

        //No break here
        case Getting_Point:
            if (m_read_mode == Skip)
            {
                file.skip((sizeof(WT_Logical_Point) + sizeof(WT_RGBA32)) * m_count);
            }
            else for (m_points_materialized=0; m_points_materialized < m_count; m_points_materialized++)
            {
                WD_CHECK (file.read(1, &m_points[m_points_materialized]));
                m_stage = Getting_Color;

                WD_CHECK (file.read (m_colors[m_points_materialized]));
                m_stage = Getting_Point;
            }
    }

    if (m_read_mode == Materialize)
    {
        m_relativized = WD_True;
        ((WT_Point_Set *)this)->de_relativize(file);

        if (file.heuristics().apply_transform())
            transform(file.heuristics().transform());
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
// Keep all the reading code in one place - read_pointset
// Materialize and Skip are simply different modes of reading
WT_Result WT_Gouraud_Point_Set::materialize_ascii(WT_File & file)
{
    m_read_mode = Materialize;
    return read_pointset_ascii(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::skip_operand_ascii(WT_File & file)
{
    m_read_mode = Skip;
    return read_pointset_ascii(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::read_pointset_ascii(WT_File & file)
{
    switch (m_stage)
    {
        case Getting_Count:
            m_points_materialized = 0;

            // Find out how many vertices there will be.
            WD_CHECK (file.read_ascii(m_count));

            if (m_count < 1)
                return WT_Result::Corrupt_File_Error;

            if (m_read_mode == Materialize)
            {
                WD_Assert(m_points == WD_Null);
                m_points = new WT_Logical_Point[m_count];

                if (!m_points)
                    return WT_Result::Out_Of_Memory_Error;

                m_allocated = m_count;

                WD_Assert(m_colors == WD_Null);
                m_colors = new WT_RGBA32[m_count];

                if (!m_colors)
                    return WT_Result::Out_Of_Memory_Error;

                m_colors_allocated = m_count;
            }
            m_stage = Getting_Point;

        //No break here
        case Getting_Point:
        {
            WT_Logical_Point dummy_point, *pPoint;
            WT_RGBA32 dummy_color, *pColor;

            for (m_points_materialized=0; m_points_materialized < m_count; m_points_materialized++)
            {
                pPoint = (m_read_mode == Skip) ? &dummy_point : &m_points[m_points_materialized];
                pColor = (m_read_mode == Skip) ? &dummy_color : &m_colors[m_points_materialized];

                WD_CHECK (file.read_ascii(*pPoint));
                m_stage = Getting_Color;

                WD_CHECK (file.read_ascii(*pColor));
                m_stage = Getting_Point;
            }
            if (m_read_mode == Skip)
                m_points_materialized=0;

            m_stage = Getting_Close_Paren;
            break;
        }

        default:
            return WT_Result::Internal_Error;
    }

    if (m_read_mode == Materialize)
    {
        m_relativized = WD_False;
        // No need to de-relativize ASCII -- they are always absolute
        if (file.heuristics().apply_transform())
            transform(file.heuristics().transform());
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
// Keep all the reading code in one place - read_pointset
// Materialize and Skip are simply different modes of reading
WT_Result WT_Gouraud_Point_Set::materialize_16_bit(WT_File & file)
{
    m_read_mode = Materialize;
    return read_pointset_16_bit(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::skip_operand_16_bit(WT_File & file)
{
    m_read_mode = Skip;
    return read_pointset_16_bit(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Point_Set::read_pointset_16_bit(WT_File & file)
{
    WT_Byte        count;

    // Find out how many vertices there will be.
    switch (m_stage)
    {
        case Getting_Color:
        case Getting_Close_Paren:
        {
            break;
        }
        case Getting_Count:
            m_points_materialized = 0;

            // Find out how many vertices there will be.
            WD_CHECK (file.read(count));

            // If there are more than 255 vertices, a zero will indicate that there
            // will be a two-byte vertex count following.
            if (count == 0)
                m_count = -1;
            else
                m_count = count;

            if (m_count == -1)
            {
                // We have more than 255 vertices, so we will read an additional two-byte
                // vertex counter.
                WT_Unsigned_Integer16    tmp_short;
                WD_CHECK (file.read(tmp_short));

                m_count = 256 + tmp_short;
            }

            if (m_read_mode == Materialize)
            {
                WD_Assert(m_points == WD_Null);
                m_points = new WT_Logical_Point[m_count];
                if (!m_points)
                    return WT_Result::Out_Of_Memory_Error;

                m_allocated = m_count;

                WD_Assert(m_colors == WD_Null);
                m_colors = new WT_RGBA32[m_count];
                if (!m_colors)
                    return WT_Result::Out_Of_Memory_Error;

                m_colors_allocated = m_count;
            }

            m_stage = Getting_Point;

        //No break here
        case Getting_Point:

            if (m_read_mode == Skip)
            {
                file.skip((sizeof(WT_Logical_Point_16) + sizeof(WT_RGBA32)) * m_count);
            }
            else for (m_points_materialized=0; m_points_materialized < m_count; m_points_materialized++)
            {
                WT_Logical_Point_16     tmp;
                WD_CHECK (file.read(1, &tmp));
                m_points[m_points_materialized] = tmp;
                m_stage = Getting_Color;

                WD_CHECK (file.read (m_colors[m_points_materialized]));
                m_stage = Getting_Point;
            }
    }

    if (m_read_mode == Materialize)
    {
        m_relativized = WD_True;
        ((WT_Point_Set *)this)->de_relativize(file);

        if (file.heuristics().apply_transform())
            transform(file.heuristics().transform());
    }

    return WT_Result::Success;
}
