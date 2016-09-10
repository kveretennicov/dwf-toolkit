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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/units.cpp 1     9/12/04 8:57p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Units::object_id() const
{
    return WT_Object::Units_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Units::object_type() const
{
    return WT_Object::Definition;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Units::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (!file.heuristics().allow_binary_data())
        WD_CHECK(file.write_tab_level());

    WD_CHECK(file.write("(Units "));
    WD_CHECK(m_units.serialize(file));
    WD_CHECK(file.write((WT_Byte)' '));


    WT_Matrix_IO   adjusted_transform(m_transform);

    if (file.heuristics().apply_transform())
        adjusted_transform *= file.heuristics().transform();


    if (file.heuristics().transform().rotation() &&
        file.heuristics().apply_transform_to_units_matrix())
    {
        WT_Matrix_IO    temp;
        adjusted_transform.rotate(temp, file.heuristics().transform().rotation());
        WD_CHECK(temp.serialize(file));
    }
    else
    {
        WD_CHECK(adjusted_transform.serialize(file));
    }
    return file.write((WT_Byte)')');
}
#else
WT_Result WT_Units::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Units::materialize(WT_Opcode const & opcode, WT_File & file)
{

    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK(file.eat_whitespace());
        m_stage = Getting_Units_String;

        // No break
    case Getting_Units_String:
        WD_CHECK(m_units.materialize(file));
        m_stage = Getting_Transform;

        // No break
    case Getting_Transform:
        {
            WT_Matrix_IO oMatrix;
            WD_CHECK(oMatrix.materialize(file));
            m_transform = oMatrix;
            m_stage = Getting_Close_Paren;
        }

        // No break
    case Getting_Close_Paren:
        WD_CHECK(opcode.skip_past_matching_paren(file));
        // Done!
        m_stage = Eating_Initial_Whitespace;
        break;
    default:
        return WT_Result::Internal_Error;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Units::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Units::process(WT_File & file)
{
    WD_Assert(file.units_action());
    return (file.units_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Units::default_process(WT_Units & units, WT_File & file)
{
    file.rendition().drawing_info().units() = units;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Units::set_application_to_dwf_transform(WT_Matrix const & xform)
{
    m_transform = xform;
    m_adjoint_valid = WD_False;
}

///////////////////////////////////////////////////////////////////////////
void WT_Units::set_units(int length, WT_Unsigned_Integer16 const * units)
{
    m_units.set(length,units);
}

///////////////////////////////////////////////////////////////////////////
WT_Point3D WT_Units::transform(WT_Logical_Point const & in_pt)
{
    WT_Point3D  point(in_pt.m_x, in_pt.m_y, 0.0);

    return transform_from_DWF_to_application(point);
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Point WT_Units::transform(WT_Point3D const & in_pt) const
{
    WT_Point3D  result;

    result = transform_from_application_to_DWF(in_pt);

    return WT_Logical_Point((WT_Integer32)result.m_x, (WT_Integer32)result.m_y);
}

///////////////////////////////////////////////////////////////////////////
WT_Point3D WT_Units::transform_from_DWF_to_application(WT_Point3D const & in_pt)
{
    WT_Point3D  out;

    // Going from DWF coordinates to Application coordinates
    if (!m_adjoint_valid)
    {
        m_transform.adjoin(m_adjoint);
        m_adjoint_valid = WD_True;
    }

    m_adjoint.transform(in_pt, out);

    return out;
}

///////////////////////////////////////////////////////////////////////////
WT_Point3D WT_Units::transform_from_application_to_DWF(WT_Point3D const & in_pt) const
{
    WT_Point3D          result;

    m_transform.transform(in_pt, result);

    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Matrix const &  WT_Units::dwf_to_application_adjoint_transform()
{
    if (!m_adjoint_valid)
    {
        m_transform.adjoin(m_adjoint);
        m_adjoint_valid = WD_True;
    }

    return m_adjoint;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Units::operator==( WT_Units const & u ) const
{
    if (m_units != u.m_units ||
        application_to_dwf_transform() != u.application_to_dwf_transform())
        return WD_False;
    return WD_True;
}
