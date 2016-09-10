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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/wtstring.cpp 9     8/22/05 6:55p Gopalas $

#include "whiptk/pch.h"
#include "whiptk/convert_utf.h"
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////
int WT_String::wcslen (WT_Unsigned_Integer16 const * string) {
    if (string == WD_Null) return 0;

    WT_Unsigned_Integer16 const *ptr = string;

    while (*ptr++) { /* do nothing */ }
    // Ensure we don't overflow the maximum value of an 'int'
    WD_Assert((ptr - string - 1) == (int)(ptr - string - 1) );
    return ((int)(ptr - string - 1));
}

///////////////////////////////////////////////////////////////////////////
const WT_String WT_String::kNull;

///////////////////////////////////////////////////////////////////////////
WT_String::~WT_String()
{
    if( m_string )
    {
        delete [] m_string;
        m_string = WD_Null;
    }

    if( m_ascii_string )
    {
        delete [] m_ascii_string;
        m_ascii_string = WD_Null;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String( WT_String const & string ) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    *this = string;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String( WT_Unsigned_Integer16 const * string ) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    *this = string;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String( WT_Byte const * string ) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Only_ASCII)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    *this = string;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String( char const * string ) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Only_ASCII)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    *this = string;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String(int length, WT_Unsigned_Integer16 const * string) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    WT_Result res = set(length,string);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String(int length, WT_Byte const * string) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    WT_Result res = set(length,string);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_String::WT_String(int length, char const * string) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    WT_Result res = set(length,string);
	if(res != WT_Result::Success)
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_String const & WT_String::operator= ( WT_Unsigned_Integer16 const * string ) throw(WT_Result)
{
    WT_Result res = set(string?wcslen(string):0,string);
	if(res == WT_Result::Success)
		return *this;
	else
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_String const & WT_String::operator= (WT_Byte const * string) throw(WT_Result)
{
    WT_Result res = set(string?static_cast<int>(strlen((char*)string)):0,string);
    if(res == WT_Result::Success)
		return *this;
	else
		throw res;
}


///////////////////////////////////////////////////////////////////////////
WT_String const & WT_String::operator= (char const * string) throw(WT_Result)
{
    WT_Result res = set(string?static_cast<int>(strlen(string)):0,string);
    if(res == WT_Result::Success)
		return *this;
	else
		throw res;

}

///////////////////////////////////////////////////////////////////////////
WT_String const & WT_String::operator= (WT_String const & string) throw(WT_Result)
{
    WT_Result res = set(string);
    if(res == WT_Result::Success)
		return *this;
	else
		throw res;

}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::set(int length, char const * string)
{
    return set(length,(WT_Byte const*)string);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::set(int length, WT_Byte const * string)
{
    *this = WT_String::kNull;
    if(!string)
		return WT_Result::Success;
    m_length=length;
    if (!length)
        return WT_Result::Success;
    if( WT_String::is_ascii(length,(char*)string) )
    {
        m_ascii_string = new WT_Byte[m_length+1];
        if(!m_ascii_string)
            return WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY(
            string,
            m_length,
            m_ascii_string);
        m_ascii_string[m_length] = 0;
        m_string_type = Only_ASCII;
    }
    else
    {
        m_string = WT_String::to_unicode(length, string);
        m_string_type = Non_ASCII;
    }

	return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::set(int length, WT_Unsigned_Integer16 const * string)
{
    *this = WT_String::kNull;
    if(!string)
		return WT_Result::Success;
    m_length=length;
    if(!length)
		return WT_Result::Success;
    if( WT_String::is_ascii(length,string) )
    {
        m_ascii_string =
            WT_String::to_ascii(
                m_length,
                string);
        m_string_type = Only_ASCII;
    }
    else
    {
        m_string = new WT_Unsigned_Integer16[m_length+1];
        if(!m_string)
            return WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY(
            string,
            m_length * sizeof(WT_Unsigned_Integer16),
            m_string);
        m_string[m_length] = 0;
        m_string_type = Non_ASCII;
    }

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::set( WT_String const & string )
{
    m_string_type = string.m_string_type;

    if( m_string )
    {
        delete [] m_string;
        m_string = WD_Null;
    }

    if( m_ascii_string )
    {
        delete [] m_ascii_string;
        m_ascii_string = WD_Null;
    }

    m_length=string.length();
    if(!m_length)
        return WT_Result::Success;

    if( string.is_ascii() )
    {
        m_ascii_string = new WT_Byte[m_length+1];
        if(!m_ascii_string)
            return WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY(
            string.ascii(),
            m_length,
            m_ascii_string);
        m_ascii_string[m_length] = 0;
    }
    else
    {
        m_string = new WT_Unsigned_Integer16[m_length+1];
        if(!m_string)
            return WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY(
            string.unicode(),
            m_length * sizeof(WT_Unsigned_Integer16),
            m_string);
        m_string[m_length] = 0;
    }

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::is_ascii() const
{
    if( m_string_type == Only_ASCII )
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::is_ascii(int length, WT_Unsigned_Integer16 const * string)
{
    WT_Unsigned_Integer16 const * p = string;
    for( int i=0; i<length; i++ )
        if( *(p++) > (WT_Unsigned_Integer16)127 )
            return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::is_ascii(int length, char const * string)
{
    char const * p = string;
    for( int i=0; i<length; i++ )
        if( *(p++) < 0 )  // non ascii means > 127, but since the type is signed char, we check for < 0, the two's complement equivalent
            return WD_False;
    return WD_True;
}


///////////////////////////////////////////////////////////////////////////
char const * WT_String::ascii() const
{
    ((WT_String*)this)->restore();
    return (char const *) m_ascii_string;
}

///////////////////////////////////////////////////////////////////////////
WT_Unsigned_Integer16 const * WT_String::unicode() const
{
    ((WT_String*)this)->expand();
    return m_string;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_String::serialize(WT_File & file, WT_Boolean force_quotes/*=WD_False*/) const
{
    WT_Result r =
        WT_Result::Success;
    switch( m_string_type )
    {
    case Only_ASCII:
        r = file.write_quoted_string(
            ascii(),
            force_quotes);
        break;
    case Non_ASCII:
        r = file.write_quoted_string(
            unicode());
        break;
    case Not_Yet_Scanned:
    default:
        r = file.write_quoted_string("");
        break;
    }
    return r;
}
#else
WT_Result WT_String::serialize(WT_File &, WT_Boolean) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Byte WT_String::to_binary( WT_Byte ascii_hex_digit )
{
    WT_Byte d   = ascii_hex_digit;

    if( d >= '0' && d <= '9' )
        return (d - '0');
    else if( d >= 'A' && d <= 'F' )
        return (d - 'A' + 0x0A);
    else if( d >= 'a' && d <= 'f' )
        return (d - 'a' + 0x0A);
    else
    {
        WD_Assert(false);   // Not a hex digit
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::materialize(WT_File & file)
{
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_String_Type;
        // No Break Here
    case Getting_String_Type:
        WD_CHECK( file.read(m_lead_byte) );
        file.put_back(m_lead_byte);
        m_string_type =
            ( m_lead_byte!='{' && m_lead_byte!='"' )
                ? Only_ASCII
                : Non_ASCII;
        m_stage = Getting_String;
        break;
    default:
        // try the next
        // set of cases
        break;
    }

    if( m_lead_byte == '{'/*binary*/ )
    {
        switch( m_stage )
        {
        case Getting_String:
            WD_CHECK( file.read(m_lead_byte) );// get '{'
            m_stage = Getting_String_Length;
            // No Break Here
        case Getting_String_Length:
            WD_CHECK( file.read(m_length) );
            m_stage = Getting_String_Data;
            // No Break Here
        case Getting_String_Data:
            if( m_string )
            {
                delete [] m_string;
                m_string = WD_Null;
            }
            m_string = new WT_Unsigned_Integer16[m_length+1];
            if(!m_string)
                return WT_Result::Out_Of_Memory_Error;
            WD_CHECK( file.read(m_length,m_string) );
            m_string[m_length] = '\0';
            m_stage = Getting_End_Quote;
            // No Break Here
        case Getting_End_Quote:
            WD_CHECK( file.read(m_lead_byte) );// get '}'
            if (m_lead_byte != '}')
                return WT_Result::Corrupt_File_Error;
            m_stage = Eating_Initial_Whitespace;
            break;
        default:
            return WT_Result::Internal_Error;
        }
    }
    else /*ascii*/
    {
        if( m_ascii_string )
        {
            delete [] m_ascii_string;
            m_ascii_string = WD_Null;
        }
        WD_CHECK(
            file.read(
                (char*&)m_ascii_string,
                65536) );

        m_length = static_cast<WT_Integer32>(strlen((char*)m_ascii_string));
        if(!m_length)
            *this = WT_String::kNull;

        else if( m_lead_byte == '"' )
        {
            if( m_length % 4 )
                return WT_Result::Corrupt_File_Error;

            m_length /= 4;

            delete []m_string;
            m_string = new WT_Unsigned_Integer16[m_length+1];
            if(!m_string)
                return WT_Result::Out_Of_Memory_Error;

            WT_Byte* pRead = m_ascii_string;
            WT_Unsigned_Integer16* pWrite = m_string;
            for(; *pRead; pWrite++ )
            {
                *pWrite = 0;
                *pWrite = *pWrite + WT_String::to_binary(*(pRead++));
                *pWrite <<= 4;
                *pWrite = *pWrite + WT_String::to_binary(*(pRead++));
                *pWrite <<= 4;
                *pWrite = *pWrite + WT_String::to_binary(*(pRead++));
                *pWrite <<= 4;
                *pWrite = *pWrite + WT_String::to_binary(*(pRead++));
                *pWrite = WT_Endian::adjust(*pWrite);
            }
            *pWrite = 0;

            delete [] m_ascii_string;
            m_ascii_string = WD_Null;
        }

        m_stage = Eating_Initial_Whitespace;

    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::equals(WT_String const & string, WT_Boolean case_sensitive) const
{
    ((WT_String*)this)->restore();
    ((WT_String&)string).restore();

    if( m_string_type != string.m_string_type ||
        m_length != string.m_length )
        return WD_False;

    WT_Integer32 i = 0;
    if( is_ascii() )
    {
        if (case_sensitive)
        {
            for(; i<m_length; i++)
            {
                if( m_ascii_string[i] != string.m_ascii_string[i] )
                    return WD_False;
            }
        }
        else
        {
            for(; i<m_length; i++)
            {
                if( tolower(m_ascii_string[i]) != tolower(string.m_ascii_string[i]) )
                    return WD_False;
            }
        }
    }
    else
    {
        if (case_sensitive)
        {
            for(; i<m_length; i++)
            {
                if( m_string[i] != string.m_string[i] )
                    return WD_False;
            }
        }
        else
        {
            for(; i<m_length; i++)
            {
                if(tolower(m_string[i]) != tolower(string.m_string[i]) )
                    return WD_False;
            }
        }
    }
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::equals( char const * string, WT_Boolean case_sensitive ) const
{
    WT_String tmp(string);
    return equals(tmp, case_sensitive);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::equals_no_case(WT_String const & string) const
{
    return equals(string, WD_False);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::equals_no_case( char const * string ) const
{
    return equals(string, WD_False);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::operator== (WT_String const & string) const
{
    return equals(string);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::operator== ( char const * string ) const
{
    return equals(string);
}

///////////////////////////////////////////////////////////////////////////
WT_Byte * WT_String::to_ascii( int length, WT_Unsigned_Integer16 const * string ) throw(WT_Result)
{
    WT_Byte* new_string = WD_Null;
    if( !length || !string )
        return new_string;
    new_string=new WT_Byte[length+1];
    if(!new_string)
        throw WT_Result::Out_Of_Memory_Error;
    int i;
    for( i=0; i<length; i++ )
        new_string[i] = (WT_Byte) string[i];
    new_string[i] = 0;
    return new_string;
}

///////////////////////////////////////////////////////////////////////////
WT_Unsigned_Integer16 * WT_String::to_unicode( int length, WT_Byte const * string ) throw(WT_Result)
{
    WT_Unsigned_Integer16* new_string = WD_Null;
    if( !length || !string )
        return new_string;
    new_string=new WT_Unsigned_Integer16[length+1];
    if(!new_string)
        throw WT_Result::Out_Of_Memory_Error;
    int i;
    for(i=0; i<length; i++ )
        new_string[i] = (WT_Unsigned_Integer16) string[i];
    new_string[i] = 0;
    return new_string;
}

/*static*/
wchar_t * WT_String::to_wchar(
    int length, /**< The length of the string in characters. */
    WT_Unsigned_Integer16 const * string /**< The 16 bit wide character (Unicode) buffer to copy from. */
    ) throw(WT_Result)
{
    wchar_t *new_string = new wchar_t[length+1];
    if(!new_string)
        throw WT_Result::Out_Of_Memory_Error;

    // Init it to all zeros, so that there's never a problem with termination.
    WD_SET_MEMORY(new_string, (length+1)*sizeof(wchar_t), 0);

    if(sizeof(wchar_t) == 2) // Win32
    {
        if(!new_string)
        {
            throw WT_Result::Out_Of_Memory_Error;
        }

        // Note, this is assuming that the input string is UCS-2 - no UTF16 surrogate pairs
        int i;
        for(i=0; i<length; i++ )
        {
            new_string[i] = (wchar_t) string[i];
        }

        return new_string;
    }

    if (sizeof(wchar_t) == 4) // Rest of world
    {
        // Note: Don't optimize these away - ConvertUTF16toUTF32 messes with the stored vals.
        const UTF16 *source = (const UTF16 *) string;
        UTF32 *target = (UTF32*) new_string;
        ConversionResult res = ConvertUTF16toUTF32(
                                    &source, source+length, 
                                    &target, target+length,
                                    lenientConversion
                                );
        if(res==conversionOK)
        {
            return new_string;
        }
    }

    // Should never get here...
    delete new_string;

    return NULL;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::restore()
{
    if( is_ascii() && !m_ascii_string && m_string)
    {
        m_ascii_string = WT_String::to_ascii(m_length,m_string);
        delete [] m_string;
        m_string = WD_Null;
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_String::expand()
{
    if( is_ascii() && !m_string && m_ascii_string )
    {
        m_string = WT_String::to_unicode(m_length,m_ascii_string);
        delete [] m_ascii_string;
        m_ascii_string = WD_Null;
        return WD_True;
    }
    return WD_False;
}

#ifdef WHIP_USE_WCHAR_STRINGS
///////////////////////////////////////////////////////////////////////////
WT_String::WT_String( wchar_t const * string ) throw(WT_Result)
: m_stage(Eating_Initial_Whitespace)
, m_string_type(Not_Yet_Scanned)
, m_length(0)
, m_string(WD_Null)
, m_ascii_string(WD_Null)
, m_lead_byte (0)
{
    *this = string;
}

///////////////////////////////////////////////////////////////////////////
WT_String const & WT_String::operator= ( wchar_t const * string ) throw(WT_Result)
{
    WT_Result res = set((int)(string ? ::wcslen(string) : 0),string);
	if(res == WT_Result::Success)
		return *this;
	else
		throw res;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_String::set( int length, wchar_t const * string )
{
    if (sizeof(wchar_t) == 4) // Rest of world
    {
        int targetLength = (length*4) + 1;
        WT_Unsigned_Integer16 *new_string = new WT_Unsigned_Integer16[ targetLength ];
        if(!new_string)
            throw WT_Result::Out_Of_Memory_Error;

        // Init it to all zeros, so that there's never a problem with termination.
        WD_SET_MEMORY(new_string, (targetLength)*sizeof(WT_Unsigned_Integer16), 0);

        // Note: Don't optimize these away - ConvertUTF16toUTF32 messes with the stored vals.
        const UTF32 *source = (const UTF32 *) string;
        UTF16 *target = (UTF16*) new_string;
        ConversionResult res = ConvertUTF32toUTF16(
                                    &source, source+length, 
                                    &target, target+(targetLength-1),
                                    lenientConversion
                                );
        WT_Result result = WT_Result::Success;
        if(res == conversionOK)
        {
            result = set ( wcslen( new_string ), new_string );
        }
        delete[] new_string;
        return result;
    }
    else if (sizeof(wchar_t) == 2)
    {
        return set( length, (WT_Unsigned_Integer16*)string );
    }
    else
    {
        return WT_Result::Internal_Error;
    }
}
#endif
