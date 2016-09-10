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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/polytri.cpp 5     7/05/05 8:57p Bangiav $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Polytriangle::object_id() const
{
    return WT_Object::Polytriangle_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Polytriangle::serialize(WT_File & file) const
{
    WD_Assert(m_count > 2);

    // Make sure we have a legal triangle
    if (m_count < 3)
    {
        return WT_Result::Success; // Drawing a 2 pt. triangle is easy: do nothing!
    }

    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::User_Fill_Pattern_Bit  |
                                        WT_Rendition::User_Hatch_Pattern_Bit |
                                        WT_Rendition::Contrast_Color_Bit;


    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    return file.merge_or_delay_drawable(*this);
}
#else
WT_Result WT_Polytriangle::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Polytriangle::dump(WT_File & file) const
{
    return WT_Point_Set::serialize(file, (WT_Byte) 'T', (WT_Byte)'t', (WT_Byte)0x14);
}
#else
WT_Result   WT_Polytriangle::dump(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Polytriangle::delay(WT_File & file) const
{
    WT_Polytriangle * delayed = new WT_Polytriangle(count(), points(), WD_True);
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    file.set_delayed_drawable(delayed);

    return WT_Result::Success;
}
#else
WT_Result   WT_Polytriangle::delay(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Boolean WT_Polytriangle::merge(WT_Drawable const & new_one)
{
    enum
    {
        LAST_TO_FIRST,
        FIRST_TO_LAST,
        LAST_TO_LAST,
        FIRST_TO_FIRST
    }                       merge_type;

    WD_Assert (new_one.object_id() == Polytriangle_ID);

    WT_Polytriangle const *  current = (WT_Polytriangle const *)(&new_one);   // Cast safe cast to a full Polymarker

    // Are we dealing with triangles or strips of triangles?
    // Single triangles have more possible ways of being merged than
    // strips do.

    if (count() == 3 && current->count() == 3     &&
        ( points()[0] == current->points()[0] ||
          points()[0] == current->points()[1] ||
          points()[0] == current->points()[2]   ) &&
        ( points()[2] == current->points()[0] ||
          points()[2] == current->points()[1] ||
          points()[2] == current->points()[2]   )    )
    {
        // In this special case we have to reorder the first triangles points
        // such that the following tests will see that a merge is possible.
        WT_Logical_Point    tmp;

        tmp = m_points[0];
        m_points[0] = m_points[1];
        m_points[1] = m_points[2];
        m_points[2] = tmp;
    }

    // There is one optimization we aren't using: the case where the old
    // one is a single triangle and the new one is a polytriangle and where
    // the single triangle wont match unless it is reordered.  This would
    // be an infrequent case so lets not worry about it now.


    // See if these two polytriangles can be merged.  Test:
    //  a) first two points of previous polytri match last  two points of next polytri in order
    //  b) last  two points of previous polytri match first two points of next polytri in order
    if      (points()[0] == current->points()[current->count() - 2] &&
             points()[1] == current->points()[current->count() - 1]    )
        merge_type = FIRST_TO_LAST;
    else if (points()[count() - 2] == current->points()[0] &&
             points()[count() - 1] == current->points()[1]    )
        merge_type = LAST_TO_FIRST;
    else if (points()[count() - 2] == current->points()[current->count() - 1] &&
             points()[count() - 1] == current->points()[current->count() - 2]    )
        merge_type = LAST_TO_LAST;
    else if (points()[0] == current->points()[1] &&
             points()[1] == current->points()[0]    )
        merge_type = FIRST_TO_FIRST;
    else if (current->count() == 3)
    {
        WT_Logical_Point    tmp;

        // If the new one is a single trianlge, there are more ways to
        // merge it to the old polytriangle beyond what would normally
        // be possible between two polytriangles.   Check to see if
        // reordering the new triangle's points would help.  If so reorder
        // such that we have a LAST_TO_FIRST or FIRST_TO_LAST relationship.

        // See if we can hook it on to end of strip
        merge_type = FIRST_TO_LAST;

        if      (points()[count() - 1] == current->points()[0])
        {
            if      (points()[count() - 2] == current->points()[1])
            {
                tmp = current->points()[0];
                current->points()[0] = current->points()[1];
                current->points()[1] = tmp;
                merge_type = LAST_TO_FIRST;
            }
            else if (points()[count() - 2] == current->points()[2])
            {
                tmp = current->points()[0];
                current->points()[0] = current->points()[2];
                current->points()[2] = current->points()[1];
                current->points()[1] = tmp;
                merge_type = LAST_TO_FIRST;
            }
            // Else these two polytriangles are not mergeable at end.
        }
        else if (points()[count() - 1] == current->points()[2])
        {
            if      (points()[count() - 2] == current->points()[0])
            {
                tmp = current->points()[1];
                current->points()[1] = current->points()[2];
                current->points()[2] = tmp;
                merge_type = LAST_TO_FIRST;
            }
            else if (points()[count() - 2] == current->points()[1])
            {
                tmp = current->points()[0];
                current->points()[0] = current->points()[1];
                current->points()[1] = current->points()[2];
                current->points()[2] = tmp;
                merge_type = LAST_TO_FIRST;
            }
            // else These two polytriangles are not mergeable at end.
        } // Can we hook to end?

        if (merge_type == FIRST_TO_LAST)
        {
            // We didn't attach it to the end, try the beginning.
            if      (points()[0] == current->points()[0])
            {
                if      (points()[1] == current->points()[1])
                {
                    tmp = current->points()[0];
                    current->points()[0] = current->points()[2];
                    current->points()[2] = current->points()[1];
                    current->points()[1] = tmp;
                }
                else if (points()[1] == current->points()[2])
                {
                    tmp = current->points()[0];
                    current->points()[0] = current->points()[1];
                    current->points()[1] = tmp;
                }
                else
                    return WD_False; // These two polytriangles are not mergeable.
            }
            else if (points()[0] == current->points()[2])
            {
                if      (points()[1] == current->points()[0])
                {
                    tmp = current->points()[0];
                    current->points()[0] = current->points()[1];
                    current->points()[1] = current->points()[2];
                    current->points()[2] = tmp;
                }
                else if (points()[1] == current->points()[1])
                {
                    tmp = current->points()[1];
                    current->points()[1] = current->points()[2];
                    current->points()[2] = tmp;
                }
                else
                    return WD_False; // These two polytriangles are not mergeable.
            }
            else
                return WD_False; // These two polytriangles are not mergeable.
        }
    }
    else
        return WD_False; // These two polytriangles are not mergeable.

    // Yeah!! These two polytriangles are mergable.  Now make space to hold them.

    int needed = count() + current->count() - 2;  // Two of the points are duplicates.

    if (needed > allocated())
    {
        WT_Logical_Point *  new_pts = new WT_Logical_Point[needed * 2];
        if (!new_pts)
            throw WT_Result::Out_Of_Memory_Error;

        if (merge_type == LAST_TO_FIRST || merge_type == LAST_TO_LAST)
            WD_COPY_MEMORY(points(), count() * sizeof(WT_Logical_Point), new_pts);
        else
            WD_COPY_MEMORY(points(), count() * sizeof(WT_Logical_Point), &new_pts[current->count() - 2]);

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
        WT_Logical_Point *  dest = src + current->count() - 2; // Two points are duplicated.

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
        WD_COPY_MEMORY(current->points() + 2, (current->count() - 2) * sizeof(WT_Logical_Point), &m_points[count()]);
        break;
    case FIRST_TO_LAST:
        WD_COPY_MEMORY(current->points(), (current->count() - 2) * sizeof(WT_Logical_Point), points());
        break;
    case LAST_TO_LAST:
        {
            WT_Logical_Point *  src  = current->points() + current->count() - 3;
            WT_Logical_Point *  dest = points() + count();

            while (src >= current->points())
                *dest++ = *src--;
        } break;
    case FIRST_TO_FIRST:
        {
            WT_Logical_Point *  src  = current->points() + current->count() - 1;
            WT_Logical_Point *  dest = points();
            WT_Logical_Point *  term = current->points() + 2;
            while (src >= term)
                *dest++ = *src--;
        } break;
    default:
        throw WT_Result::Internal_Error;
    } // switch

    m_count += current->count() - 2; // Two of the points are duplicated

    return WD_True; // Yes, they were merged.
}
#else
WT_Boolean WT_Polytriangle::merge(WT_Drawable const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polytriangle::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x14: // Ctrl-T
                {
                    WD_CHECK (WT_Point_Set::materialize_16_bit(file));
                } break;
            case 'T':
                {
                    WD_CHECK (WT_Point_Set::materialize_ascii(file));
                } break;
            case 't':
                {
                    WD_CHECK (WT_Point_Set::materialize(file));
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

    m_materialized = WD_True;
    return WT_Result::Success;
}

void WT_Polytriangle::update_bounds(WT_File * file)
{
    WT_Drawable::update_bounds(*this, file);
}

WT_Result WT_Polytriangle::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x14:  // Ctrl-T
                {
                    WD_CHECK (WT_Point_Set::skip_operand_16_bit(file));
                } break;
            case 't':
                {
                    WD_CHECK (WT_Point_Set::skip_operand(file));
                } break;
            default:
                WD_CHECK (WT_Point_Set::skip_operand_ascii(file));
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



WT_Result WT_Polytriangle::process(WT_File & file)
{
    WD_Assert (file.polytriangle_action());
    return (file.polytriangle_action())(*this, file);
}

WT_Result WT_Polytriangle::default_process(WT_Polytriangle &, WT_File &)
{
    return WT_Result::Success;
}
