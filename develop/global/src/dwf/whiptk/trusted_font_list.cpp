//  Copyright (c) 2001-2006 by Autodesk, Inc.
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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/trusted_font_list.cpp 1     9/12/04 8:57p Evansg $

#include "whiptk/pch.h"

#include "whiptk/trusted_font_list.h"

///////////////////////////////////////////////////////////////////////////
WT_Trusted_Font_List const &
    WT_Trusted_Font_List::operator=(WT_Trusted_Font_List const & trusted_font)
{
    remove_all();

    if (trusted_font.is_empty())
        return *this;

    WT_Trusted_Font_Item *current =
        (WT_Trusted_Font_Item *) trusted_font.get_head();

    while (current != WD_Null)
    {
        WT_String font_name;
        font_name.set(current->font_name());
        WT_Trusted_Font_Item new_item(current->index(), font_name);
        add(new_item);
        current = (WT_Trusted_Font_Item *) current->next();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Trusted_Font_List::process(WT_File & file)
{
    WD_Assert(file.trusted_font_list_action());
    return (file.trusted_font_list_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean
    WT_Trusted_Font_List::operator==(WT_Trusted_Font_List const & list) const
{
    if (count() != list.count())
        return WD_False;

    WT_Trusted_Font_Item *current_me = (WT_Trusted_Font_Item *) get_head();
    WT_Trusted_Font_Item *current_other =
        (WT_Trusted_Font_Item *) list.get_head();

    while (current_me)
    {
        if ( current_me->font_name() != current_other->font_name() )
            return WD_False;

        current_me = (WT_Trusted_Font_Item *) current_me->next();
        current_other = (WT_Trusted_Font_Item *) current_other->next();
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Trusted_Font_List::object_id() const
{
    return Trusted_Font_List_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Trusted_Font_List::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Trusted_Font_List::skip_operand(WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no named_view_list opcode
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Trusted_Font_List::add(WT_Trusted_Font_Item & item)
{
    WT_Trusted_Font_Item *item_local =
        new WT_Trusted_Font_Item(item.index(), item.font_name());
    WT_Item_List::add_tail(item_local);
}

///////////////////////////////////////////////////////////////////////////
WT_Trusted_Font_List::~WT_Trusted_Font_List()
{
    remove_all();
}

///////////////////////////////////////////////////////////////////////////
WT_Trusted_Font_Item*
    WT_Trusted_Font_List::trusted_font_item_from_index(WT_Integer32 index)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_Trusted_Font_Item *item = (WT_Trusted_Font_Item *) current;

        if (item->index() == index)
            return item;

        current = current->next();
    }

    return WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Trusted_Font_List::default_process(WT_Trusted_Font_List &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Integer32 WT_Trusted_Font_List::
    index_from_trusted_font_item(WT_Trusted_Font_Item & item_in)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_Trusted_Font_Item *item = (WT_Trusted_Font_Item *) current;

        if ( item_in.font_name() == item->font_name() )
            return item->index();

        current = current->next();
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Trusted_Font_List::serialize(WT_File & file) const
{
    WT_Trusted_Font_Item *   current     = (WT_Trusted_Font_Item *) get_head();

    // Shouldn't call the following two routines if there's nothing to do
    WD_Assert(current!=NULL);
    WD_Assert(count()!=0);

    //ASCII mode
    {
        // New way

        WD_CHECK (file.dump_delayed_drawable());

        file.desired_rendition().blockref();
        WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(TrustedFontList "));

        while (current)
        {
            {
                WD_CHECK (file.write_quoted_string(current->font_name().ascii()
                    , WD_True));
                WD_CHECK (file.write((WT_Byte) ' '));
            }

            current = (WT_Trusted_Font_Item *) current->next();

            if (current)
                WD_CHECK (file.write((WT_Byte) ' '));
        }
        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Trusted_Font_List::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Trusted_Font_List::materialize(WT_Opcode const & opcode,
                                            WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            WT_Byte     a_paren = '\\';
            WT_Integer32 index = 0;
            char *  a_string;
            WT_String font_name;
            WT_Byte a_byte;

            while (a_paren != ')')
            {
                a_string = WD_Null;
                index = 0;
                font_name = "";
                a_byte = '\0';
                WD_CHECK (file.read(a_string, 65535));
                WD_CHECK (file.eat_whitespace());
                font_name = a_string;
                WT_Trusted_Font_Item item(index, font_name);
                add(item);
                index++;
                WD_CHECK(file.read(a_paren));
                if(a_paren != ')')
                    file.put_back(a_paren);
            }
        } break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Trusted_Font_Item::WT_Trusted_Font_Item(   WT_Integer32 index,
                            WT_String &  font_name)
                            : m_font_name(font_name)
{
    m_index = index;
}

///////////////////////////////////////////////////////////////////////////
void WT_Trusted_Font_Item::set(  WT_Integer32       index,
                        WT_Unsigned_Integer16 const *   font_name)
{
    m_font_name.set(WT_String::wcslen(font_name), font_name);
    m_index = index;
}

///////////////////////////////////////////////////////////////////////////
void WT_Trusted_Font_Item::set(  WT_Integer32    index,
                        char const *    font_name)
{
    m_index = index;
    m_font_name = font_name ? (WT_Byte*) font_name : WT_String::kNull;
}
