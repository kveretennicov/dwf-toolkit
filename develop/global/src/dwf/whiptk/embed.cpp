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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/embed.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Embed::object_id() const
{
    return Embed_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Embed::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Embed::WT_Embed(WT_Embed const & embed)
    : WT_Object()
{
    m_stage = Eating_Initial_Whitespace;
    *this = embed;          // Call the operator= method below...
}

///////////////////////////////////////////////////////////////////////////
WT_Embed const & WT_Embed::operator=(WT_Embed const & embed)
{
    m_incarnation =    embed.m_incarnation;
    m_MIME_type = embed.MIME_type();
    m_MIME_subtype = embed.MIME_subtype();
    m_MIME_options = embed.MIME_options();
    m_description = embed.description();
    m_filename = embed.filename();
    m_url = embed.url();
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_whole_MIME(char const * mime, WT_File & file)
{
    // Parse the whole mime string into the three parts: Type/Subtype;options

    if (!mime)
    {
        set_MIME_type   ("", file);
        set_MIME_subtype("", file);
        set_MIME_options("", file);
    }
    else
    {
        char * tmp_copy;
        char * type_subtype_separator_pos;
        char * subtype_options_separator_pos;
        char * end_pos;

        tmp_copy = new char[strlen(mime) + 1];
        if (!tmp_copy)
            return WT_Result::Out_Of_Memory_Error;

        strcpy(tmp_copy, mime);

        type_subtype_separator_pos = strchr(tmp_copy, '/');
        subtype_options_separator_pos = strchr(tmp_copy, ';');
        end_pos = tmp_copy + strlen(mime);

        if (type_subtype_separator_pos)
            *type_subtype_separator_pos++ = '\0';
        else
            type_subtype_separator_pos = end_pos;

        if (subtype_options_separator_pos)
            *subtype_options_separator_pos++ = '\0';
        else
            subtype_options_separator_pos = end_pos;

        set_MIME_type(tmp_copy, file);
        set_MIME_subtype(type_subtype_separator_pos, file);
        set_MIME_options(subtype_options_separator_pos, file);

        delete []tmp_copy;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Embed::serialize(WT_File & file) const
{
    if ( (m_MIME_type.length() && !m_MIME_type.is_ascii())
        || (m_MIME_subtype.length() && !m_MIME_subtype.is_ascii())
        || (m_MIME_options.length() && !m_MIME_options.is_ascii()) )
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(Embed '"));

    //
    // We have started a quote. If we serialize the string
    // and the string contains any quotable characters then
    // we might end up with nested quotes. To avoid this
    // we'll just serialize the characters.
    //
    if( m_MIME_type.length() )
    {
        WD_CHECK( file.write( m_MIME_type.ascii() ) );
    }

    WD_CHECK (file.write("/"));

    if( m_MIME_subtype.length() )
    {
        WD_CHECK( file.write( m_MIME_subtype.ascii() ) );
    }

    WD_CHECK (file.write(";"));

    if( m_MIME_options.length() )
    {
        WD_CHECK( file.write( m_MIME_options.ascii() ) );
    }

    WD_CHECK (file.write("' "));

    if( m_description.length() )
    {   if( m_description.is_ascii() )
        {
            WD_CHECK (file.write_quoted_string(m_description.ascii(), WD_True));
            WD_CHECK (file.write((WT_Byte)' '));
        }
        else
        {   WD_CHECK (m_description.serialize(file));
            WD_CHECK (file.write((WT_Byte)' '));
        }
    }
    else
        WD_CHECK (file.write("'' "));

    if( m_filename.length() )
    {   if( m_filename.is_ascii() )
        {
            WD_CHECK (file.write_quoted_string(m_filename.ascii(), WD_True));
            WD_CHECK (file.write((WT_Byte)' '));
        }
        else
        {   WD_CHECK (m_filename.serialize(file,WD_False));
            WD_CHECK (file.write((WT_Byte)' '));
        }
    }
    else
        WD_CHECK (file.write("'' "));

    if( m_url.length() )
    {   if( m_url.is_ascii() )
        {   WD_CHECK (file.write((WT_Byte)'\''));
            WD_CHECK (file.write(m_url.ascii()));
            WD_CHECK (file.write("' "));
        }
        else
        {   WD_CHECK (m_url.serialize(file,WD_False));
            WD_CHECK (file.write((WT_Byte)' '));
        }
    }
    else
        WD_CHECK (file.write("''"));

    return file.write(")");
}
#else
WT_Result WT_Embed::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Embed::operator== (WT_Object const & object) const
{
    if (object.object_id() == Embed_ID)
    {
        if (m_incarnation == ((WT_Embed const &)object).m_incarnation)
            return WD_True;
        if (m_MIME_type == ((WT_Embed const &)object).m_MIME_type &&
            m_MIME_subtype == ((WT_Embed const &)object).m_MIME_subtype &&
            m_MIME_options == ((WT_Embed const &)object).m_MIME_options &&
            m_description == ((WT_Embed const &)object).m_description &&
            m_filename == ((WT_Embed const &)object).m_filename &&
            m_url == ((WT_Embed const &)object).m_url)
            return WD_True;
    }

    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Eating_Initial_Whitespace:

                WD_CHECK (file.eat_whitespace());
                m_stage = Gathering_MIME;
                // No Break

            case Gathering_MIME:

                WD_CHECK (m_MIME_type.materialize(file));
                set_whole_MIME(m_MIME_type.ascii(), file);

                m_stage = Eating_Post_MIME_Whitespace;
                // No Break

            case Eating_Post_MIME_Whitespace:

                WD_CHECK (file.eat_whitespace());
                m_stage = Gathering_Description;
                // No Break

            case Gathering_Description:

                WD_CHECK (m_description.materialize(file));
                m_stage = Eating_Post_Description_Whitespace;
                // No break

            case Eating_Post_Description_Whitespace:

                WD_CHECK (file.eat_whitespace());
                m_stage = Gathering_Filename;
                // No break

            case Gathering_Filename:

                WD_CHECK (m_filename.materialize(file));
                m_stage = Eating_Post_Filename_Whitespace;
                // No break

            case Eating_Post_Filename_Whitespace:

                WD_CHECK (file.eat_whitespace());
                m_stage = Gathering_URL;
                // No break

            case Gathering_URL:

                WD_CHECK (m_url.materialize(file));
                m_stage = Eating_Trailing_Whitespace;
                // No break

            case Eating_Trailing_Whitespace:

                WD_CHECK (opcode.skip_past_matching_paren(file));
                m_stage = Eating_Initial_Whitespace;
                // No break

            } // switch (m_stage)

        } break;
    } // switch (opcode_type)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::process(WT_File & file)
{
    WD_Assert(file.embed_action());
    return (file.embed_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::default_process(WT_Embed &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_type(char const * type, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_type = (unsigned char const *)type;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_type(WT_Unsigned_Integer16 const * type, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_type.set(WT_String::wcslen(type),type);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_subtype(char const * subtype, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_subtype = (unsigned char const*)subtype;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_subtype(WT_Unsigned_Integer16 const * subtype, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_subtype.set(WT_String::wcslen(subtype),subtype);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_options(char const * options, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_options = (unsigned char const*)options;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_MIME_options(WT_Unsigned_Integer16 const * options, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_MIME_options.set(WT_String::wcslen(options),options);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_description(char const * description, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_description = (unsigned char const*)description;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_description(WT_Unsigned_Integer16 const * description, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_description.set(WT_String::wcslen(description),description);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_filename(char const * filename, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_filename = (unsigned char const*) filename;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_filename(WT_Unsigned_Integer16 const * filename, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_filename.set(WT_String::wcslen(filename),filename);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_url(char const * url, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_url = (unsigned char const*)url;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embed::set_url(WT_Unsigned_Integer16 const * url, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_url.set(WT_String::wcslen(url),url);
    return WT_Result::Success;
}
