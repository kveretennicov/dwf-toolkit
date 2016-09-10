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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/viewport.cpp 6     5/09/05 12:41a Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Viewport::WT_Viewport(
        WT_File &                   file,
        char const *                name,
        int                         count,
        WT_Logical_Point const *    points,
        WT_Boolean                  ) throw(WT_Result)
    : m_stage(Eating_Initial_Whitespace)
    , m_temp_point_set(WD_Null)
    , m_temp_opcode(WD_Null)
    , m_fields_defined(0)
    , m_name(static_cast<int>(strlen(name)),name)
    , m_incarnation(file.next_incarnation())
{
    m_contour_set = new WT_Contour_Set(file, count, points);
    if (!m_contour_set)
        throw WT_Result::Out_Of_Memory_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Viewport::WT_Viewport(
        WT_File &                       file,
        WT_Unsigned_Integer16 const *   name,
        int                             count,
        WT_Logical_Point const *        points,
        WT_Boolean                      ) throw(WT_Result)
    : m_stage(Eating_Initial_Whitespace)
    , m_temp_point_set(WD_Null)
    , m_temp_opcode(WD_Null)
    , m_fields_defined(0)
    , m_name(WT_String::wcslen(name),name)
    , m_incarnation(file.next_incarnation())
{
    m_contour_set = new WT_Contour_Set(file, count, points);
    if (!m_contour_set)
        throw WT_Result::Out_Of_Memory_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Viewport::WT_Viewport(
        WT_File &                       file,
        WT_String const &               name,
        WT_Contour_Set const &          boundary,
        WT_Boolean                      bCopy ) throw(WT_Result)
    : m_stage(Eating_Initial_Whitespace)
    , m_temp_point_set(WD_Null)
    , m_temp_opcode(WD_Null)
    , m_fields_defined(0)
    , m_name(name)
    , m_incarnation(file.next_incarnation())
{
    m_contour_set = new WT_Contour_Set(boundary, bCopy);
    if (!m_contour_set)
        throw WT_Result::Out_Of_Memory_Error;
}

///////////////////////////////////////////////////////////////////////////
// Copy constructor
WT_Viewport::WT_Viewport(WT_Viewport const & vport) throw(WT_Result)
    : WT_Attribute()
    , m_stage(Eating_Initial_Whitespace)
    , m_contour_set(WD_Null)
    , m_temp_point_set(WD_Null)
    , m_temp_opcode(WD_Null)
    , m_fields_defined(0)
{
	WT_Result res = set(vport, WD_True);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_Viewport::~WT_Viewport()
{
    delete m_contour_set;
    delete m_temp_point_set;
    delete m_temp_opcode;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Viewport::object_id() const
{
    return WT_Attribute::Viewport_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Viewport::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());

    // Viewport is an ASCII opcode that contains a contour set or a point
    // set.  We need to force the contained opcodes to also serialize themselves
    // in ASCII mode...
    WT_Boolean  allow_binary    = file.heuristics().allow_binary_data();
    file.heuristics().set_allow_binary_data(WD_False);

    // Find out which parts of the viewport definition have changed.
    WT_Unsigned_Integer16   parts_changed(0);

    if (viewport_units() != file.rendition().viewport().viewport_units())
        parts_changed |= VIEWPORT_UNITS_BIT;

    // Output required parts of this Attribute object.
    WD_CHECK (file.write("(Viewport"));
    if( !m_contour_set || !m_contour_set->contours())
        goto Finish;
    WD_CHECK (file.write((WT_Byte)' '));
    WD_CHECK (m_name.serialize(file));

    if (file.heuristics().target_version() < REVISION_WHEN_CONTOUR_SET_USED_FOR_VIEWPORTS)
    {
        // Downcast and possibly lose part of the viewport, since there might have
        // been multiple regions to the contour.  However, old files couldn't handle
        // that case.
        WT_Point_Set    temp_point_set( m_contour_set->counts()[0],
                                        m_contour_set->points(),
                                        WD_False);

        WD_CHECK ( temp_point_set.serialize( file,
                                             (WT_Byte)' ',
                                             (WT_Byte)' ',
                                             (WT_Byte)0x10));  // shouldn't that be ' ' rather than 0x10??
    }
    else
    {
        WD_CHECK(file.write((WT_Byte)' '));
        WD_CHECK(m_contour_set->serialize(file, WD_True)); // True indicates it will be embeded in this opcode (so it doesn't sync attributes)
        WD_CHECK(file.write((WT_Byte)' '));
    }

    // Output only the viewport options that have changed.
    if (parts_changed & VIEWPORT_UNITS_BIT)
    {   // WT_Units serialize to their own tab level;
        // so ensure that they conform to WT_Viewport's
        // tab level.
        file.set_tab_level(file.tab_level() + 5);
        WD_CHECK (viewport_units().serialize(*this,file));
        file.set_tab_level(file.tab_level() - 5); // restore tab level
    }

Finish:
    WD_CHECK (file.write(")"));
    // Restore the orig. state we had bashed on
    file.heuristics().set_allow_binary_data(allow_binary);
    return WT_Result::Success;
}
#else
///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Viewport::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() != WT_Attribute::Viewport_ID )
        return WD_False;
        
    WT_Viewport const & other_vp = (WT_Viewport const &)attrib;

    if (m_incarnation == other_vp.m_incarnation)
        return WD_True;

    if (m_name != other_vp.m_name || viewport_units() != other_vp.viewport_units())
        return WD_False;

    if (m_contour_set && other_vp.m_contour_set)
    {
        // Both viewports have a contour defined
        if (  !(*m_contour_set == *other_vp.m_contour_set) )
            return WD_False;
    }
    else
    {
        // At least one viewport doesn't have a contour defined,
        // for the two contours to be equal, they must *both* have undefined
        // contours.
        if (m_contour_set || other_vp.m_contour_set)
            return WD_False;
    }


    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Viewport::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) || 
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().viewport())
    {
        WD_CHECK (serialize(file));        
		file.rendition().viewport() = *this;
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Viewport::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::materialize(WT_Opcode const & opcode, WT_File & file)
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    m_incarnation = file.next_incarnation();

    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        // Clean out anything old before reading in something new...
        delete m_temp_opcode;
        delete m_contour_set;
        m_temp_opcode = WD_Null;
        m_contour_set = WD_Null;
        m_stage = Checking_For_Attribute_End;
        // No Break Here

    case Checking_For_Attribute_End:
    {
        WT_Byte a_byte;
        WD_CHECK (file.read(a_byte));
        file.put_back(a_byte);
        if (a_byte == ')')
        {   m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Getting_Viewport_Name;
        // No Break Here
    }

    case Getting_Viewport_Name:
        WD_CHECK (m_name.materialize(file));
        m_stage = Getting_Viewport_Polygon;
        // No Break Here

    case Getting_Viewport_Polygon:
    {
        //check for attribute end (might just be re-activating a viewport)
        WT_Byte a_byte;
        WD_CHECK (file.read(a_byte));
        file.put_back(a_byte);
        if (a_byte == ')')
        {   m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }

        if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_CONTOUR_SET_USED_FOR_VIEWPORTS)
        {
            // Since we might run out of data, the temp point set we are using
            // must be a class member since a stack variable would get lost
            // upon running out of data from the stream.
            if (!m_temp_point_set)
            {
                m_temp_point_set = new WT_Point_Set;
                if (!m_temp_point_set)
                    return WT_Result::Out_Of_Memory_Error;
            }

            WD_CHECK (m_temp_point_set->materialize_ascii(file));
            // Convert this old point set to the newer Contour set definition of the viewport boundary
            delete m_contour_set;
            m_contour_set = new WT_Contour_Set(file, m_temp_point_set->count(), m_temp_point_set->points());
            if (!m_contour_set)
                return WT_Result::Out_Of_Memory_Error;

            // Now free the memory for the point set:
            delete m_temp_point_set;
            m_temp_point_set = WD_Null;

            m_stage = Getting_Viewport_Optioncode;
            goto Get_Next_Viewport_Optioncode;
        }
        else
        {
            // We have a newer file with a contour as a viewport boundary.
            // Note that the temp opcode needs to be a member since a stack variable
            // would go out of scope if the stream were to run out of data.
            if (!m_temp_opcode)
            {
                m_temp_opcode = new WT_Opcode;
                if (!m_temp_opcode)
                    return WT_Result::Out_Of_Memory_Error;
            }

            WD_CHECK(m_temp_opcode->get_opcode(file));

            WT_Result   reslt;
            WT_Object *  temp_object = m_temp_opcode->object_from_opcode(file.rendition(), reslt, file);
            if (!temp_object || temp_object->object_id() != Contour_Set_ID ||
                reslt != WT_Result::Success)
                return WT_Result::Corrupt_File_Error;

            delete temp_object;
            m_stage = Materialize_Contour_Set;
        }
        // No Break Here
    }

    case Materialize_Contour_Set:
        if (!m_contour_set)
        {
            m_contour_set = new WT_Contour_Set();
            if (!m_contour_set)
                return WT_Result::Out_Of_Memory_Error;
        }
        WD_CHECK(m_contour_set->materialize(*m_temp_opcode, file));

        delete m_temp_opcode;
        m_temp_opcode = WD_Null;

        // No Break here

Get_Next_Viewport_Optioncode:
    case Getting_Viewport_Optioncode:
        WD_CHECK (m_optioncode.get_optioncode(file));
        if (m_optioncode.type() == WT_Opcode::Null_Optioncode)
        {
            m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Materializing_Viewport_Option;
        // No Break Here

    case Materializing_Viewport_Option:
        switch (m_optioncode.option_id())
        {
        case WT_Viewport_Optioncode::Viewport_Units_Option:
            m_fields_defined |= VIEWPORT_UNITS_BIT;
            WD_CHECK (m_viewport_units.materialize(*this,m_optioncode,file));
            break;
        case WT_Viewport_Optioncode::Viewport_Unknown_Option:
            WD_CHECK (m_optioncode.skip_past_matching_paren(file));
            break;
        default:
            return WT_Result::Internal_Error;
        } // switch (m_optioncode.option_id())
        m_stage = Getting_Viewport_Optioncode;
        goto Get_Next_Viewport_Optioncode;
end_ascii:
    case Eating_End_Whitespace:
        WD_CHECK (opcode.skip_past_matching_paren(file));
        m_stage = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }

    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::process(WT_File & file)
{
    WD_Assert (file.viewport_action());
    return (file.viewport_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::default_process(WT_Viewport & item, WT_File & file)
{
    file.rendition().viewport() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Viewport & WT_Viewport::operator=(WT_Viewport const & viewport)
{
    if (m_contour_set)
        delete m_contour_set;
    if (m_temp_point_set)
        delete m_temp_point_set;
    if (m_temp_opcode)
        delete m_temp_opcode;
    m_contour_set    = WD_Null;
    m_temp_point_set = WD_Null;
    m_temp_opcode    = WD_Null;
    set(viewport,WD_True);
    return *this;
}


///////////////////////////////////////////////////////////////////////////
void WT_Viewport::set(
    WT_String const & name) 
{
    m_name = name;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::set(
    WT_Contour_Set const * boundary, WT_Boolean bCopy) 
{
    if (m_contour_set)
        delete m_contour_set;

    if (boundary != WD_Null)
    {
        m_contour_set = boundary->copy( bCopy );
        if (!m_contour_set)
            return WT_Result::Out_Of_Memory_Error;
    }
    else
    {
        m_contour_set = WD_Null;
    }

	return WT_Result::Success;

}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Viewport::set(WT_Viewport const & viewport, WT_Boolean bCopy)
{
    m_incarnation = viewport.m_incarnation;
    m_stage = Eating_Initial_Whitespace;

    if (viewport.m_fields_defined & VIEWPORT_UNITS_BIT)
        m_viewport_units = viewport.viewport_units();
    m_fields_defined |= viewport.m_fields_defined;

    set( viewport.m_name );
    WD_CHECK( set( viewport.m_contour_set, bCopy) );

	return WT_Result::Success;
}

