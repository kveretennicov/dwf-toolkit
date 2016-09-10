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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/guid_list.cpp 1     9/12/04 8:53p Evansg $

#include "whiptk/pch.h"

#include "whiptk/guid_list.h"

///////////////////////////////////////////////////////////////////////////
WT_Guid_List const &
    WT_Guid_List::operator=(WT_Guid_List const & list)
{
    remove_all();

    if (list.is_empty())
        return *this;

    WT_Guid *current =
        (WT_Guid *) list.get_head();

    while (current != WD_Null)
    {
        WT_Guid new_item(*current);
        add(new_item);
        current = (WT_Guid *) current->next();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid_List::process(WT_File & file)
{
    WD_Assert(file.guid_list_action());
    return (file.guid_list_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean
    WT_Guid_List::operator==(WT_Guid_List const & list) const
{
    if (count() != list.count())
        return WD_False;

    WT_Guid *current_me = (WT_Guid *) get_head();
    WT_Guid *current_other =
        (WT_Guid *) list.get_head();

    while (current_me)
    {
        if ( current_me != current_other )
            return WD_False;

        current_me = (WT_Guid *) current_me->next();
        current_other = (WT_Guid *) current_other->next();
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Guid_List::object_id() const
{
    return Guid_List_ID;
}


///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Guid_List::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
WT_Unsigned_Integer16    WT_Guid_List::get_total_opcode_binary_size()
{
    WT_Integer32 total_guids_size = 0;
    WT_Integer32 item_count;
    item_count = count();
    if(item_count > 0)
        total_guids_size = BINARYSIZEOFGUID * item_count;
    return (WT_Unsigned_Integer16)(
            (sizeof(WT_Byte) +//for '{')
            sizeof(WT_Integer32) +//for opcode integer size
            sizeof(WT_Unsigned_Integer16) + //for opcode
            sizeof(WT_Integer32) + //for total number of guids
            total_guids_size +
            sizeof(WT_Byte)));  // The closing "}"
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid_List::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
void WT_Guid_List::add(WT_Guid & item)
{
    WT_Guid *item_local =
        new WT_Guid(item);
    WT_Item_List::add_tail(item_local);
}

///////////////////////////////////////////////////////////////////////////
WT_Guid_List::~WT_Guid_List()
{
    remove_all();
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid_List::default_process(WT_Guid_List &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Guid_List::serialize(WT_File & file) const
{
    WT_Guid *   current     = (WT_Guid *) get_head();
    WT_Integer32    item_count  = count();

    // Shouldn't call the following two routines if there's nothing to do
    WD_Assert(current);
    WD_Assert(item_count);

    {
        // New way

        WD_CHECK (file.dump_delayed_drawable());

        file.desired_rendition().blockref();
        WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

        if (file.heuristics().allow_binary_data())
        {
            WD_CHECK (file.write((WT_Byte) '{'));
            WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                                 sizeof(WT_Integer32) +          // guid list count
                                                 BINARYSIZEOFGUID * item_count +
                                                 sizeof(WT_Byte)                 // The closing "}"
                                                )));
            WD_CHECK (file.write((WT_Unsigned_Integer16) WD_EXBO_GUID_LIST ));
            WD_CHECK (file.write(item_count));
            while (current)
            {
                WD_CHECK (current->serialize(file));
                current = (WT_Guid *) current->next();
            }
            WD_CHECK (file.write("}"));
        }
        else {
            WD_CHECK (file.write_geom_tab_level());
            WD_CHECK (file.write("(GuidList "));
            WD_CHECK (file.write_ascii(item_count));
            WD_CHECK (file.write(" "));

            while (current)
            {
                {
                    WD_CHECK (current->serialize(file));
                    WD_CHECK (file.write((WT_Byte) ' '));
                }

                current = (WT_Guid *) current->next();
            }
            WD_CHECK (file.write(")"));
        }
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Guid_List::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid_List::materialize(WT_Opcode const & opcode,
                                            WT_File & file)
{
    #if DESIRED_CODE(WHIP_INPUT)
    WT_Integer32 item_count = 0;
    WT_Opcode optioncode;
    WT_Guid *guid;
    int i = 0;

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Starting:
                m_stage = Getting_No_Of_Guids;
                // No break;

            case Getting_No_Of_Guids:
                WD_CHECK (file.read_ascii(item_count));
                if(item_count <= 0)
                {
                    m_stage = Eating_End_Whitespace;
                    goto end_ascii;
                }
                m_stage = Getting_Guid;

                // No break
            case Getting_Guid:
                for(i=0; i< item_count; i++) {
                    guid = WD_Null;
                    guid = new WT_Guid();
                    if (!guid)
                        return WT_Result::Out_Of_Memory_Error;

                    WD_CHECK (optioncode.get_opcode(file));
                    WD_CHECK (guid->materialize(optioncode, file));
                    add(*guid);
                    delete guid;
                    m_stage = Getting_Guid;
                }
                m_stage = Getting_Close;
end_ascii:
            case Eating_End_Whitespace:
                WD_CHECK (opcode.skip_past_matching_paren(file));
                m_stage = Starting;
                break;

            default:
                return WT_Result::Internal_Error;
            }
        } break;
    case WT_Opcode::Extended_Binary:
        {
            switch (m_stage)
            {
            case Starting:
                m_stage = Getting_No_Of_Guids;
                // No break;

            case Getting_No_Of_Guids:
                WD_CHECK (file.read(item_count));
                if(item_count < 0)
                    return WT_Result::Internal_Error;

                m_stage = Getting_Guid;

                // No break
            case Getting_Guid:
                for(i=0; i< item_count; i++) {
                    guid = WD_Null;
                    guid = new WT_Guid();
                    if (!guid)
                        return WT_Result::Out_Of_Memory_Error;

                    WD_CHECK (optioncode.get_opcode(file));
                    WD_CHECK (guid->materialize(optioncode, file));
                    add(*guid);
                    delete guid;
                    m_stage = Getting_Guid;
                }
            m_stage = Getting_Close;

            // No break
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

                // Yeah!! We succeeded!
                break;  // Go below to clean up...
            default:
                return WT_Result::Internal_Error;  // Illegal stage.
            }
        } break;
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
