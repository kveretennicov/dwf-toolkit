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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/url_list.cpp 1     9/12/04 8:57p Evansg $

#include "whiptk/pch.h"

#include "whiptk/url_list.h"

class WT_URL_Item;
///////////////////////////////////////////////////////////////////////////
namespace whiptk_url_list
{
    void copy_list(WT_URL_List &dest, WT_URL_List const & source)
    {
        dest.remove_all();

        if (source.is_empty())
            return;

        WT_URL_Item *current = (WT_URL_Item *) source.get_head();

        while (current != WD_Null)
        {
            WT_String address;
            WT_String name;
            address.set(current->address());
            name.set(current->friendly_name());
            WT_URL_Item new_item(current->index(), address, name);
            dest.add(new_item);
            current = (WT_URL_Item *) current->next();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
WT_URL_List::WT_URL_List(WT_URL_List const & url)
  : WT_Item_List()
  , WT_Object()
{
    whiptk_url_list::copy_list(*this, url);
}

///////////////////////////////////////////////////////////////////////////
WT_URL_List const & WT_URL_List::operator=(WT_URL_List const & url)
{
    whiptk_url_list::copy_list(*this, url);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_URL_List::operator==(WT_URL_List const & list) const
{
    if (count() != list.count())
        return WD_False;

    WT_URL_Item *current_me = (WT_URL_Item *) get_head();
    WT_URL_Item *current_other = (WT_URL_Item *) list.get_head();

    while (current_me)
    {
        if ( (current_me->address() != current_other->address()) ||
             (current_me->friendly_name() != current_other->friendly_name()) )

            return WD_False;

        current_me = (WT_URL_Item *) current_me->next();
        current_other = (WT_URL_Item *) current_other->next();
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_URL_List::object_id() const
{
    return URL_List_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_URL_List::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_URL_List::skip_operand (WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no url list opcode (this object is contained within the url object)
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result    WT_URL_List::process(WT_File & )
{
    // Return internal error.  This method should never be called
    //  as there is no url list opcode (this object is contained within the url object)
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
void WT_URL_List::add(WT_URL_Item & item)
{
    WT_URL_Item *item_local = new WT_URL_Item(item.index(), item.address(), item.friendly_name());
    WT_Item_List::add_tail(item_local);
}

///////////////////////////////////////////////////////////////////////////
WT_URL_List::~WT_URL_List()
{
    remove_all();
}

///////////////////////////////////////////////////////////////////////////
WT_URL_Item* WT_URL_List::url_item_from_index(WT_Integer32 index)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_URL_Item *item = (WT_URL_Item *) current;

        if (item->index() == index)
            return item;

        current = current->next();
    }

    return WD_Null;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Boolean WT_URL_List::check_whether_emitted(WT_Integer32 index) const
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_URL_Item *item = (WT_URL_Item *) current;

        if (item->index() == index)
            return item->get_emitted_status();

        current = current->next();
    }

    return WD_False;
}
#else
WT_Boolean WT_URL_List::check_whether_emitted(WT_Integer32) const
{
    return WD_False;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_URL_List::set_as_being_emitted(WT_Integer32 index)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_URL_Item *item = (WT_URL_Item *) current;

        if (item->index() == index)
        {
            item->set_emitted_status(WD_True);
            return;
        }

        current = current->next();
    }

}

///////////////////////////////////////////////////////////////////////////
WT_Integer32 WT_URL_List::index_from_url_item(WT_URL_Item & item_in)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_URL_Item *item = (WT_URL_Item *) current;

        if ( (item_in.address() == item->address()) && (item_in.friendly_name() == item->friendly_name()))
            return item->index();

        current = current->next();
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_URL_List::serialize(WT_File & file) const
{
    WT_URL_Item *   current     = (WT_URL_Item *) get_head();

    WD_Assert(current!=NULL);     // Shouldn't call this routine if there's nothing to do
    WD_Assert(count()>0);  // Shouldn't call this routine if there's nothing to do

    if (file.heuristics().target_version() < REVISION_WHEN_MULTI_URLS_WERE_SUPPORTED)
    {
        // Old way
        WD_CHECK (current->address().serialize(file));
    }
    else
    {
        // New way

        WT_URL_List & master_list = const_cast<WT_URL_List &>( file.desired_rendition().url_lookup_list() );

        while (current)
        {
            WT_Integer32 index = master_list.index_from_url_item( *current );
            
            if ( current->index() != index )
            {
                if (current->index() >= 0 && index < 0 )
                {
                    //couldn't find it, so re-key the index and send it out.
                    current->index() = master_list.count();
                    master_list.add( *current );
                }
                else
                {
                    current->index() = index;
                }
            }

            if (master_list.check_whether_emitted(current->index()))
            {
                WD_CHECK (file.write_ascii(current->index()));
            }
            else
            {
                WD_CHECK (file.write("("));
                WD_CHECK (file.write_ascii(current->index()));
                WD_CHECK (file.write((WT_Byte) ' '));
                WD_CHECK (current->address().serialize(file));
                WD_CHECK (file.write((WT_Byte) ' '));
                WD_CHECK (current->friendly_name().serialize(file));
                WD_CHECK (file.write(")"));
                file.desired_rendition().url_lookup_list().set_as_being_emitted(current->index());
            }

            current = (WT_URL_Item *) current->next();

            if (current)
                WD_CHECK (file.write((WT_Byte) ' '));
        }
    }

    return WT_Result::Success;
}
#else
WT_Result WT_URL_List::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_URL_List::materialize(WT_Opcode const &, WT_File &)
{
    // Return internal error.  This method should never be called
    //  as there is no url list opcode (this object is contained within the url object)

    //TODO: remove materialize processing from url object and move here, pattern this after the pointset code
    //      url object should inherit from url list
    return WT_Result::Internal_Error;
}


///////////////////////////////////////////////////////////////////////////
WT_URL_Item::WT_URL_Item(   WT_Integer32 index,
                            WT_String &  address,
                            WT_String &  friendly_name)
    : m_address(address)
    , m_friendly_name(friendly_name)
    , m_emitted(WD_False)
{
    m_index = index;
}

///////////////////////////////////////////////////////////////////////////
void WT_URL_Item::set(  WT_Integer32                    index,
                        WT_Unsigned_Integer16 const *   address,
                        WT_Unsigned_Integer16 const *   friendly_name)
{
    m_address.set(WT_String::wcslen(address), address);
    m_friendly_name.set(WT_String::wcslen(friendly_name), friendly_name);
    m_index = index;
    m_emitted = WD_False;
}

///////////////////////////////////////////////////////////////////////////
void WT_URL_Item::set(  WT_Integer32    index,
                        char const *    address,
                        char const *    friendly_name)
{
    m_index = index;
    m_address = address ? (WT_Byte*) address : WT_String::kNull;
    m_friendly_name = friendly_name ? (WT_Byte*) friendly_name : WT_String::kNull;
    m_emitted = WD_False;
}
