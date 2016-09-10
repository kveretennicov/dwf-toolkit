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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/pointset.cpp 1     9/12/04 8:56p Evansg $


#include "whiptk/pch.h"

#define WD_MAXIMUM_POINT_SET_SIZE    (256 + 65535)

///////////////////////////////////////////////////////////////////////////
WT_Point_Set_Data::WT_Point_Set_Data(
    int                         count,
    WT_Logical_Point const *    points,
    WT_Boolean                  copy) throw(WT_Result)
    : m_count(count)
    , m_allocated(0)
    , m_transformed(WD_False)
    , m_relativized(WD_False)
{
    set(count, points, copy);
}

///////////////////////////////////////////////////////////////////////////
WT_Point_Set_Data const &
WT_Point_Set_Data::set(
    int                         count,
    WT_Logical_Point const *    points,
    WT_Boolean                  copy) throw(WT_Result)
{
    m_transformed = WD_False;
    m_relativized = WD_False;
    m_count = count;

    WD_Assert(count <= WD_MAXIMUM_POINT_SET_SIZE);
    WD_Assert(count > 0);

    if (count > WD_MAXIMUM_POINT_SET_SIZE)
        count = WD_MAXIMUM_POINT_SET_SIZE;

    if (m_allocated)
        delete[] m_points;

    if (!copy)
    {
        m_points = (WT_Logical_Point *) points;
        m_allocated = 0;
    }
    else
    {
        m_allocated = count + 3;    // Make room for some extra points so we don't need to realloc to do a merge
        m_points = new WT_Logical_Point[m_allocated];
        if (!m_points)
            throw WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY((void *)points, count * sizeof(WT_Logical_Point), m_points);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Point_Set_Data::~WT_Point_Set_Data()
{
    if (m_allocated)
        delete []m_points;
}


///////////////////////////////////////////////////////////////////////////
void WT_Point_Set_Data::clear()
{
    if (m_allocated)
        delete []m_points;
    m_points = WD_Null;
    m_allocated = 0;
    m_count = 0;
    m_points_materialized = WD_False;
    m_transformed = WD_False;
    m_relativized = WD_False;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set_Data::transform(WT_Transform const & transform)
{
    if (!m_transformed)
    {
        WT_Logical_Point *    source = m_points;
        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;

        if (!m_allocated)
        {
            dest = new_points = new WT_Logical_Point[m_count];
            if (!new_points)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_count; loop++)
            *(dest++) = *(source++) * transform;

        if (!m_allocated)
        {
            m_points = new_points;
            m_allocated = m_count;
        }

        m_transformed = WD_True;
    } // If (!transformed)

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Point_Set_Data::relativize(WT_File & file)
{
    if (!m_relativized)
    {
        WT_Logical_Point *    source = m_points;
        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;

        if (!m_allocated)
        {
            dest = new_points = new WT_Logical_Point[m_count];
            if (!new_points)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_count; loop++)
        {
            *dest++ = file.update_current_point(*source++);
        }

        if (!m_allocated)
        {
            m_points = new_points;
            m_allocated = m_count;
        }

        m_relativized = WD_True;
    } // If (!relativized)

	return WT_Result::Success;
}
#else
WT_Result WT_Point_Set_Data::relativize(WT_File &)
{
	return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set_Data::de_relativize(WT_File & file)
{
    if (m_relativized)
    {
        WT_Logical_Point *    source = m_points;
        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;

        if (!m_allocated)
        {
            dest = new_points = new WT_Logical_Point[m_count];
            if (!new_points)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_count; loop++)
        {
            *dest++ = file.de_update_current_point(*source++);
        }

        if (!m_allocated)
        {
            m_points = new_points;
            m_allocated = m_count;
        }

        m_relativized = WD_False;
    } // If (!relativized)

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
// Note, you must first relativize the points before you can check to see if
// they can be stored in 16 bits.  This routine can't call the relativize method
// itself, since the "relativization" timing depends on the higher level routines.
//  (WT_File gets changed)
WT_Boolean WT_Point_Set_Data::first_point_fits_in_16_bits() const
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (!m_relativized)
        return WD_False;

    if (WD_fits_in_short(m_points[0].m_x) &&
        WD_fits_in_short(m_points[0].m_y)    )
        return WD_True;
    else
        return WD_False;

#else
    return WD_False;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
// Note, you must first relativize the points before you can check to see if
// they can be stored in 16 bits.  This routine can't call the relativize method
// itself, since the "relativization" timing depends on the higher level routines.
//  (WT_File gets changed)
WT_Boolean WT_Point_Set_Data::remaining_points_fit_in_16_bits() const
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WT_Logical_Point *    source = m_points + 1;

    if (!m_relativized)
        return WD_False;

    for (int loop = 1; loop < m_count; loop++, source++)
    {
        if (!WD_fits_in_short(source->m_x) ||
            !WD_fits_in_short(source->m_y)    )
            return WD_False;
    }

    return WD_True;

#else
    return WD_False;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Point_Set_Data::operator== (WT_Point_Set_Data const & set) const
{
    if (m_count != set.m_count ||
        m_transformed != set.m_transformed ||
        m_relativized != set.m_relativized )
        return WD_False;
    for (int loop = 0; loop < m_count; loop++)
        if (!( m_points[loop] == set.m_points[loop] ))
            return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Point_Set::serialize(
                                  WT_File &  file,
                                  WT_Byte    opcode_ascii,
                                  WT_Byte    opcode_32bit,
                                  WT_Byte    opcode_16bit) const
{
    // This method assumes that binary data is allowed.
    WD_Assert(m_count > 0);

    if (file.heuristics().apply_transform())
        ((WT_Point_Set_Data *)this)->transform(file.heuristics().transform());

    if (   file.heuristics().allow_binary_data()
        && m_count <= WD_MAX_DWF_COUNT_VALUE)
    {
        WD_Assert(!m_relativized);
        WT_Logical_Point    first_point_absolute(m_points[0]);

        ((WT_Point_Set_Data *)this)->relativize(file);

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
            }
            return WT_Result::Success;
        }
        else
        {
            return file.write(m_count, m_points);
        }
    } // If binary
    else
    {
        // ASCII only output

        WD_Assert (!m_relativized);

        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write(opcode_ascii));
        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(m_count));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii(1, &m_points[0]));   // First Point

        for (int loop = 1; loop < m_count; loop++)
        {
            if (!(loop % 4))
            {
                WD_CHECK (file.write_geom_tab_level());
                WD_CHECK (file.write("    "));
            }
            else
                WD_CHECK (file.write((WT_Byte) ' '));

            WD_CHECK (file.write_ascii(1, &m_points[loop]));
        }
        return WT_Result::Success;
    }
}
#else
WT_Result WT_Point_Set::serialize(WT_File &, WT_Byte, WT_Byte, WT_Byte) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::materialize(WT_File & file)
{
    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        WD_CHECK(file.read_count(m_count));

        WD_Assert(m_points == WD_Null);
        m_points = new WT_Logical_Point[m_count];
        if (!m_points)
            return WT_Result::Out_Of_Memory_Error;
        m_allocated = m_count;
    }

    WD_CHECK (file.read(m_count, m_points));

    m_relativized = WD_True;
    ((WT_Point_Set_Data *)this)->de_relativize(file);

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::materialize_ascii(WT_File & file)
{
    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        WD_CHECK (file.read_ascii(m_count));

        if (m_count < 1)
            return WT_Result::Corrupt_File_Error;

        WD_Assert(m_points == WD_Null);
        m_points = new WT_Logical_Point[m_count];

        if (!m_points)
            return WT_Result::Out_Of_Memory_Error;

        m_points_materialized = 0;
        m_allocated = m_count;
    }

    for (int loop = m_points_materialized; loop < m_count; loop++)
    {
        WD_CHECK (file.read_ascii(m_points[loop]));
        m_points_materialized++;
    }

    m_relativized = WD_False;

    // No need to de-relativize ASCII -- they are always absolute

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::materialize_16_bit(WT_File & file)
{
    WT_Byte        count;
    WT_Logical_Point_16 * tmp_buf = WD_Null;


    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        if (m_count == 0)
        {
            // It is time to find out how many vertices there are in this polything.
            WD_CHECK (file.read(count));

            // If there are more than 255 vertices, a zero will indicate that there
            // will be a two-byte vertex count following.
            if (count == 0)
                m_count = -1;
            else
                m_count = count;
        }

        if (m_count == -1)
        {
            // We have more than 255 vertices, so we will read an additional two-byte
            // vertex counter.
            WT_Unsigned_Integer16    tmp_short;
            WD_CHECK (file.read(tmp_short));

            m_count = 256 + tmp_short;
        }

        WD_Assert(m_points == WD_Null);

        m_points = new WT_Logical_Point[m_count];
        if (!m_points)
            return WT_Result::Out_Of_Memory_Error;
        m_allocated = m_count;
    }

    tmp_buf  = new WT_Logical_Point_16[m_count];
    if (!tmp_buf)
        return WT_Result::Out_Of_Memory_Error;

    WT_Result    result = file.read(m_count, tmp_buf);
    if (result != WT_Result::Success)
    {
        delete []tmp_buf;
        return result;
    }

    for (int loop = 0; loop < m_count; loop++)
    {
        m_points[loop] = tmp_buf[loop];
    }

    delete []tmp_buf;

    m_relativized = WD_True;
    ((WT_Point_Set_Data *)this)->de_relativize(file);

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::skip_operand(WT_File & file)
{
    WT_Byte        count;

    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        if (m_count == 0)
        {
            WD_CHECK (file.read(count));

            if (count == 0)
                m_count = -1;
            else
                m_count = count;
        }

        if (m_count == -1)
        {
            WT_Unsigned_Integer16    tmp_short;
            WD_CHECK (file.read(tmp_short));

            m_count = 256 + tmp_short;
        }
    }

    file.skip(sizeof(WT_Logical_Point) * m_count);

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::skip_operand_ascii(WT_File & file)
{
    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        WD_CHECK (file.read_ascii(m_count));

        if (m_count < 1)
            return WT_Result::Corrupt_File_Error;
    }

    for (int loop = 0; loop < m_count; loop++)
    {
        WT_Logical_Point tmp;
        WD_CHECK (file.read_ascii(tmp));
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Point_Set::skip_operand_16_bit(WT_File & file)
{
    WT_Byte        count;

    // Find out how many vertices there will be.
    if (m_count <= 0)
    {
        if (m_count == 0)
        {
            WD_CHECK (file.read(count));

            if (count == 0)
                m_count = -1;
            else
                m_count = count;
        }

        if (m_count == -1)
        {
            WT_Unsigned_Integer16    tmp_short;
            WD_CHECK (file.read(tmp_short));

            m_count = 256 + tmp_short;
        }
    }

    file.skip(sizeof(WT_Logical_Point_16) * m_count);

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Point_Set::clear()
{
    WT_Point_Set_Data::clear();
    m_points_materialized = WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Point_Set::~WT_Point_Set()
{
}
