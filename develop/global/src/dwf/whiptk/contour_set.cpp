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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/contour_set.cpp 8     7/05/05 8:56p Bangiav $

// All lines above the precompiled header are ignored.
#include "whiptk/pch.h"

#define WD_MAXIMUM_POINT_SET_SIZE    (256 + 65535)

///////////////////////////////////////////////////////////////////////////
WT_Contour_Set::WT_Contour_Set(
        WT_File &                   file,
        WT_Integer32                contours,
        WT_Integer32 const *        counts,
        WT_Integer32                total_point_count,
        WT_Logical_Point const *    points,
        WT_Boolean                  copy) throw(WT_Result)
      : m_incarnation()
      , m_num_contours()
      , m_counts_list(WD_Null)
      , m_total_point_count()
      , m_local_copy(WD_False)
      , m_local_copy_counts(WD_False)
      , m_points(WD_Null)
      , m_transformed()
      , m_relativized()
      , m_stage()
      , m_stage_item_counter()
{
	WT_Result res = set(file.next_incarnation(), contours, counts, total_point_count, points, copy);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_Contour_Set* WT_Contour_Set::copy(
    WT_Boolean bCopy) const
{
    return new WT_Contour_Set( *this, bCopy );
}

///////////////////////////////////////////////////////////////////////////
// Copy constructor
WT_Contour_Set::WT_Contour_Set(
        WT_Contour_Set const &  source,
        WT_Boolean              copy) throw(WT_Result)
      : WT_Drawable()
      , m_incarnation()
      , m_num_contours()
      , m_counts_list(WD_Null)
      , m_total_point_count()
      , m_local_copy(WD_False)
      , m_local_copy_counts(WD_False)
      , m_points(WD_Null)
      , m_transformed()
      , m_relativized()
      , m_stage()
      , m_stage_item_counter()
{

	WT_Result res = set (   source.m_incarnation,
            source.m_num_contours,
            source.counts(),
            source.m_total_point_count,
            source.points(),
			copy);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_Contour_Set::WT_Contour_Set(
    WT_File &                   file,
    WT_Integer32                point_count,
    WT_Logical_Point const *    points) throw(WT_Result)
      : m_incarnation()
      , m_num_contours()
      , m_counts_list(WD_Null)
      , m_total_point_count()
      , m_local_copy(WD_False)
      , m_local_copy_counts(WD_False)
      , m_points(WD_Null)
      , m_transformed()
      , m_relativized()
      , m_stage()
      , m_stage_item_counter()
{
	WT_Result res = set (   file.next_incarnation(),
            1,  // one contour in a polygon
            &point_count,
            point_count,
            points,
			WD_True);// we MUST copy the data since the point count is a stack variable
    if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_Contour_Set::~WT_Contour_Set()
{
    if (m_local_copy)
    {
        delete []m_points;
    }
    if(m_local_copy_counts)
    {
        delete []m_counts_list;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::set(
        WT_Unsigned_Integer32       incarnation,
        WT_Integer32                contours,
        WT_Integer32 const *        counts,
        WT_Integer32                total_point_count,
        WT_Logical_Point const *    points,
        WT_Boolean                  copy) 
{
    WD_Assert(total_point_count <= WD_MAXIMUM_POINT_SET_SIZE);
    WD_Assert(total_point_count > 0);

    m_incarnation = incarnation;
    m_transformed = WD_False;
    m_relativized = WD_False;
    m_stage       = Getting_Num_Contours;
    m_stage_item_counter = 0;

    if (total_point_count > WD_MAXIMUM_POINT_SET_SIZE)
    {
        m_total_point_count = WD_MAXIMUM_POINT_SET_SIZE;
    }
    else
    {
        m_total_point_count = total_point_count;
    }

    m_num_contours = contours;

    // Clean out any previous stuff
    if (m_local_copy)
    {
         delete []m_points;
         m_points = WD_Null;
    }
    if( m_local_copy_counts)
    {
        delete []m_counts_list;
        m_counts_list = WD_Null;
    }

    m_local_copy_counts = m_local_copy = copy;

    if (!copy)
    {
        m_counts_list = (WT_Integer32 *) counts;
        m_points      = (WT_Logical_Point *) points;
    }
    else
    {
        WD_Assert(m_points == WD_Null);
        m_points      = new WT_Logical_Point[m_total_point_count];
        if (m_points == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        WD_Assert(m_counts_list == WD_Null);
        m_counts_list = new WT_Integer32[m_num_contours];
        if (m_counts_list == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        WD_COPY_MEMORY( (void *)points,
                        m_total_point_count * sizeof(WT_Logical_Point),
                        m_points);

        WD_COPY_MEMORY( (void *)counts,
                        m_num_contours * sizeof(WT_Integer32),
                        m_counts_list);
    }

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Contour_Set::object_id() const
{
    return Contour_Set_ID;
}

///////////////////////////////////////////////////////////////////////////
// Note, you must first relativize the points before you can check to see if
// they can be stored in 16 bits.  This routine can't call the relativize method
// itself, since the "relativization" timing depends on the higher level routines.
//  (WT_File gets changed)
WT_Boolean WT_Contour_Set::first_point_fits_in_16_bits() const
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert (m_relativized);
    if (!m_relativized)
        return WD_False; // The points are in absolute coords, usage error.

    if (WD_fits_in_short(m_points[0].m_x) && WD_fits_in_short(m_points[0].m_y))
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
WT_Boolean WT_Contour_Set::remaining_points_fit_in_16_bits() const
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (!m_relativized)
    {
        return WD_False;
    }

    WT_Logical_Point *    source = m_points + 1;

    for (int loop = 1; loop < m_total_point_count; loop++, source++)
    {
        if (!WD_fits_in_short(source->m_x) || !WD_fits_in_short(source->m_y))
        {
            return WD_False;
        }
    }

    return WD_True;

#else
    return WD_False;
#endif  // DESIRED_CODE()
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Contour_Set::serialize(WT_File & file, WT_Boolean embeded_in_another_opcode) const
{
    // Make sure we have a legal contour set
    WD_Assert(m_total_point_count > 2);

    if (!embeded_in_another_opcode)
    {
        WD_CHECK (file.dump_delayed_drawable());

        WT_Integer32 parts_to_sync =        WT_Rendition::Color_Bit           |
                                        //  WT_Rendition::Color_Map_Bit       |
                                            WT_Rendition::Fill_Bit            |
                                            WT_Rendition::Visibility_Bit      |
                                            WT_Rendition::Line_Weight_Bit     |
                                            WT_Rendition::Pen_Pattern_Bit     |
                                            WT_Rendition::Line_Pattern_Bit    |
                                            WT_Rendition::BlockRef_Bit        |
                                            WT_Rendition::Dash_Pattern_Bit    |
                                        //  WT_Rendition::Marker_Size_Bit     |
                                        //  WT_Rendition::Marker_Symbol_Bit   |
                                            WT_Rendition::URL_Bit             |
                                            WT_Rendition::Viewport_Bit        |
                                            WT_Rendition::Layer_Bit           |
                                            WT_Rendition::Object_Node_Bit     |
                                            WT_Rendition::Delineate_Bit       |
                                            WT_Rendition::User_Fill_Pattern_Bit  |
                                            WT_Rendition::User_Hatch_Pattern_Bit |
                                            WT_Rendition::Contrast_Color_Bit;


        WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));
    }

    if (file.heuristics().apply_transform())
    {
        ((WT_Contour_Set *)this)->transform(file.heuristics().transform());
    }

    if (   file.heuristics().allow_binary_data()
        && m_total_point_count <= WD_MAX_DWF_COUNT_VALUE
        && m_num_contours      <= WD_MAX_DWF_COUNT_VALUE)
    {
        WT_Logical_Point    first_point_absolute(m_points[0]);

        WD_Assert(!m_relativized);
        ((WT_Contour_Set *)this)->relativize(file);

        WT_Boolean  use_16_bit_mode;

        if (remaining_points_fit_in_16_bits() &&
            (first_point_fits_in_16_bits() || m_total_point_count >= 3))
        {
            if (!first_point_fits_in_16_bits())
            {
                // We can save space by setting the first point as the origin,
                // and using relative coordinates for the whole point set.

                WT_Origin   new_origin (first_point_absolute);

                WD_CHECK(new_origin.serialize (file, WD_False));

                m_points[0] = WT_Logical_Point (0,0);       // new relative position
            }

            WD_CHECK (file.write((WT_Byte)0x0B));   // 16-bit Contourset opcode
            use_16_bit_mode = WD_True;
        }
        else
        {
            WD_CHECK (file.write((WT_Byte)'k'));    // 32-bit contourset opcode
            use_16_bit_mode = WD_False;
        }

        // write number of contours
        WD_CHECK (file.write_count (m_num_contours));

        // write number of points per contour
        for (int ii = 0; ii < m_num_contours; ii++)
        {
            WD_CHECK (file.write_count (m_counts_list[ii]));
        }

        if (use_16_bit_mode)
        {
            for (int loop = 0; loop < m_total_point_count; loop++)
            {
                WD_CHECK (file.write((WT_Integer16) m_points[loop].m_x));
                WD_CHECK (file.write((WT_Integer16) m_points[loop].m_y));
            }
            return WT_Result::Success;
        }
        else
        {
            return file.write(m_total_point_count, m_points);
        }
    } // If binary
    else
    {
        // ASCII only output

        WD_Assert (!m_relativized);     // ASCII output needs to have *absolute* coordinates

        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Contour "));
        WD_CHECK (file.write_ascii(m_num_contours));

        int loop;
        for (loop = 0; loop < m_num_contours; loop++)
        {
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii((WT_Integer32) m_counts_list[loop]));
        }

        for (loop = 0; loop < m_total_point_count; loop++)
        {
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write_ascii(1, &m_points[loop]));
        }

        return file.write((WT_Byte) ')');
    }
}
#else
WT_Result WT_Contour_Set::serialize(WT_File &, WT_Boolean) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Logical_Point_16 * points_16_bit = NULL;
    WT_Boolean  use_16_bit;

    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            int         loop;

            switch (opcode.token()[0])
            {
            case 0x0B: // Ctrl-k
                use_16_bit = WD_True;
                break;
            case 'k':
                use_16_bit = WD_False;
                break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            } // switch

            switch (m_stage)
            {
            case Skipping_Close_Paren:
            {
                break;
            }
            case Getting_Num_Contours:
                // Find out how many contours are in the set
                WD_CHECK (file.read_count (m_num_contours));

                if (m_local_copy)
                {
                    delete []m_points;
                    m_points = WD_Null;
                }
                if(m_local_copy_counts)
                {
                    delete []m_counts_list;
                    m_counts_list = WD_Null;
                }

                WD_Assert(m_counts_list == WD_Null);
                m_counts_list = new WT_Integer32[m_num_contours];
                if (!m_counts_list)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }

                m_local_copy_counts = m_local_copy = WD_True;
                m_stage_item_counter = 0;
                m_stage = Getting_Count_List;

                // No Break here!
            case Getting_Count_List:

                while (m_stage_item_counter < m_num_contours)
                {
                    WD_CHECK (file.read_count(m_counts_list[m_stage_item_counter]));
                    m_total_point_count += m_counts_list[m_stage_item_counter];
                    m_stage_item_counter++;
                }


                WD_Assert(m_points == WD_Null);
                m_points = new WT_Logical_Point[m_total_point_count];
                if (use_16_bit)
                {
                    points_16_bit = new WT_Logical_Point_16[m_total_point_count];
                }
                if (!m_points || (use_16_bit && !points_16_bit))
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
                m_stage = Getting_Points;

                // No Break here!
            case Getting_Points:

                if (use_16_bit)
                {
                    WD_CHECK (file.read(m_total_point_count, points_16_bit));
                    for (loop = 0; loop < m_total_point_count; loop++)
                    {
                        m_points[loop] = points_16_bit[loop];
                    }
                    delete [] points_16_bit;
                }
                else
                {
                  WD_CHECK (file.read(m_total_point_count, m_points));
                }

            }   // switch (m_stage)

          m_relativized = WD_True;
          ((WT_Contour_Set *)this)->de_relativize(file);

        } break;    // Single byte
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Getting_Num_Contours:

                WD_CHECK(file.read_ascii(m_num_contours));

                if (m_local_copy_counts)
                {
                    delete []m_counts_list;
                    m_counts_list = WD_Null;
                }
                if( m_local_copy)
                {
                    delete []m_points;
                    m_points = WD_Null;
                }

                WD_Assert(m_counts_list == WD_Null);
                m_counts_list = new WT_Integer32[m_num_contours];
                
                if (!m_counts_list)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
                
                m_local_copy_counts = m_local_copy = WD_True;
                m_stage_item_counter = 0;
                m_stage = Getting_Count_List;

                // No Break here!
            case Getting_Count_List:

                while (m_stage_item_counter < m_num_contours)
                {
                    WT_Integer32    a_count;
                    WD_CHECK(file.read_ascii(a_count));
                    m_counts_list[m_stage_item_counter] = a_count;
                    m_total_point_count += a_count;
                    m_stage_item_counter++;
                }

                m_stage_item_counter = 0;
                WD_Assert(m_points == WD_Null);
                m_points = new WT_Logical_Point[m_total_point_count];
                if (!m_points)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
                m_stage = Getting_Points;

                // No Break here!
            case Getting_Points:

                while (m_stage_item_counter < m_total_point_count)
                {
                    WD_CHECK(file.read_ascii(m_points[m_stage_item_counter]));
                    m_stage_item_counter++;
                }

                m_stage = Skipping_Close_Paren;

                // No Break here!
            case Skipping_Close_Paren:

                WD_CHECK (opcode.skip_past_matching_paren(file));
                // Yeah! we finished.

                break;
            default:
                return WT_Result::Internal_Error;
            } // switch (m_stage)

        } break;  // Extended ASCII
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    m_stage = Getting_Num_Contours;
    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Contour_Set::update_bounds(WT_File * file)
{
    WT_Drawable::update_bounds(m_points, m_total_point_count, file);
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return WT_Contour_Set::materialize(opcode,file);
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::process(WT_File & file)
{
    WD_Assert (file.contour_set_action());
    return (file.contour_set_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::default_process(WT_Contour_Set &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::transform(WT_Transform const & transform)
{
    if (!m_transformed)
    {
        WT_Logical_Point *    source = m_points;
        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;

        if (!m_local_copy)
        {
            dest = new_points = new WT_Logical_Point[m_total_point_count];
            if (new_points == NULL)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_total_point_count; loop++)
            *(dest++) = *(source++) * transform;

        if (!m_local_copy)
        {
            m_points = new_points;
            m_local_copy = WD_True;
        }

        m_transformed = WD_True;
    } // If (!transformed)

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Contour_Set::relativize(WT_File & file)
{
    if (!m_relativized)
    {

        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;
        WT_Logical_Point *    source = m_points;

        if (!m_local_copy)
        {
            dest = new_points = new WT_Logical_Point[m_total_point_count];
            if (new_points == NULL)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_total_point_count; loop++)
        {
            *dest++ = file.update_current_point(*source++);
        }

        if (!m_local_copy)
        {
            m_points = new_points;
            m_local_copy = WD_True;
        }

        m_relativized = WD_True;
    } // If (!relativized)

	return WT_Result::Success;
}
#else
WT_Result WT_Contour_Set::relativize(WT_File &)
{
	return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contour_Set::de_relativize(WT_File & file)
{
    if (m_relativized)
    {

        WT_Logical_Point *    dest = NULL;
        WT_Logical_Point *    new_points = NULL;
        WT_Logical_Point *    source = m_points;

        if (!m_local_copy)
        {
            dest = new_points = new WT_Logical_Point[m_total_point_count];
            if (new_points == NULL)
                return WT_Result::Out_Of_Memory_Error;
        }
        else
            dest = m_points;

        for (int loop = 0; loop < m_total_point_count; loop++)
        {
            *dest++ = file.de_update_current_point(*source++);
        }

        if (!m_local_copy)
        {
            m_points = new_points;
            m_local_copy = WD_True;
        }

        m_relativized = WD_False;
    } // If (!relativized)
	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Contour_Set::operator==(WT_Contour_Set const & other) const
{
    //
    // Incarnation numbers are only valid if they have been set.
    //
    if ((m_incarnation != 0xFFFFFFFFU) && (m_incarnation == other.m_incarnation))
        return WD_True;

    if (contours() != other.contours() ||
        m_total_point_count != other.m_total_point_count)
        return WD_False;

    int loop;

    // Compare the counts list
    for (loop = 0; loop < contours(); loop++)
    {
        if (m_counts_list[loop] != other.m_counts_list[loop])
            return WD_False;
    }

    // Compare the points
    for (loop = 0; loop < m_total_point_count; loop++)
    {
        if (m_points[loop] == other.m_points[loop])
            continue;
        return WD_False;
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Contour_Set const & WT_Contour_Set::operator=(WT_Contour_Set const & source) throw(WT_Result)
{
    if(set (   source.m_incarnation,
            source.m_num_contours,
            source.counts(),
            source.m_total_point_count,
            source.points(),
			WD_True) == WT_Result::Success)       // force a copy

		return *this;
	else
		throw WT_Result::Out_Of_Memory_Error;

}
