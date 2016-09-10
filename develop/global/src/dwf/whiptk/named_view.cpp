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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/named_view.cpp 1     9/12/04 8:55p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Named_View::~WT_Named_View ()
{
    delete m_view;
    m_view = WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View::WT_Named_View (WT_Logical_Box const & view, const char * name)
    : m_view (WD_Null)
    , m_name ()
    , m_has_been_serialized (WD_False)
    , m_stage (Eating_Initial_Whitespace)
{
    set (view);
    set (name);
    set_next (NULL);
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View::WT_Named_View (WT_Logical_Box const & view, WT_Unsigned_Integer16 const * name)
    : m_view (WD_Null)
    , m_name ()
    , m_has_been_serialized (WD_False)
    , m_stage (Eating_Initial_Whitespace)
{
    set (view);
    set (name);
    set_next (NULL);
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View::WT_Named_View (WT_Named_View const & named_view)
    : WT_Item()
    , WT_Object()
    , m_view (WD_Null)
    , m_name()
    , m_has_been_serialized (WD_False)
    , m_stage (Eating_Initial_Whitespace)
{
    set (named_view);
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View const & WT_Named_View::operator= (WT_Named_View const & named_view)
{
    set (named_view);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Named_View::object_id() const
{
    return Named_View_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Named_View::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Named_View::operator== (WT_Named_View const & named_view) const
{
    if ( this->view() && named_view.view() )
    {
        if ( ( *this->view() == *named_view.view() ) &&
             ( this->name() == named_view.name() ) )
            return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Named_View::serialize (WT_File & file) const
{
    if (m_has_been_serialized)
        return WT_Result::Success;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write( "(NamedView " ));

    WT_Logical_Point        adjusted_min;
    WT_Logical_Point        adjusted_max;
    WT_Logical_Point        final_min;
    WT_Logical_Point        final_max;


    if (file.heuristics().apply_transform())
    {
        WT_Transform const &    xform        = file.heuristics().transform();
        WT_Logical_Point        orig_xformed_min_cnr;
        WT_Logical_Point        orig_xformed_max_cnr;

        orig_xformed_min_cnr = view()->m_min * xform;
        orig_xformed_max_cnr = view()->m_max * xform;

        switch (xform.rotation())
        {
            case 0:
                adjusted_min = orig_xformed_min_cnr;
                adjusted_max = orig_xformed_max_cnr;
                break;
            case 90:
                adjusted_min = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                adjusted_max = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                break;
            case 180:
                adjusted_min = orig_xformed_max_cnr;
                adjusted_max = orig_xformed_min_cnr;
                break;
            case 270:
                adjusted_min = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                adjusted_max = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                break;
            default:
                throw WT_Result::Internal_Error;
        }
    }
    else
    {
        adjusted_min = view()->m_min;
        adjusted_max = view()->m_max;
    }

    final_min.m_x = WD_MIN(adjusted_min.m_x, adjusted_max.m_x);
    final_min.m_y = WD_MIN(adjusted_min.m_y, adjusted_max.m_y);

    final_max.m_x = WD_MAX(adjusted_min.m_x, adjusted_max.m_x);
    final_max.m_y = WD_MAX(adjusted_min.m_y, adjusted_max.m_y);

    WD_CHECK (file.write_ascii(1, &final_min));
    WD_CHECK (file.write(" "));
    WD_CHECK (file.write_ascii(1, &final_max));

    // write name
    WD_CHECK (file.write(" "));
    WD_CHECK (m_name.serialize(file));
    WD_CHECK (file.write( ")" ));

    // Get around the const-ness of this method.  Sneaky sneaky.
    ((WT_Named_View*)this)->m_has_been_serialized = WD_True;
    return WT_Result::Success;
}
#else
WT_Result WT_Named_View::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Named_View::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Logical_Box      view;

    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch (m_stage)
    {
    case Eating_End_Whitespace:
    {
        break;
    }
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_View;
        // No Break here

    case Gathering_View:
        WD_CHECK(file.read_ascii(view));

        if (file.heuristics().apply_transform())
        {
            view.m_min.m_x = (long)((view.m_min.m_x + file.heuristics().transform().m_translate.m_x) * file.heuristics().transform().m_x_scale);
            view.m_min.m_y = (long)((view.m_min.m_y + file.heuristics().transform().m_translate.m_y) * file.heuristics().transform().m_y_scale);
            view.m_max.m_x = (long)((view.m_max.m_x + file.heuristics().transform().m_translate.m_x) * file.heuristics().transform().m_x_scale);
            view.m_max.m_y = (long)((view.m_max.m_y + file.heuristics().transform().m_translate.m_y) * file.heuristics().transform().m_y_scale);
        }

        set ( WT_Logical_Box (view.m_min.m_x, view.m_min.m_y, view.m_max.m_x, view.m_max.m_y) );
        m_stage = Eating_Middle_Whitespace;
        // No Break here

    case Eating_Middle_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_Name;
        // No Break here

    case Gathering_Name:
        WD_CHECK (m_name.materialize(file));
        m_stage = Eating_End_Whitespace;
        // No Break here
    }
    WD_CHECK ( opcode.skip_past_matching_paren(file) );

    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Named_View::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Named_View::sync (WT_File &) const
{
    //TODO:: implement
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Named_View::process(WT_File & file)
{
    WD_Assert(file.named_view_action());
    return (file.named_view_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Named_View::default_process(WT_Named_View & named_view, WT_File & file)
{
    file.rendition().drawing_info().named_view_list().add_named_view (named_view    );
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Named_View::set (WT_Named_View const & named_view)
{
    if ( !(*this == named_view))
    {
        m_has_been_serialized = WD_False;
        set (*(named_view.view ()) );
        set ( (named_view.name ()) );
        set_next (((WT_Named_View*)&named_view)->next());
        m_has_been_serialized = WD_False;
    }
}

///////////////////////////////////////////////////////////////////////////
void WT_Named_View::set(const char * name)
{
    m_has_been_serialized = WD_False;
    m_name = name;
}

///////////////////////////////////////////////////////////////////////////
void WT_Named_View::set(WT_Unsigned_Integer16 const * name)
{
    m_has_been_serialized = WD_False;
    m_name.set(WT_String::wcslen(name),name);
}

///////////////////////////////////////////////////////////////////////////
void WT_Named_View::set(int length, WT_Unsigned_Integer16 const * name)
{
    m_has_been_serialized = WD_False;
    m_name.set(length,name);
}

///////////////////////////////////////////////////////////////////////////
void WT_Named_View::set(WT_String const & name)
{
    m_has_been_serialized = WD_False;
    m_name = name;
}

///////////////////////////////////////////////////////////////////////////
void  WT_Named_View::set (WT_Logical_Box const & view)
{
    m_has_been_serialized = WD_False;
    // make copy of view
    delete m_view;
    m_view = new WT_Logical_Box
                (view.m_min.m_x,
                 view.m_min.m_y,
                 view.m_max.m_x,
                 view.m_max.m_y);
}

///////////////////////////////////////////////////////////////////////////
WT_Named_View* WT_Named_View::copy() const
{
    WT_Named_View *pView = new WT_Named_View( *this );
    return pView;
}
