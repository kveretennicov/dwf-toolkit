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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/inked_area.cpp 3     5/09/05 12:42a Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Inked_Area::object_id() const
{
    return WT_Object::Inked_Area_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::set_inked_area(WT_Logical_Point const * bounds)
{
    return set(bounds);
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Point const * WT_Inked_Area::get_inked_area() const
{
    return bounds();
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Inked_Area::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (file.heuristics().apply_transform())
        ((WT_Inked_Area *)this)->transform(file.heuristics().transform());

    // ASCII output
    if (m_bounds)
    {
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(InkedArea "));
        file.write_ascii(4, m_bounds);
        WD_CHECK (file.write((WT_Byte) ')'));
    } // if (m_bounds)

    return WT_Result::Success;
}
#else
WT_Result WT_Inked_Area::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::materialize(WT_Opcode const & opcode,
                                     WT_File & file)
{
    if (opcode.type() == WT_Opcode::Extended_ASCII)
    {
        // ASCII
        switch (m_stage)
        {
        case Getting_Operand:
            if (!m_bounds)
            {
                m_bounds = new WT_Logical_Point[4];
                if (!m_bounds)
                    return WT_Result::Out_Of_Memory_Error;
            }

            m_materialization_counter = 0;
            m_stage = Getting_Bounds;

        case Getting_Bounds:

            while (m_materialization_counter < 4)
            {
                WD_CHECK(file.read_ascii(m_bounds[m_materialization_counter]));
                m_materialization_counter++;
            }

            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(opcode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
        } // switch (m_stage)
    } // ASCII
    else {
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::set(WT_Logical_Point const * bounds)
{
    delete []m_bounds;

    if (bounds)
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            return WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds[0];
        m_bounds[1] = bounds[1];
        m_bounds[2] = bounds[2];
        m_bounds[3] = bounds[3];
    }
    else
        m_bounds = WD_Null;

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Inked_Area::transform(WT_Transform const & transform)
{
    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = tmp_pts[0] * transform;
        m_bounds[1] = tmp_pts[1] * transform;
        m_bounds[2] = tmp_pts[2] * transform;
        m_bounds[3] = tmp_pts[3] * transform;
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_Inked_Area::relativize(WT_File & file)
{
    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = file.update_current_point(tmp_pts[0]);
        m_bounds[1] = file.update_current_point(tmp_pts[1]);
        m_bounds[2] = file.update_current_point(tmp_pts[2]);
        m_bounds[3] = file.update_current_point(tmp_pts[3]);
    }
}
#else
void WT_Inked_Area::relativize(WT_File &)
{
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_Inked_Area::de_relativize(WT_File & file)
{
    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = file.de_update_current_point(tmp_pts[0]);
        m_bounds[1] = file.de_update_current_point(tmp_pts[1]);
        m_bounds[2] = file.de_update_current_point(tmp_pts[2]);
        m_bounds[3] = file.de_update_current_point(tmp_pts[3]);
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Inked_Area::WT_Inked_Area(WT_Inked_Area const & bounds) throw(WT_Result)
    : WT_Attribute()
    , m_materialization_counter(0)
{
    if (bounds.bounds())
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            throw WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds.bounds()[0];
        m_bounds[1] = bounds.bounds()[1];
        m_bounds[2] = bounds.bounds()[2];
        m_bounds[3] = bounds.bounds()[3];
    }
    else
        m_bounds = WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Inked_Area::WT_Inked_Area(WT_Logical_Point const * pos) throw(WT_Result)
	:m_materialization_counter(0)
{
	if(!pos)
		m_bounds = WD_Null;
	else
	{
		m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            throw WT_Result::Out_Of_Memory_Error;
		m_bounds[0] = pos[0];
		m_bounds[1] = pos[1];
		m_bounds[2] = pos[2];
		m_bounds[3] = pos[3];
	}
	
}

///////////////////////////////////////////////////////////////////////////
WT_Inked_Area const & WT_Inked_Area::operator=(WT_Inked_Area const & bounds) throw(WT_Result)
{
    delete []m_bounds;
    m_bounds = WD_Null;

    if (bounds.bounds())
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            throw WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds.bounds()[0];
        m_bounds[1] = bounds.bounds()[1];
        m_bounds[2] = bounds.bounds()[2];
        m_bounds[3] = bounds.bounds()[3];
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Inked_Area::~WT_Inked_Area()
{
    delete []m_bounds;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::process(WT_File & file)
{
    WD_Assert(file.inked_area_action());
    return (file.inked_area_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Inked_Area::default_process(WT_Inked_Area & item, WT_File & file)
{
    file.rendition().rendering_options().inked_area() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Inked_Area::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().rendering_options().inked_area() )
    {
      file.rendition().rendering_options().inked_area() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Inked_Area::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Inked_Area::operator== (WT_Attribute const & attrib) const
{
    if(attrib.object_id() != Inked_Area_ID)
        return WD_False;

    if (attrib.object_id() == Inked_Area_ID)
    {
        const WT_Logical_Point *bounds;
        bounds = ((WT_Inked_Area const &)attrib).bounds();
        if ((bounds == WD_Null) != (m_bounds == WD_Null))
            return WD_False;
        if (bounds != WD_Null)
        {
            for(int i=0; i<4; i++)
                if(m_bounds[i] == bounds[i])
                    continue;
                else
                    return WD_False;
        }
    }
    return WD_True;
}
