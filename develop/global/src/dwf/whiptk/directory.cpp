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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/directory.cpp 1     9/12/04 8:52p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Directory const &
    WT_Directory::operator=(WT_Directory const & directory)
{
    remove_all();

    m_file_offset = ((WT_Directory &)directory).get_file_offset();
    if (directory.is_empty())
        return *this;

    WT_BlockRef *current = (WT_BlockRef *) directory.get_head();

    while (current != WD_Null)
    {
        WT_BlockRef new_item(*current);
        add(new_item);
        current = (WT_BlockRef *) current->next();
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Directory::process(WT_File & file)
{
    WD_Assert(file.directory_action());
    return (file.directory_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean
    WT_Directory::operator==(WT_Directory const & list) const
{
    if (count() != list.count())
        return WD_False;

    WT_BlockRef *current_me    = (WT_BlockRef *) get_head();
    WT_BlockRef *current_other = (WT_BlockRef *) list.get_head();

    while (current_me)
    {
        if ( (*current_me) != (*current_other) )
            return WD_False;

        current_me = (WT_BlockRef *) current_me->next();
        current_other = (WT_BlockRef *) current_other->next();
    }

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Directory::object_id() const
{
    return Directory_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type    WT_Directory::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Directory::skip_operand(WT_Opcode const & opcode,
                                             WT_File & file)
{
    switch(opcode.type())
    {
        case WT_Opcode::Extended_ASCII:
            WD_CHECK(opcode.skip_past_matching_paren(file));
            break;
        case WT_Opcode::Extended_Binary:
        {
            WT_Integer32 item_count = 0;
            WT_Integer32 loopcount  = 0;
            WT_BlockRef block_ref;
            WT_Opcode optioncode;
            WT_Unsigned_Integer32 file_offset;

            switch (m_stage)
            {
            case Starting:
                m_stage = Getting_BlockRef_Count;
                // No break;

            case Getting_BlockRef_Count:
                WD_CHECK (file.read(item_count));
                m_stage = Getting_BlockRefs;

                // No break
            case Getting_BlockRefs:
                for(loopcount=0; loopcount < item_count; loopcount++) {
                    WD_CHECK (optioncode.get_opcode(file));
                    int    ext_bin_value;
                    ext_bin_value = ((optioncode.token())[6] << 8) + (optioncode.token())[5];
                    block_ref.set_format((WT_BlockRef::WT_BlockRef_Format) ext_bin_value);
                    WD_CHECK (block_ref.materialize(optioncode, file, WD_True));
                }
                m_stage = Getting_Directory_File_Offset;

                // No break
            case Getting_Directory_File_Offset:
                WD_CHECK (file.read(file_offset));
                m_stage = Getting_Close;

                // No break
            case Getting_Close:
                WT_Byte close_brace;
                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

                // Yeah!! We succeeded!
                break;
                default:
                    return WT_Result::Internal_Error;  // Illegal stage.
            } // switch (m_stage) for Binary

            m_stage = Starting;  // We finished, so reset stage to start.
        }
        default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;  // Illegal Opcode Type
        }
    }
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Directory::add(WT_BlockRef & item)
{
    WT_BlockRef *item_local = WD_Null;
    item_local = new WT_BlockRef(item);
    if(!item_local)
        return WT_Result::Out_Of_Memory_Error;

    WT_Item_List::add_tail(item_local);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Directory::~WT_Directory()
{
    remove_all();
}

///////////////////////////////////////////////////////////////////////////
WT_BlockRef* WT_Directory::blockref_from_index(WT_Integer32 index)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_BlockRef *item = (WT_BlockRef *) current;

        if (item->index() == index)
            return item;

        current = current->next();
    }

    return WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Directory::default_process(WT_Directory & list, WT_File & file)
{
    file.set_directory(list);
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Integer32 WT_Directory::index_from_blockref(WT_BlockRef & item_in)
{
    WT_Item * current = get_head();

    while (current != WD_Null)
    {
        WT_BlockRef *item = (WT_BlockRef *) current;

        if ( item_in.get_block_guid() == item->get_block_guid() )
            return item->index();

        current = current->next();
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Directory::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    WT_BlockRef *   current     = (WT_BlockRef *) get_head();
    WT_Integer32    item_count  = count();

    WD_CHECK (file.dump_delayed_drawable());

    WT_Boolean compressor_stop_flag = WD_False;

    //by turning on the file heuristics to not allow compression
    //we will be writing this opcode in uncompressed form always

    compressor_stop_flag = file.heuristics().allow_data_compression();
    file.heuristics().set_allow_data_compression(WD_False);

    if (file.heuristics().allow_binary_data())
    {
        WT_Integer32 directory_opcode_size = 0;

        WT_BlockRef *   currentitem     = (WT_BlockRef *) get_head();
        while (currentitem)
        {
            directory_opcode_size +=
                currentitem->get_total_binary_opcode_size();
            currentitem = (WT_BlockRef *) currentitem->next();
        }

        directory_opcode_size += sizeof(WT_Unsigned_Integer16); //for opcode
        directory_opcode_size += sizeof(WT_Integer32);          //for blockref count
        directory_opcode_size += sizeof(WT_Unsigned_Integer32); //for blockref list file offset
        directory_opcode_size += sizeof(WT_Byte); //for '}'
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK((file.stream_tell_action())(file,
            (unsigned long *)&m_file_offset));

        ((WT_Directory *) this)->m_file_offset -= sizeof(WT_Byte); //we omit '}'

        WD_CHECK (file.write((WT_Integer32) directory_opcode_size));
        WD_CHECK (file.write((WT_Unsigned_Integer16) WD_EXBO_DIRECTORY));
        WD_CHECK (file.write((WT_Integer32) item_count));

        if(item_count > 0) {
            WD_CHECK(file.set_block_size_for_tail_blockref(
                ((WT_Directory *) this)->get_file_offset()));
        }

        while (current != WD_Null)
        {
            WD_CHECK(current->serialize(file, WD_True, WD_False));
            current = (WT_BlockRef *) current->next();
        }
        WD_CHECK (file.write((WT_Unsigned_Integer32)
            (((WT_Directory *) this)->get_file_offset())));
        WD_CHECK (file.write("}"));
    }
    else { //ASCII

        // New way
        WD_CHECK (file.dump_delayed_drawable());
        WD_CHECK (file.write_tab_level());
        WD_CHECK((file.stream_tell_action())(file,
            (unsigned long *)&m_file_offset));

        //reduce the file offset by the size of characters that would
        //have been possibly written by the call to write_tab_level fn.
        ((WT_Directory *) this)->m_file_offset -=
            ((file.tab_level() + (int)strlen(WD_NEWLINE) ) * sizeof(WT_Byte));

        WD_CHECK (file.write("(Directory "));
        WD_CHECK (file.write_padded_ascii(item_count));
        WD_CHECK (file.write((WT_Byte) ' '));
        if(item_count > 0) {
            WD_CHECK( file.set_block_size_for_tail_blockref(
                ((WT_Directory *) this)->get_file_offset()) );
        }

        while (current != WD_Null)
        {
            {
                WD_CHECK( current->serialize(file, WD_True, WD_False) );
            }

            current = (WT_BlockRef *) current->next();

            if (current)
                WD_CHECK (file.write((WT_Byte) ' '));
        }
        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_padded_ascii((WT_Unsigned_Integer32)
            ((WT_Directory *) this)->get_file_offset()));
        WD_CHECK (file.write(")"));
    }

    file.heuristics().set_allow_data_compression(compressor_stop_flag);
    return WT_Result::Success;
}
#else
WT_Result WT_Directory::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Directory::materialize(WT_Opcode const & opcode,
                                            WT_File & file)
{
    WT_Integer32 item_count = 0;
    WT_Integer32 loopcount  = 0;

    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_BlockRef_Count;
            // No break;

        case Getting_BlockRef_Count:
            WD_CHECK (file.read(item_count));
            m_stage = Getting_BlockRefs;

            // No break
        case Getting_BlockRefs:
            for(loopcount=0; loopcount < item_count; loopcount++) {
                m_optioncode = WD_Null;
                m_optioncode = new WT_Opcode;
                if (!m_optioncode)
                    return WT_Result::Out_Of_Memory_Error;
                WD_CHECK (m_optioncode->get_opcode(file));
                WT_BlockRef *block_ref = NULL;
                block_ref = new WT_BlockRef();
                if (!block_ref)
                    return WT_Result::Out_Of_Memory_Error;
                int    ext_bin_value;
                ext_bin_value = ((m_optioncode->token())[6] << 8) +
                    (m_optioncode->token())[5];
                block_ref->set_format((WT_BlockRef::WT_BlockRef_Format)
                    ext_bin_value);
                WD_CHECK (block_ref->materialize(*m_optioncode,
                    file, WD_True));
                // we accumulate the blockref's contained in the directory as they materialize.
                WD_CHECK( add(*block_ref) );
                delete block_ref;
                block_ref = WD_Null;
                delete m_optioncode;
                m_optioncode = WD_Null;
            }
            m_stage = Getting_Directory_File_Offset;

            // No break
        case Getting_Directory_File_Offset:
            WD_CHECK (file.read(m_file_offset));
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
        } // switch (m_stage) for Binary

        break;

    case WT_Opcode::Extended_ASCII:

        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_BlockRef_Count;
            // No break;

        case Getting_BlockRef_Count:
            WD_CHECK (file.read_ascii(item_count));
            m_stage = Getting_BlockRefs;

            // No break
        case Getting_BlockRefs:
            for(loopcount=0; loopcount < item_count; loopcount++) {
                m_optioncode = WD_Null;
                m_optioncode = new WT_Opcode;
                if (!m_optioncode)
                    return WT_Result::Out_Of_Memory_Error;
                WD_CHECK (m_optioncode->get_opcode(file));
                WT_BlockRef *block_ref = NULL;
                block_ref = new WT_BlockRef();
                if (!block_ref)
                    return WT_Result::Out_Of_Memory_Error;
                WD_CHECK (block_ref->materialize(*m_optioncode,
                    file, WD_True));
                // we accumulate the blockref's contained in the directory as they materialize.
                WD_CHECK( add(*block_ref) );
                delete block_ref;
                block_ref = WD_Null;
                delete m_optioncode;
                m_optioncode = WD_Null;
            }
            m_stage = Getting_Directory_File_Offset;

            // No break
        case Getting_Directory_File_Offset:
            WD_CHECK (file.read_ascii(m_file_offset));
            m_stage = Getting_Close;

            // No break
        case Getting_Close:

            // There will be two close parens: one for the data, one for the opcode
            WD_CHECK (opcode.skip_past_matching_paren(file));

            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.

        } // switch (m_stage) for ASCII

        break;

    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;  // Illegal Opcode Type
    } // switch (opcode type)

    m_stage = Starting;  // We finished, so reset stage to start.

    m_materialized = WD_True;
    return WT_Result::Success;
}
