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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/dashpat.cpp 1     9/12/04 8:52p Evansg $
#include "whiptk/pch.h"


const WT_Dash_Pattern WT_Dash_Pattern::kNull;

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Dash_Pattern::object_id() const
{
    return Dash_Pattern_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::set(
    WT_Integer32 index,
    WT_Integer16 length,
    WT_Integer16 const * pArray )
{
    if (length%2 != 0)
        return WT_Result::Toolkit_Usage_Error;
    m_number = index;
    if( m_data )
    {   delete[] m_data;
        m_data = WD_Null;
    }
    m_size = m_allocated = length;
    if( !length || !pArray )
		return WT_Result::Success;
    m_data = new WT_Integer16[m_size];
    if (!m_data)
        return WT_Result::Out_Of_Memory_Error;
    for( int i=0; i<m_size; i++ )
        m_data[i] = pArray[i];

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::set(
    WT_Dash_Pattern const & r )
{
	return set(
        r.number(),
        r.length(),
        r.pattern());
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::skip_operand(
    WT_Opcode const& opcode,
    WT_File& file )
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::process( WT_File& file )
{
    WD_Assert( file.dash_pattern_action() );
    return (file.dash_pattern_action())(*this,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Dash_Pattern::operator== ( WT_Attribute const & atref ) const
{
    if( atref.object_id() != Dash_Pattern_ID )
        return WD_False;

    WT_Dash_Pattern const& r = (WT_Dash_Pattern const &)atref;

    if( number() != r.number() )
        return WD_False;
    if( length() != r.length() )
        return WD_False;
    WT_Integer16 const * pL = pattern();
    WT_Integer16 const * pR = r.pattern();
    for( int i=0; i<length(); i++ )
        if( pL[i] != pR[i] )
            return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Dash_Pattern::sync( WT_File& file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().dash_pattern() )
    {   file.rendition().dash_pattern() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Dash_Pattern::sync( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::default_process(
    WT_Dash_Pattern& item,
    WT_File& file )
{
    file.rendition().dash_pattern() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Integer16& WT_Dash_Pattern::operator[] ( WT_Integer16 i )
{
    if( !m_data || i>length() )
        throw WT_Result::Toolkit_Usage_Error;
    return m_data[i];
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Dash_Pattern::serialize_pattern_definition( WT_File& file ) const
{
    if( !m_size || !m_data )
        return WT_Result::Success;
    WD_CHECK( file.write((WT_Byte)' ') );
    WT_Integer16 const * p = pattern();
    for( int i=0; i<m_size; p++, i++ )
    {   if(!( i % 6 ))
        {   WD_CHECK( file.write_tab_level() );
            WD_CHECK( file.write((WT_Byte)'\t') );
        }
        WD_CHECK( file.write_ascii(*p) );
        if( i < m_size-1 )
            WD_CHECK( file.write((WT_Byte)',') );
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Dash_Pattern::serialize_pattern_definition( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Dash_Pattern::serialize( WT_File& file ) const
{
    WD_CHECK( file.dump_delayed_drawable() );

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::Line_Style_Bit));
    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(DashPattern ") );
    WD_CHECK( file.write_ascii(m_number) );

    // update definition to the file
    WT_Dash_Pattern const * pPattern = file.dash_pattern_list().find_pattern(m_number);

    if( !pPattern && !m_data )
        throw WT_Result::Toolkit_Usage_Error;
    if( !pPattern || *this != *pPattern )
        WD_CHECK( serialize_pattern_definition(file) );
    if( !pPattern )
        file.dash_pattern_list().add_pattern(*this);

    WD_CHECK( file.write((WT_Byte)')') );
    return WT_Result::Success;
}
#else
WT_Result WT_Dash_Pattern::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Dash_Pattern::materialize(
    WT_Opcode const & opcode,
    WT_File& file )
{

    WT_Byte a_byte = 0;
    WT_Integer32 tmp = 0;

    if( opcode.type() != WT_Opcode::Extended_ASCII )
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Number;
        // No Break Here

    case Getting_Number:
        WD_CHECK( file.read_ascii(m_number) );
        m_stage = Checking_For_Attribute_End;
        // No Break Here

    case Checking_For_Attribute_End:
        WD_CHECK( file.read(a_byte) );
        file.put_back(a_byte);
        if( a_byte == ')' )
        {   m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        if( !m_data )
            m_data = new WT_Integer16[m_allocated=10];
        else
            // don't reallocate
            // just zero prev data
            ::memset( (void*)m_data, 0, m_allocated*sizeof(WT_Integer16) );
        m_size = 0;
        m_stage = Getting_Data;
        // No Break Here

next_value:
    case Getting_Data:
        WD_CHECK( file.read_ascii(tmp) );
        if(!( m_size < m_allocated-1 ))
        {   // grow the buffer
            WT_Integer16* pData =
                new WT_Integer16[m_allocated+=10];
            WD_COPY_MEMORY(
                m_data,
                m_size * sizeof(WT_Integer16),
                pData);
            delete[] m_data;
            m_data = pData;
        }
        m_data[m_size++] = (WT_Integer16)tmp;
        m_stage = Getting_Separator;
        // No Break Here

    case Getting_Separator:
        WD_CHECK( file.read(a_byte) );
        if( a_byte == ',' )
        {   m_stage = Getting_Data;
            goto next_value;
        }
        else
        {   file.put_back(a_byte);
            m_data[m_size] = -1;// heidi terminator
        }
        m_stage = Eating_End_Whitespace;
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
