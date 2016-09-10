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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/ellipse.cpp 7     7/05/05 8:56p Bangiav $

#include "whiptk/pch.h"
#include "whiptk/wversion.h"

const double PI = 3.14159265358979323846;

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Filled_Ellipse::object_id() const
{
    return Filled_Ellipse_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Outline_Ellipse::object_id() const
{
    return Outline_Ellipse_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Filled_Ellipse::serialize(WT_File & file) const
{
    return WT_Ellipse::serialize (file, WD_True);
}
#else
WT_Result WT_Filled_Ellipse::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Outline_Ellipse::serialize(WT_File & file) const
{
    return WT_Ellipse::serialize (file, WD_False);
}
#else
WT_Result WT_Outline_Ellipse::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Ellipse::serialize(WT_File & file, WT_Boolean filled) const
{
    WD_CHECK (file.dump_delayed_drawable());

    // TODO: Check rendition incarnation

    // Attributes that Filled_Ellipses care about:
    //        color, line weight, line cap, line join,
    //        line pattern, fill mode, visibility, URL
    //        layer

    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    | // strange one; move below?
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit;

    if (filled)
    {
        // do not move these bits into the "if" below, as then the only
        // time the pattern will sync is on a Fill state change.
        parts_to_sync |= WT_Rendition::User_Fill_Pattern_Bit  |
                         WT_Rendition::User_Hatch_Pattern_Bit |
                         WT_Rendition::Contrast_Color_Bit;

        if (!file.rendition().fill().fill())
        {
            file.desired_rendition().fill() = WD_True;
            parts_to_sync |= WT_Rendition::Fill_Bit;
        }
    }
    else
    {
        parts_to_sync |= WT_Rendition::Line_Weight_Bit  |
                         WT_Rendition::Pen_Pattern_Bit  |
                         WT_Rendition::Line_Pattern_Bit |
                         WT_Rendition::Dash_Pattern_Bit |
                         WT_Rendition::Line_Style_Bit;

        if (file.rendition().fill().fill())
        {
            file.desired_rendition().fill() = WD_False;
            parts_to_sync |= WT_Rendition::Fill_Bit;
        }
    }

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
        ((WT_Filled_Ellipse *)this)->transform(file.heuristics().transform());  // Cast "this" from const to alterable

    if (file.heuristics().allow_binary_data())
    {
        WT_Logical_Point    relpos = file.update_current_point(m_position);

        // Binary output
        if (m_major != m_minor)
        {
            // output as Ellipse
            WD_CHECK (file.write ((WT_Byte)'e'));
            WD_CHECK (file.write (1, &relpos));
            WD_CHECK (file.write (m_major));
            WD_CHECK (file.write (m_minor));
            WD_CHECK (file.write (m_start));
            WD_CHECK (file.write ((WT_Unsigned_Integer16)(m_end & 0xFFFF)));
            return    file.write (m_tilt);
        }
        else if (m_start != m_end)
        {
            // Output a 32 bit partial circle

            // merge any ellipse tilt into the circle angles
            long    start = ((long)m_start + (long)m_tilt) & 0x0000FFFF;
            long    end   = ((long)m_end   + (long)m_tilt) & 0x0000FFFF;

            WD_CHECK (file.write ((WT_Byte)0x92));
            WD_CHECK (file.write (1, &relpos));
            WD_CHECK (file.write (m_major));
            WD_CHECK (file.write ((WT_Unsigned_Integer16)start));
            return    file.write ((WT_Unsigned_Integer16)end);
        }
        else if (WD_fits_in_short (relpos.m_x) && WD_fits_in_short (relpos.m_y) &&
                 WD_fits_in_short (m_major))
        {
            // Output a 16 bit relative full circle
            WT_Logical_Point_16 tmp_point;
            tmp_point = relpos;

            WD_CHECK (file.write ((WT_Byte)0x12));
            WD_CHECK (file.write (1, &tmp_point));
            return    file.write ((WT_Integer16)m_major);
        }
        else
        {
            // Output a 32 bit relative full circle
            WD_CHECK (file.write ((WT_Byte)'r'));
            WD_CHECK (file.write (1, &relpos));
            return    file.write (m_major);
        }
    }  // allow binary data
    else
    {
        // ASCII only output -----------------------
        WD_CHECK (file.write_geom_tab_level());

        if (m_major != m_minor)
        {
            // output as Ellipse in ASCII

            if ((m_start != m_end) || m_tilt)
            {
                // Full description of an ellipse
                WD_CHECK (file.write ("(Ellipse "));
                WD_CHECK (file.write_ascii (m_position));
                WD_CHECK (file.write ((WT_Byte) ' '));

                WD_CHECK (file.write_ascii (m_major));
                WD_CHECK (file.write ((WT_Byte) ','));
                WD_CHECK (file.write_ascii (m_minor));
                WD_CHECK (file.write ((WT_Byte) ' '));

                WD_CHECK (file.write_ascii (m_start));
                WD_CHECK (file.write ((WT_Byte) ','));
                WD_CHECK (file.write_ascii (m_end));
                WD_CHECK (file.write ((WT_Byte) ' '));

                WD_CHECK (file.write_ascii (m_tilt));

                return file.write((WT_Byte) ')');
            }
            else
            {
                // Basic description of an ellipse
                WD_CHECK (file.write ("E "));
                WD_CHECK (file.write_ascii (1, &m_position));
                WD_CHECK (file.write ((WT_Byte) ' '));

                WD_CHECK (file.write_ascii (m_major));
                WD_CHECK (file.write ((WT_Byte) ','));
                return    file.write_ascii (m_minor);
            }
        }
        else
        {
            // merge any ellipse tilt into the circle angles
            long    start = ((long)m_start + (long)m_tilt) & 0x0000FFFF;
            long    end   = ((long)m_end   + (long)m_tilt) & 0x0000FFFF;

            // Output a circle in ASCII
            if (start != end)
            {
                // Full description of the circle/arc
                WD_CHECK (file.write ("(Circle "));
                WD_CHECK (file.write_ascii (m_position));
                WD_CHECK (file.write ((WT_Byte) ' '));
                WD_CHECK (file.write_ascii (m_major));

                WD_CHECK (file.write ((WT_Byte) ' '));
                WD_CHECK (file.write_ascii (start));
                WD_CHECK (file.write ((WT_Byte) ','));
                WD_CHECK (file.write_ascii (end));

                return file.write((WT_Byte) ')');
            }
            else
            {
                // Basic circle
                WD_CHECK (file.write ("R "));
                WD_CHECK (file.write_ascii (m_position));
                WD_CHECK (file.write ((WT_Byte) ' '));
                return    file.write_ascii (m_major);
            }
        }
    }
}
#else
WT_Result WT_Ellipse::serialize(WT_File &, WT_Boolean) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
bool containsAngle(double angle, double angleStart, double angleEnd)
{
    // get the orientation
    bool isCCW = angleStart < angleEnd;

    // put the supplied parameter value on the extent side of the start angle
    if (isCCW)
    {
        while (angle < angleStart)
            angle += 2.0*PI;
        while (angle- 2.0*PI > angleStart)
            angle -= 2.0*PI;
    }
    else
    {
        while (angle > angleStart)
            angle -= 2.0*PI;
        while (angle+2.0*PI < angleStart)
            angle += 2.0*PI;
    }

    // do the range check
    if (isCCW)
    {
        // normal orientation
        if (angle >= angleStart && angle <= angleEnd)
            return true;
    }
    else
    {
        // reversed orientation
        if (angle >= angleEnd && angle <= angleStart)
            return true;
    }

    // not in range
    return false;
}

///////////////////////////////////////////////////////////////////////////
void WT_Ellipse::update_bounds(WT_File * file)
{
    //Bounds algorithm lifted and ported from Kona

    double maj = major();
    double min = minor();

    double ang = tilt_radian();
    double csa = cos(ang);
    double sna = sin(ang);

    double ax = maj * csa;
    double bx = min * sna;
    double ay = maj * sna;
    double by = min * csa;
    double xmax = sqrt(ax*ax + bx*bx);
    double ymax = sqrt(ay*ay + by*by);

    double asValue = start_radian();;
    double css = cos(asValue);
    double sns = sin(asValue);
    double xs  = ax*css - bx*sns;
    double ys  = ay*css + by*sns;

    double ae  = end_radian();
    double cse = cos(ae);
    double sne = sin(ae);
    double xe  = ax*cse - bx*sne;
    double ye  = ay*cse + by*sne;

    WT_Drawable::update_bounds(WT_Logical_Point((int)xs+m_position.m_x,(int)ys+m_position.m_y), file);
    WT_Drawable::update_bounds(WT_Logical_Point(((int)xs)+m_position.m_x+1,((int)ys)+m_position.m_y+1), file);
    WT_Drawable::update_bounds(WT_Logical_Point((int)xe+m_position.m_x,(int)ye+m_position.m_y), file);
    WT_Drawable::update_bounds(WT_Logical_Point(((int)xe)+m_position.m_x+1,((int)ye)+m_position.m_y+1), file);

    double asnorm = atan2(sns, css);
    double aenorm = ae + (asnorm - asValue);

    double txmax = atan2(-min*sna, maj*csa);
    double txmin = txmax - PI;
    if (txmin <= -PI)
        txmin += 2.0*PI;

    if (containsAngle(txmax, asnorm, aenorm))
        WT_Drawable::update_bounds(WT_Logical_Point(((int)xmax)+m_position.m_x+1, ((int)ys)+m_position.m_y+1), file);
    if (containsAngle(txmin, asnorm, aenorm))
        WT_Drawable::update_bounds(WT_Logical_Point((int)-xmax+m_position.m_x, (int)ys+m_position.m_y), file);

    double tymax = atan2(min*csa, maj*sna);
    double tymin = tymax - PI;
    if (tymin <= -PI)
        tymin += 2.0*PI;

    if (containsAngle(tymax, asnorm, aenorm))
        WT_Drawable::update_bounds(WT_Logical_Point(((int)xs)+m_position.m_x+1, ((int)ymax)+m_position.m_y+1), file);
    if (containsAngle(tymin, asnorm, aenorm))
        WT_Drawable::update_bounds(WT_Logical_Point((int)xs+m_position.m_x, (int)-ymax+m_position.m_y), file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Ellipse::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Logical_Point    tmp_point;

    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 'E':
                {
                    // Ellipse, single-byte full ASCII
                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read_ascii (m_position));
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read_ascii (tmp_point));
                                        m_major = tmp_point.m_x;
                                        m_minor = tmp_point.m_y;
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }

                    // Note: for full ASCII we *don't* de-relativize since we already have absolute coords.
                } break;
            case 'e':
                {
                    // Ellipse, binary, long relative coordinates
                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read (1, &m_position));
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read (m_major));
                                        m_stage = Get_Minor;
                    case Get_Minor:     WD_CHECK (file.read (m_minor));
                                        m_stage = Get_Start;
                    case Get_Start:     WD_CHECK (file.read (m_start));
                                        m_stage = Get_End;
                    case Get_End:       WT_Unsigned_Integer16   tmp;
                                        WD_CHECK (file.read (tmp));
                                        m_end = tmp;
                                        m_stage = Get_Tilt;
                    case Get_Tilt:      WD_CHECK (file.read (m_tilt));
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }

                    // convert from relative to absolute, transform if appropriate
                    m_position = file.de_update_current_point(m_position);
                } break;
            case 'R':
                {
                    // Basic Circle, Ascii
                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read_ascii (m_position));
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read_ascii (m_major));
                                        m_minor = m_major;
                                        m_start = 0;
                                        m_end = 0;
                                        m_tilt = 0;
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }
                    // Note: for full ASCII we *don't* de-relativize since we already have absolute coords.
                } break;
            case 'r':
                {
                    // Full Circle, long relative coordinates
                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read (1, &m_position));
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read (m_major));
                                        m_minor = m_major;
                                        m_start = 0;
                                        m_end = 0x00010000;
                                        m_tilt = 0;
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }

                    // convert from relative to absolute, transform if appropriate
                    m_position = file.de_update_current_point(m_position);
                } break;
            case 0x12:  // Ctrl-R
                {
                    // Full Circle, short relative coordinates
                    WT_Logical_Point_16     position;
                    WT_Unsigned_Integer16   radius;

                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read (1, &position));
                                        m_position = position;
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read (radius));
                                        m_major = m_minor = radius;
                                        m_start = 0;
                                        m_end = 0x00010000;
                                        m_tilt = 0;
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }

                    // convert from relative to absolute, transform if appropriate
                    m_position = file.de_update_current_point(m_position);
                } break;
            case 0x92:  // X-Ctrl-R
                {
                    // Partial or Full Circle, long relative coordinates, short angles
                    switch (m_stage)
                    {
                    case Get_Position:  WD_CHECK (file.read (1, &m_position));
                                        m_stage = Get_Major;
                    case Get_Major:     WD_CHECK (file.read (m_major));
                                        m_minor = m_major;
                                        m_stage = Get_Start;
                    case Get_Start:     WD_CHECK (file.read (m_start));
                                        m_stage = Get_End;
                    case Get_End:       WT_Unsigned_Integer16   tmp;
                                        WD_CHECK (file.read (tmp));
                                        m_end = tmp;
                                        m_stage = Completed;
                                        break;
                    default:
                        return WT_Result::Internal_Error;
                    }

                    // convert from relative to absolute, transform if appropriate
                    m_position = file.de_update_current_point(m_position);
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch
        } break;
    case WT_Opcode::Extended_ASCII:
        {
            if (!strcmp((char const *) opcode.token(), "(Circle"))
            {
                // Partial or Full Circle, absolute coordinates, short angles
                switch (m_stage)
                {
                case Get_Position:  WD_CHECK (file.read_ascii (m_position));
                                    m_stage = Get_Major;
                case Get_Major:     WD_CHECK (file.read_ascii (m_major));
                                    m_minor = m_major;
                                    m_stage = Get_Start;
                case Get_Start:     WD_CHECK (file.read_ascii (tmp_point));
                                    if (tmp_point.m_x < 0 || tmp_point.m_y < 0)
                                        return WT_Result::Corrupt_File_Error;
                                    m_start = (WT_Unsigned_Integer16)(tmp_point.m_x & 0XFFFF);
                                    m_end = tmp_point.m_y & 0x0001FFFF;
                                    m_stage = Get_Close_Paren;
                case Get_Close_Paren:
                                    WD_CHECK (opcode.skip_past_matching_paren(file));
                                    m_stage = Completed;
                                    break;
                default:
                    return WT_Result::Internal_Error;
                }
                // Note: for full ASCII we *don't* de-relativize since we already have absolute coords.
            }
            else if (!strcmp((char const *) opcode.token(), "(Ellipse"))
            {
                // Ellipse, single-byte full ASCII

                switch (m_stage)
                {
                case Get_Position:  WD_CHECK (file.read_ascii (m_position));
                                    m_stage = Get_Major;
                case Get_Major:     WD_CHECK (file.read_ascii (tmp_point));
                                    m_major = tmp_point.m_x;
                                    m_minor = tmp_point.m_y;
                                    m_stage = Get_Start;
                case Get_Start:     WD_CHECK (file.read_ascii (tmp_point));
                                    if (tmp_point.m_x < 0 || tmp_point.m_y < 0)
                                        return WT_Result::Corrupt_File_Error;
                                    m_start = (WT_Unsigned_Integer16)(tmp_point.m_x & 0XFFFF);
                                    m_end   = tmp_point.m_y & 0x0001FFFF;
                                    m_stage = Get_Tilt;
                case Get_Tilt:      WD_CHECK (file.read_ascii (m_tilt));
                                    m_stage = Get_Close_Paren;
                case Get_Close_Paren:
                                    WD_CHECK (opcode.skip_past_matching_paren(file));
                                    m_stage = Completed;
                                    break;
                default:
                    return WT_Result::Internal_Error;
                }
                // Note: for full ASCII we *don't* de-relativize since we already have absolute coords.
            }
            else
                return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    m_materialized = WD_True;

    if (file.rendition().drawing_info().decimal_revision() <= WHIP20_DWF_FILE_VERSION)
    {
        // Note that in old versions, if m_start == m_end this is
        // a null ellipse/circle that shouldn't be drawn.  In this case
        // we leave both m_start and m_end the same since Heidi knows not
        // to draw such a case.
        if (m_start == m_end)
            return WT_Result::Success;

        // In old versions, a complete circle was when m_end was one less than m_start.
        // Adjust for the current definition which is that a complete circle is when
        // they are the same value.  (If you want a null circle, just don't put it
        // in the DWF file, stupid!).
        if (m_end < 0x00010000)
            m_end++;
    }

    if (m_end <= m_start)
        m_end += 0x00010000;

    return WT_Result::Success;
}

WT_Result WT_Ellipse::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 'E':
                {
                    WT_Logical_Point tmp_pt;
                    WD_CHECK(file.read_ascii(tmp_pt));
                    WD_CHECK(file.read_ascii(tmp_pt));
                } break;
            case 'e':
                {
                    // This is an ellipse, with 32-bit relative coords, 32-bit axes, 16-bit angles
                    file.skip(2 * sizeof(WT_Integer32) +
                              2 * sizeof(WT_Unsigned_Integer32) +
                              3 * sizeof(WT_Unsigned_Integer16));
                } break;
            case 'R':
                {
                    WT_Integer32 tmp;
                    WT_Logical_Point tmp_pt;
                    WD_CHECK(file.read_ascii(tmp_pt));
                    WD_CHECK(file.read_ascii(tmp));
                } break;
            case 'r':
                {
                    // This is a circle, with 32-bit relative coords, 32-bit radius
                    file.skip(2 * sizeof(WT_Integer32) +
                              sizeof(WT_Unsigned_Integer32));
                } break;
            case 0x12:  // Ctrl-R
                {
                    // This is a circle, with 16-bit relative coords, 16-bit radius
                    file.skip(2 * sizeof(WT_Integer16) +
                              sizeof(WT_Unsigned_Integer16));
                } break;
            case 0x92:  // X-Ctrl-R
                {
                    // This is a circle, with 32-bit relative coords, 32-bit radius, 16-bit angles
                    file.skip(2 * sizeof(WT_Integer32) +
                              sizeof(WT_Unsigned_Integer32) +
                              2 * sizeof(WT_Unsigned_Integer16));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch


        } break;
    case WT_Opcode::Extended_ASCII:
        WD_CHECK(opcode.skip_past_matching_paren(file));
        break;
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Ellipse::transform(WT_Transform const & transform)
{
    if (!m_transformed)
    {
        // m_position.m_x = (WT_Integer32)((m_position.m_x + transform.m_translate.m_x) * transform.m_x_scale);
        // m_position.m_y = (WT_Integer32)((m_position.m_y + transform.m_translate.m_y) * transform.m_y_scale);

        // m_position.m_x = (WT_Integer32)((m_position.m_x * transform.m_x_scale) + transform.m_translate.m_x);
        // m_position.m_y = (WT_Integer32)((m_position.m_y * transform.m_y_scale) + transform.m_translate.m_y);

        WT_Logical_Point    orig_pt(m_position);

        m_position = orig_pt * transform; // we should use *=, but there seems to be a problem?

        m_major = (WT_Unsigned_Integer32)(m_major * transform.m_x_scale);
        m_minor = (WT_Unsigned_Integer32)(m_minor * transform.m_y_scale);

        if (m_major < 0)
            m_major = - m_major;
        if (m_minor < 0)
            m_minor = - m_minor;

        m_tilt  = (WT_Unsigned_Integer16)(
                  ( (long)m_tilt +
                    (long)WD_90_DEGREES_AS_SHORT * (transform.rotation() / 90)
                  ) & 0x0000FFFF);

        // TODO: note that we can simplify some ellipses by incorporating the tilt in and
        // exchanging the major and minor axes or by rotating the start and end points.

        m_transformed = WD_True;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Filled_Ellipse::process(WT_File & file)
{
    WD_Assert (file.filled_ellipse_action());
    return (file.filled_ellipse_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Filled_Ellipse::default_process(WT_Filled_Ellipse &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Outline_Ellipse::process(WT_File & file)
{
    WD_Assert (file.outline_ellipse_action());
    return (file.outline_ellipse_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Outline_Ellipse::default_process(WT_Outline_Ellipse &, WT_File &)
{
    return WT_Result::Success;
}
