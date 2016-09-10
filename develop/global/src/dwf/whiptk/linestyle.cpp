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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/linestyle.cpp 3     5/18/05 2:48p Hainese $
#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Line_Style::operator== ( WT_Attribute const & atref ) const
{   if( atref.object_id() != Line_Style_ID )
        return WD_False;
    WT_Line_Style const & r =
        (WT_Line_Style const &) atref;

    if( m_fields_defined != r.m_fields_defined
        || m_adapt_patterns != r.adapt_patterns()
        || m_pattern_scale != r.pattern_scale()
        || m_line_join != r.line_join()
        || m_dash_start_cap != r.dash_start_cap()
        || m_dash_end_cap != r.dash_end_cap()
        || m_line_start_cap != r.line_start_cap()
        || m_line_end_cap != r.line_end_cap()
        || m_miter_angle != r.miter_angle()
        || m_miter_length != r.miter_length() )
        return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::sync( WT_File& file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().line_style() )
    {   WD_CHECK( serialize(file) );
        // did update in serialize:
        // file.rendition().line_style() = *this;
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::sync( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::process( WT_File& file )
{
    WD_Assert( file.line_style_action() );
    return (file.line_style_action())(*this,file);
}

///////////////////////////////////////////////////////////////////////////
void WT_Line_Style::merge( WT_Line_Style const & style )
{
    m_fields_defined |= style.m_fields_defined;
    if( style.m_fields_defined & ADAPT_PATTERNS_BIT )
        m_adapt_patterns = style.adapt_patterns();
    if( style.m_fields_defined & PATTERN_SCALE_BIT )
        m_pattern_scale = style.pattern_scale();
    if( style.m_fields_defined & LINE_JOIN_BIT )
        m_line_join = style.line_join();
    if( style.m_fields_defined & DASH_START_CAP_BIT )
        m_dash_start_cap = style.dash_start_cap();
    if( style.m_fields_defined & DASH_END_CAP_BIT )
        m_dash_end_cap = style.dash_end_cap();
    if( style.m_fields_defined & LINE_START_CAP_BIT )
        m_line_start_cap = style.line_start_cap();
    if( style.m_fields_defined & LINE_END_CAP_BIT )
        m_line_end_cap = style.line_end_cap();
    if( style.m_fields_defined & MITER_ANGLE_BIT )
        m_miter_angle = style.miter_angle();
    if( style.m_fields_defined & MITER_LENGTH_BIT )
        m_miter_length = style.miter_length();
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::default_process(
    WT_Line_Style& item,
    WT_File& file )
{
    file.rendition().line_style().merge(item);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::skip_operand(
    WT_Opcode const & opcode,
    WT_File& file )
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Adapt_Patterns::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Corrupt_File_Error;
    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        if( !strcmp(string,"true")
            || !strcmp(string,"TRUE")
            || !strcmp(string,"1") )
        {   m_value = WD_True;
            m_materialized = WD_True;
        }
        if( !strcmp(string,"false")
            || !strcmp(string,"FALSE")
            || !strcmp(string,"0") )
        {   m_value = WD_False;
            m_materialized = WD_True;
        }
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }

    return m_materialized
        ? WT_Result::Success
        : WT_Result::Corrupt_File_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::interpret_joinstyle(
    const char * string,
    WT_Joinstyle_ID& value )
{
    if( !strcmp(string,"miter") )
    {   value = Miter_Join;
        return WT_Result::Success;
    }
    if( !strcmp(string,"bevel") )
    {   value = Bevel_Join;
        return WT_Result::Success;
    }
    if( !strcmp(string,"round") )
    {   value = Round_Join;
        return WT_Result::Success;
    }
    if( !strcmp(string,"diamond") )
    {   value = Diamond_Join;
        return WT_Result::Success;
    }
    return WT_Result::Corrupt_File_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::interpret_capstyle(
    const char * string,
    WT_Capstyle_ID& value )
{
    if( !strcmp(string,"butt") )
    {   value = Butt_Cap;
        return WT_Result::Success;
    }
    if( !strcmp(string,"square") )
    {   value = Square_Cap;
        return WT_Result::Success;
    }
    if( !strcmp(string,"round") )
    {   value = Round_Cap;
        return WT_Result::Success;
    }
    if( !strcmp(string,"diamond") )
    {   value = Diamond_Cap;
        return WT_Result::Success;
    }
    return WT_Result::Corrupt_File_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Line_Join::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;
    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( WT_Line_Style::interpret_joinstyle(string,m_value) );
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Dash_Start_Cap::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;
    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( WT_Line_Style::interpret_capstyle(string,m_value) );
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Dash_End_Cap::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;

    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( WT_Line_Style::interpret_capstyle(string,m_value) );
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Line_Start_Cap::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;

    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( WT_Line_Style::interpret_capstyle(string,m_value) );
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Line_End_Cap::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    char* string = 0;

    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( WT_Line_Style::interpret_capstyle(string,m_value) );
        delete [] string;
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Miter_Angle::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read_ascii(m_value) );
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::WT_Miter_Length::materialize(
    WT_Object&,
    WT_Optioncode const & opcode,
    WT_File& file )
{
    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Value;
        // No Break Here

    case Getting_Value:
        WD_CHECK( file.read_ascii(m_value) );
        m_stage = Eating_End_Whitespace;
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::materialize(
    WT_Opcode const & opcode,
    WT_File& file )
{
    switch( opcode.type() )
    {
    case WT_Opcode::Extended_ASCII:
        return materialize_ascii(opcode,file);
    case WT_Opcode::Single_Byte:
    case WT_Opcode::Extended_Binary:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Style::materialize_ascii(
    WT_Opcode const & opcode,
    WT_File& file )
{
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Checking_For_Attribute_End;
        // No Break Here

    case Checking_For_Attribute_End:
        WT_Byte a_byte;
        WD_CHECK( file.read(a_byte) );
        file.put_back(a_byte);
        if( a_byte == ')' )
        {   m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Getting_Optioncode;
        // No Break Here

next_option:
    case Getting_Optioncode:
        WD_CHECK( m_optioncode.get_optioncode(file) );
        if( m_optioncode.type() == WT_Opcode::Null_Optioncode )
        {
            m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Materializing_Option;
        // No Break Here

    case Materializing_Option:
        switch( m_optioncode.option_id() )
        {
        case WT_Line_Style_Option_Code::Adapt_Patterns_Option:
            m_fields_defined |= ADAPT_PATTERNS_BIT;
            WD_CHECK( m_adapt_patterns.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Pattern_Scale_Option:
            m_fields_defined |= PATTERN_SCALE_BIT;
            WD_CHECK( m_pattern_scale.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Line_Join_Option:
            m_fields_defined |= LINE_JOIN_BIT;
            WD_CHECK( m_line_join.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Dash_Start_Cap_Option:
            m_fields_defined |= DASH_START_CAP_BIT;
            WD_CHECK( m_dash_start_cap.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Dash_End_Cap_Option:
            m_fields_defined |= DASH_END_CAP_BIT;
            WD_CHECK( m_dash_end_cap.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Line_Start_Cap_Option:
            m_fields_defined |= LINE_START_CAP_BIT;
            WD_CHECK( m_line_start_cap.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Line_End_Cap_Option:
            m_fields_defined |= LINE_END_CAP_BIT;
            WD_CHECK( m_line_end_cap.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Miter_Angle_Option:
            m_fields_defined |= MITER_ANGLE_BIT;
            WD_CHECK( m_miter_angle.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Miter_Length_Option:
            m_fields_defined |= MITER_LENGTH_BIT;
            WD_CHECK( m_miter_length.materialize(*this,m_optioncode,file) );
            break;
        case WT_Line_Style_Option_Code::Unknown_Option:
            WD_CHECK( m_optioncode.skip_past_matching_paren(file) );
            break;
        default:
            return WT_Result::Internal_Error;
        } // switch (m_optioncode.option_id())
        m_stage = Getting_Optioncode;
        goto next_option;
        // No Break Here

end_ascii:
    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Adapt_Patterns::serialize(
    WT_Object const &,
    WT_File & file ) const
{

    WD_CHECK( file.dump_delayed_drawable() );

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(AdaptPatterns ") );
    if(m_value)
        return file.write("true)");
    return file.write("false)");
}
#else
WT_Result WT_Line_Style::WT_Adapt_Patterns::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Line_Join::serialize(
    WT_Object const &,
    WT_File & file ) const
{

    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(LineJoin ") );
    switch( m_value )
    {
    case Miter_Join:
        WD_CHECK( file.write("miter") );
        break;
    case Bevel_Join:
        WD_CHECK( file.write("bevel") );
        break;
    case Round_Join:
        WD_CHECK( file.write("round") );
        break;
    case Diamond_Join:
        WD_CHECK( file.write("diamond") );
        break;
    default:
        return WT_Result::Internal_Error;
    }
    return file.write (")");
}

#else
WT_Result WT_Line_Style::WT_Line_Join::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::serialize_capstyle_ascii( WT_Capstyle_ID id, WT_File& file )
{
    switch( id )
    {
    case Butt_Cap:
        return file.write("butt");

    case Square_Cap:
        return file.write("square");

    case Round_Cap:
        return file.write("round");

    case Diamond_Cap:
        return file.write("diamond");

    default:
        return WT_Result::Internal_Error;
    }
}
#else
WT_Result WT_Line_Style::serialize_capstyle_ascii( WT_Capstyle_ID, WT_File&)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Dash_Start_Cap::serialize(
    WT_Object const &,
    WT_File & file ) const
{

    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(DashStartCap "));
    WD_CHECK( WT_Line_Style::serialize_capstyle_ascii(m_value,file) );
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else

WT_Result WT_Line_Style::WT_Dash_Start_Cap::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Dash_End_Cap::serialize(
    WT_Object const &,
    WT_File & file ) const
{
    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(DashEndCap "));
    WD_CHECK( WT_Line_Style::serialize_capstyle_ascii(m_value,file) );
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::WT_Dash_End_Cap::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Line_Start_Cap::serialize(
    WT_Object const &,
    WT_File & file ) const
{
    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(LineStartCap "));
    WD_CHECK( WT_Line_Style::serialize_capstyle_ascii(m_value,file) );
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::WT_Line_Start_Cap::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Line_End_Cap::serialize(
    WT_Object const &,
    WT_File & file ) const
{
    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(LineEndCap "));
    WD_CHECK( WT_Line_Style::serialize_capstyle_ascii(m_value,file) );
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::WT_Line_End_Cap::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Miter_Angle::serialize(
    WT_Object const &,
    WT_File & file ) const
{
    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(MiterAngle "));
    WD_CHECK( file.write_ascii(m_value));
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::WT_Miter_Angle::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::WT_Miter_Length::serialize(
    WT_Object const &,
    WT_File & file ) const
{
    WD_CHECK( file.dump_delayed_drawable());
    WD_CHECK( file.write_tab_level());
    WD_CHECK( file.write("(MiterLength "));
    WD_CHECK( file.write_ascii(m_value));
    WD_CHECK( file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::WT_Miter_Length::serialize(
    WT_Object const &,
    WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Style::serialize( WT_File& file ) const
{
    WD_CHECK( file.dump_delayed_drawable() );

    // Determine required parts of this Attribute object.
    WT_Line_Style &         style   = file.rendition().line_style();
    WT_Unsigned_Integer32   fields  = 0;

    if( m_fields_defined & ADAPT_PATTERNS_BIT &&
        m_adapt_patterns != style.adapt_patterns() )
            fields |= ADAPT_PATTERNS_BIT;

    if( m_fields_defined & PATTERN_SCALE_BIT &&
        m_pattern_scale != style.pattern_scale() )
            fields |= PATTERN_SCALE_BIT;

    if( m_fields_defined & LINE_JOIN_BIT &&
        m_line_join != style.line_join() )
            fields |= LINE_JOIN_BIT;

    if( m_fields_defined & DASH_START_CAP_BIT &&
        m_dash_start_cap != style.dash_start_cap() )
            fields |= DASH_START_CAP_BIT;

    if( m_fields_defined & DASH_END_CAP_BIT &&
        m_dash_end_cap != style.dash_end_cap() )
            fields |= DASH_END_CAP_BIT;

    if( m_fields_defined & LINE_START_CAP_BIT &&
        m_line_start_cap != style.line_start_cap() )
            fields |= LINE_START_CAP_BIT;

    if( m_fields_defined & LINE_END_CAP_BIT &&
        m_line_end_cap != style.line_end_cap() )
            fields |= LINE_END_CAP_BIT;

    if( m_fields_defined & MITER_ANGLE_BIT &&
        m_miter_angle != style.miter_angle() )
            fields |= MITER_ANGLE_BIT;

    if( m_fields_defined & MITER_LENGTH_BIT &&
        m_miter_length != style.miter_length() )
            fields |= MITER_LENGTH_BIT;

    if( !fields )
        // there are no fields
        // that need to be serialized
        return WT_Result::Success;

    // Serialize this pattern's attributes if this pattern has never been serialized,
    // or if one this pattern's attributes is different from its previous instance in
    // the file.
    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(LineStyle") );
    if( fields & ADAPT_PATTERNS_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_adapt_patterns.serialize(*this,file) );
        style.adapt_patterns() = m_adapt_patterns;
    }
    if( fields & PATTERN_SCALE_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_pattern_scale.serialize(*this,file) );
        style.pattern_scale() = m_pattern_scale;
    }
    if( fields & LINE_JOIN_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_line_join.serialize(*this,file) );
        style.line_join() = m_line_join;
    }
    if( fields & DASH_START_CAP_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_dash_start_cap.serialize(*this,file) );
        style.dash_start_cap() = m_dash_start_cap;
    }
    if( fields & DASH_END_CAP_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_dash_end_cap.serialize(*this,file) );
        style.dash_end_cap() = m_dash_end_cap;
    }
    if( fields & LINE_START_CAP_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_line_start_cap.serialize(*this,file) );
        style.line_start_cap() = m_line_start_cap;
    }
    if( fields & LINE_END_CAP_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_line_end_cap.serialize(*this,file) );
        style.line_end_cap() = m_line_end_cap;
    }
    if( fields & MITER_ANGLE_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_miter_angle.serialize(*this,file) );
        style.miter_angle() = m_miter_angle;
    }
    if( fields & MITER_LENGTH_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_miter_length.serialize(*this,file) );
        style.miter_length() = m_miter_length;
    }

    WD_CHECK( file.write(")") );
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Style::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
int WT_Line_Style::WT_Line_Style_Option_Code::option_id_from_optioncode()
{
    if( !strcmp((char const *)token(), "(AdaptPatterns") )
        return Adapt_Patterns_Option;
    if( !strcmp((char const *)token(), "(LinePatternScale") )
        return Pattern_Scale_Option;
    if( !strcmp((char const *)token(), "(LineJoin") )
        return Line_Join_Option;
    if( !strcmp((char const *)token(), "(DashStartCap") )
        return Dash_Start_Cap_Option;
    if( !strcmp((char const *)token(), "(DashEndCap") )
        return Dash_End_Cap_Option;
    if( !strcmp((char const *)token(), "(LineStartCap") )
        return Line_Start_Cap_Option;
    if( !strcmp((char const *)token(), "(LineEndCap") )
        return Line_End_Cap_Option;
    if( !strcmp((char const *)token(), "(MiterAngle") )
        return Miter_Angle_Option;
    if( !strcmp((char const *)token(), "(MiterLength") )
        return Miter_Length_Option;
    return 0;
}
