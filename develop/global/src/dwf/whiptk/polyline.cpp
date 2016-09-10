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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/polyline.cpp 4     2/17/05 7:26p Hainese $



#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Polyline::object_id() const
{
    return WT_Object::Polyline_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Polyline::serialize(WT_File & file) const
{
    // Make sure we have a legal point set.  Consumers must deal with degenerate cases.
    WD_Assert(m_count > 0);

    // TODO: Check rendition incarnation

    // Attributes that polylines care about:
    //        color, line weight, line cap, line join,
    //        line pattern, fill mode, visibility, URL
    //        layer

    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                        WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                        WT_Rendition::Line_Pattern_Bit    |
                                        WT_Rendition::Dash_Pattern_Bit    |
                                        WT_Rendition::Line_Style_Bit      |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::Delineate_Bit;


    // Avoid cleverness of not turning off fill for two point polylines (which
    // have no fill area). These two point polylines can get merged and so have
    // an area. Fix for defect 638876.
    if (file.rendition().fill().fill())
        file.desired_rendition().fill() = WD_False;

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    return file.merge_or_delay_drawable(*this);
}
#else
WT_Result WT_Polyline::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Polyline::dump(WT_File & file) const
{
    if (m_count == 2)
    {
        // A single line segment
        if (file.heuristics().apply_transform())
            ((WT_Polyline *)this)->transform(file.heuristics().transform());  // Cast "this" from const to alterable

        if (file.heuristics().allow_binary_data())
        {
            ((WT_Polyline *)this)->relativize(file);  // Cast "this" from const to alterable

            // Binary output
            if (first_point_fits_in_16_bits() && remaining_points_fit_in_16_bits())
            {
                // Output a 16 bit relative 2 point line
                WT_Logical_Point_16 tmp_points[2];

                tmp_points[0] = m_points[0];
                tmp_points[1] = m_points[1];

                WD_CHECK (file.write((WT_Byte)0x0C));
                WD_CHECK (file.write(2, tmp_points));
                return WT_Result::Success;
            }
            else
            {
                // Output a 32 bit relative 2 point line
                WD_CHECK (file.write((WT_Byte)'l'));
                return    file.write(2, m_points);
            }
        }  // allow binary data
        else
        {
            // ASCII only 2-point line

            WD_Assert (!m_relativized);

            WD_CHECK (file.write_geom_tab_level());
            WD_CHECK (file.write("L "));
            WD_CHECK (file.write_ascii(1, &m_points[0]));
            WD_CHECK (file.write((WT_Byte) ' '));
            return    file.write_ascii(1, &m_points[1]);
        }
    } // if (a two point line)
    else
    {    // A polyline
        return WT_Point_Set::serialize(file, (WT_Byte) 'P', (WT_Byte)'p', (WT_Byte)0x10);
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Polyline::delay(WT_File & file) const
{
    WT_Polyline * delayed = new WT_Polyline(count(), points(), WD_True);
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    file.set_delayed_drawable(delayed);

    return WT_Result::Success;
}
#else
WT_Result   WT_Polyline::delay(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Boolean WT_Polyline::merge(WT_Drawable const & new_one)
{
    enum
    {
        LAST_TO_FIRST,
        FIRST_TO_LAST,
        LAST_TO_LAST,
        FIRST_TO_FIRST
    }                       merge_type;

    WD_Assert (new_one.object_id() == Polyline_ID);

    WT_Polyline const *  current = (WT_Polyline const *)(&new_one);   // Cast safe cast to a full Polymarker

    // See if these two polylines can be merged.  Test:
    //  1) first point of old line to first point of new line
    //  2) last  point of old line to first point of new line
    //  3) last  point of old line to last  point of new line
    //  4) first point of old line to last  point of new line
    if (*points() == *current->points())
        merge_type = FIRST_TO_FIRST;
    else if (points()[count() - 1] == *current->points())
        merge_type = LAST_TO_FIRST;
    else if (points()[count() - 1] == current->points()[current->count() - 1])
        merge_type = LAST_TO_LAST;
    else if (*points() == current->points()[current->count() - 1])
        merge_type = FIRST_TO_LAST;
    else
        return WD_False; // These two polylines are not mergeable.

    // Yeah!! These two polylines are mergable.  Now make space to hold them.

    int needed = count() + current->count() - 1;  // One of the points is a duplicate.

    if (needed > allocated())
    {
        WT_Logical_Point *  new_pts = new WT_Logical_Point[needed * 2];
        if (!new_pts)
            throw WT_Result::Out_Of_Memory_Error;

        if (merge_type == LAST_TO_FIRST || merge_type == LAST_TO_LAST)
            WD_COPY_MEMORY(points(), count() * sizeof(WT_Logical_Point), new_pts);
        else
            WD_COPY_MEMORY(points(), count() * sizeof(WT_Logical_Point), &new_pts[current->count() - 1]);

        if (m_allocated)
            delete []m_points;  // We had our own local copy of the old points, so we need to delete them

        m_points = new_pts;
        m_allocated = needed * 2;
    }
    else if (merge_type == FIRST_TO_LAST || merge_type == FIRST_TO_FIRST)
    {
        // We have enough space, but we need to shift the old points to
        // allow linkage to the new points.
        WT_Logical_Point *  src  = points() + count() - 1;
        WT_Logical_Point *  dest = src + current->count() - 1; // One point is duplicated.

        // Need to copy points going backwards so we don't overwrite ourself.
        while (src >= points())
        {
            *dest-- = *src--;
        }
    }

    // At this point we know that:
    //   1) we have our own local copy of the data
    //   2) we have enough space to hold the old and new points
    //   3) the old points are in the proper position for overlay of new points.

    switch (merge_type)
    {
    case LAST_TO_FIRST:
        WD_COPY_MEMORY(current->points() + 1, (current->count() - 1) * sizeof(WT_Logical_Point), &m_points[count()]);
        break;
    case FIRST_TO_LAST:
        WD_COPY_MEMORY(current->points(), (current->count() - 1) * sizeof(WT_Logical_Point), points());
        break;
    case LAST_TO_LAST:
        {
            WT_Logical_Point *  src  = current->points() + current->count() - 2;
            WT_Logical_Point *  dest = points() + count();

            while (src >= current->points())
                *dest++ = *src--;
        } break;
    case FIRST_TO_FIRST:
        {
            WT_Logical_Point *  src  = current->points() + current->count() - 1;
            WT_Logical_Point *  dest = points();

            while (src > current->points())
                *dest++ = *src--;
        } break;
    default:
        throw WT_Result::Internal_Error;
    } // switch

    m_count += current->count() - 1; // One of the points is a duplicate

    return WD_True; // Yes, they were merged.
}
#else
WT_Boolean WT_Polyline::merge(WT_Drawable const &)
{
    return WD_False;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polyline::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x0C:  // Ctrl-L
                {
                    // Binary, 2 point line, 16-bit relative coords.
                    if (!m_points)
                    {
                        m_count = 2;
                        m_points = new WT_Logical_Point[m_count];
                        if (!m_points)
                            return WT_Result::Out_Of_Memory_Error;

                        m_allocated = m_count; // Need this here so that de_relativize works.
                    }

                    WT_Logical_Point_16    tmp_points[2];

                    WD_CHECK (file.read(2, tmp_points));

                    m_points[0] = tmp_points[0];
                    m_points[1] = tmp_points[1];

                    m_relativized = WD_True;
                    ((WT_Polyline *)this)->de_relativize(file);

                    if (file.heuristics().apply_transform())
                        transform(file.heuristics().transform());
                } break;
            case 'L':
                {
                    // ASCII: 2 point line with absolute coords
                    if (!m_points)
                    {
                        m_count = 2;
                        m_points = new WT_Logical_Point[m_count];
                        if (!m_points)
                            return WT_Result::Out_Of_Memory_Error;

                        m_allocated = m_count; // Need this here so that transform works.
                    }

                    WT_Logical_Box  tmp;
                    WD_CHECK (file.read_ascii(tmp));
                    m_points[0] = tmp.minpt();
                    m_points[1] = tmp.maxpt();

                    // Note: ASCII lines are in absolute coords

                    if (file.heuristics().apply_transform())
                        transform(file.heuristics().transform());
                } break;
            case 'l':
                {
                    // Binary, 2 point line, with 32-bit relative coords.
                    if (!m_points)
                    {
                        m_count = 2;
                        m_points = new WT_Logical_Point[m_count];
                        if (!m_points)
                            return WT_Result::Out_Of_Memory_Error;

                        m_allocated = m_count; // Need this here so that de_relativize works.
                    }

                    WD_CHECK (file.read(2, m_points));

                    m_relativized = WD_True;
                    ((WT_Polyline *)this)->de_relativize(file);

                    if (file.heuristics().apply_transform())
                        transform(file.heuristics().transform());
                } break;
            case 'P':
                {
                    // This is a multi-point polyline, in ASCII.
                    WD_Assert(!file.rendition().fill().fill());

                    WD_CHECK (WT_Point_Set::materialize_ascii(file));
                } break;
            case 'p':
                {
                    // This is a multi-point polyline, 32-bit coords.
                    WD_Assert(!file.rendition().fill().fill());

                    WD_CHECK (WT_Point_Set::materialize(file));
                } break;
            case 0x10:  // Ctrl-P
                {
                    // This is a multi-point polyline, 16-bit relative coords.
                    WD_Assert(!file.rendition().fill().fill());

                    WD_CHECK (WT_Point_Set::materialize_16_bit(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch
        } break;
    case WT_Opcode::Extended_ASCII:
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_allocated = m_count;

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Polyline::update_bounds(WT_File * file)
{
    WT_Drawable::update_bounds(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polyline::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x0C:  // Ctrl-L
                {
                    // This is a two point line, with 16-bit absolute coords.
                    file.skip(sizeof(WT_Logical_Point_16) * 2);
                } break;
            case 'L':
                {
                    WT_Logical_Point tmp;
                    WD_CHECK (file.read_ascii(tmp)); //start
                    WD_CHECK (file.read_ascii(tmp)); //end
                    break;
                } break;
            case 'l':
                {
                    // This is a two point line, with 32-bit absolute coords.
                    file.skip(sizeof(WT_Logical_Point) * 2);
                } break;
            case 0x10:  // Ctrl-P
                {
                    WD_CHECK (WT_Point_Set::skip_operand_16_bit(file));
                } break;
            case 'p':
                {
                    // This is a multi-point polyline.
                    WD_Assert(!file.rendition().fill().fill());
                    WD_CHECK (WT_Point_Set::skip_operand(file));
                } break;
            case 'P':
                {
                    // This is a multi-point polyline.
                    WD_Assert(!file.rendition().fill().fill());
                    WD_CHECK (WT_Point_Set::skip_operand_ascii(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch


        } break;
    case WT_Opcode::Extended_ASCII:
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polyline::process(WT_File & file)
{
    WD_Assert (file.polyline_action());
    return (file.polyline_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polyline::default_process(WT_Polyline &, WT_File &)
{
    return WT_Result::Success;
}
