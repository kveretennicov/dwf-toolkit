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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/whiptk/attribute_url.cpp#1 $


#include "whiptk/pch.h"

WT_Attribute_URL::WT_Attribute_URL( WT_Attribute &attribute )
    : m_incarnation(-1)
    , m_current_index(0)
    , m_count (0)
    , m_attribute_id( attribute.object_id() )
    , m_stage(Eating_Initial_Whitespace)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Attribute_URL::WT_Attribute_URL( WT_Attribute &attribute, 
                WT_Integer32 index,
                WT_Unsigned_Integer16 const *   address,
                WT_Unsigned_Integer16 const *   friendly_name)
    : m_incarnation(-1)
    , m_count (0)
    , m_attribute_id( attribute.object_id() )
    , m_stage(Eating_Initial_Whitespace)
{
    set(index, address, friendly_name);
}

WT_Attribute_URL::WT_Attribute_URL( WT_Attribute &attribute, WT_URL_Item & item)
    : m_incarnation(-1)
    , m_count (0)
    , m_attribute_id( attribute.object_id() )
    , m_stage(Eating_Initial_Whitespace)
{
    add_url(item);
}

///////////////////////////////////////////////////////////////////////////
WT_Integer32 const& 
WT_Attribute_URL::attribute_id(void) const
{
    return m_attribute_id;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Attribute_URL::object_id() const
{
    return WT_Object::Attribute_URL_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Attribute_URL::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));
    WD_CHECK (file.desired_rendition().sync(file,WT_Rendition::Code_Page_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(AttributeURL"));
	if(m_attribute_id >= 0)
	{
		WD_CHECK (file.write(" <"));
		WD_CHECK (file.write_ascii(m_attribute_id));
		WD_CHECK (file.write(">"));
	}

    if (m_url_list.count())
    {
        WD_CHECK (file.write(" "));
        WD_CHECK (m_url_list.serialize(file));
    }

    WD_CHECK (file.write(")"));

    return WT_Result::Success;
}
#else
WT_Result WT_Attribute_URL::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_Attribute_URL::flush_rendition(WT_File & file)
{
    m_incarnation = file.next_incarnation();
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Attribute_URL::operator == (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Attribute_URL_ID)
    {
        if (m_incarnation == ((WT_Attribute_URL const &)attrib).m_incarnation
            && m_attribute_id == ((WT_Attribute_URL const &)attrib).m_attribute_id
            && m_url_list.count() == ((WT_Attribute_URL const &)attrib).m_url_list.count() 
            && m_url_list == ((WT_Attribute_URL const &)attrib).m_url_list )
            return WD_True;
    }

    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Attribute_URL::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().url())
    {
        file.rendition().attribute_url() = *this;
        return serialize(file);
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Attribute_URL::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Attribute_URL::materialize(WT_Opcode const & opcode, WT_File & file)
{
    m_incarnation = file.next_incarnation();

    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_MULTI_URLS_WERE_SUPPORTED)
    {
        switch (m_stage)
        {
        case Skipping_Last_Paren:
        case Getting_Next_Block:
        case Getting_Assigned_Index:
        case Getting_Address:
        case Getting_Friendly_Name:
        case Getting_Closing_Block_Paren:
        case Reading_Index:
        case Completed:
        {
            break;
        }
        case Eating_Initial_Whitespace:
            WD_CHECK (file.eat_whitespace());
            m_stage = Gathering_String;
            // No Break Here

        case Gathering_String:
            WD_CHECK (m_current_address.materialize(file));
            m_stage = Eating_End_Whitespace;
            // No Break Here

        case Eating_End_Whitespace:
            WD_CHECK (opcode.skip_past_matching_paren(file));
            m_stage = Eating_Initial_Whitespace;

            m_url_list.remove_all();
            add_url(m_count++, m_current_address, m_current_friendly_name);
        }
    }
    else
    {
        WT_Byte a_byte;

        switch (m_stage)
        {
        case Gathering_String:
        case Eating_End_Whitespace:
        case Skipping_Last_Paren:
        {
            break;
        }

LABEL_Eating_Initial_Whitespace:
        case Eating_Initial_Whitespace:
            WD_CHECK (file.eat_whitespace());
            m_stage = Getting_Next_Block;
            // No Break Here

        case Getting_Next_Block:

            WD_CHECK (file.read(a_byte));

            if ( a_byte == '(')
            {
                m_stage = Getting_Assigned_Index;
            }
            else if ( a_byte == ')')
            {
                file.put_back(a_byte);
                WD_CHECK (opcode.skip_past_matching_paren(file));
                m_materialized = WD_True;
                break;
            }
            else if ( a_byte == '\'')
            {
                file.put_back(a_byte);
                WD_CHECK (m_current_address.materialize(file));
                m_stage = Completed;
                goto LABEL_Finished;
            }
            else if (a_byte == '<')
            {
				goto LABEL_Reading_AttributeID;
            }
			else
            {
                file.put_back(a_byte);
                m_stage = Reading_Index;
                goto LABEL_Reading_Index;
            }
            // No break

        case Getting_Assigned_Index:

            WD_CHECK (file.read_ascii(m_current_index));
            m_stage = Getting_Address;
            // no break

        case Getting_Address:
            WD_CHECK (m_current_address.materialize(file));
            m_stage = Getting_Friendly_Name;
            // no break

        case Getting_Friendly_Name:
            WD_CHECK (m_current_friendly_name.materialize(file));
            m_stage = Getting_Closing_Block_Paren;
            // no break

        case Getting_Closing_Block_Paren:
            WD_CHECK (file.read(a_byte));
            if (a_byte == ')')
            {
                WT_URL_Item item(m_current_index, m_current_address, m_current_friendly_name);

                add_url(item);
                file.rendition().url_lookup_list().add(item);

                m_stage = Eating_Initial_Whitespace;
                goto LABEL_Eating_Initial_Whitespace;
            }
            else
            {
                return WT_Result::Corrupt_File_Error;
            }

LABEL_Reading_Index:
        case Reading_Index:
            {
                WD_CHECK (file.read_ascii(m_current_index));
                WT_URL_Item *item = file.rendition().url_lookup_list().url_item_from_index(m_current_index);
                if (item)
				{
                    add_url(-1, item->address(), item->friendly_name());
				}
                m_stage = Eating_Initial_Whitespace;
                goto LABEL_Eating_Initial_Whitespace;
            }

LABEL_Reading_AttributeID:
		case Reading_AttributeID:
			{
				WD_CHECK (file.read_ascii(m_attribute_id));
				WD_CHECK (file.read(a_byte));
				if (a_byte == '>')
				{
					m_stage = Eating_Initial_Whitespace;
					goto LABEL_Eating_Initial_Whitespace;
				}
				else
				{
					return WT_Result::Corrupt_File_Error;
				}
			}

LABEL_Finished:
        case Completed:
            WD_CHECK (opcode.skip_past_matching_paren(file));
            m_stage = Eating_Initial_Whitespace;
        } // switch (m_stage)

    }

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Attribute_URL::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Attribute_URL::process(WT_File & file)
{
    WD_Assert (file.attribute_url_action());
    return (file.attribute_url_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Attribute_URL::default_process(WT_Attribute_URL & item, WT_File & file)
{
    file.rendition().attribute_url() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Attribute_URL const & WT_Attribute_URL::operator=(WT_Attribute_URL const & url)
{
    m_url_list = url.m_url_list;
    m_incarnation = url.m_incarnation;
    m_attribute_id = url.m_attribute_id;
    return *this;
}

///////////////////////////////////////////////////////////////////////////
void WT_Attribute_URL::set(WT_Integer32 index, WT_Unsigned_Integer16 const *address, WT_Unsigned_Integer16 const *friendly_name)
{
    WT_URL_Item item;
    item.address().set(WT_String::wcslen(address), address);
    item.friendly_name().set(WT_String::wcslen(friendly_name), friendly_name);
    item.index() = index;
    add_url(item);
}


///////////////////////////////////////////////////////////////////////////
void WT_Attribute_URL::add_url_optimized(WT_URL_Item & item, WT_File & file)
{
    m_incarnation = file.next_incarnation();

    WT_URL_Item tmp_item(item.index(), item.address(), item.friendly_name());

    WT_Integer32 index = file.desired_rendition().url_lookup_list().index_from_url_item(tmp_item);

    if (index >= 0)
    {
        tmp_item.index() = index;
    }
    else
    {
       tmp_item.index() = file.desired_rendition().url_lookup_list().count();
       tmp_item.set_emitted_status(WD_False);
       file.desired_rendition().url_lookup_list().add(tmp_item);
    }

    m_url_list.add(tmp_item);
}
