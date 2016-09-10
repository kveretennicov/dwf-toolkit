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
// $Header:


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif

///////////////////////////////////////////////////////////////////////////
WT_SignData::WT_SignData(
            WT_Guid_List                block_guid_list,
            WT_Integer32                data_size,
            WT_Byte *                   data,
            WT_Boolean                  copy) throw(WT_Result)
            : m_data_size(data_size)
            , m_data(WD_Null)
            , m_local_data_copy(copy)
            , m_stage(Starting)
{
    WD_Assert (data_size && data);

    m_block_guid_list.remove_all();
    m_block_guid_list = block_guid_list;

    if (copy)
    {
        m_local_data_copy = copy;
        m_data = new WT_Byte[data_size];

        if (m_data)
            memcpy(m_data, data, data_size);
        else
            throw WT_Result::Out_Of_Memory_Error;
    }  // if (copy)
    else
    {
        m_data = data;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_SignData::object_id() const
{
    return SignData_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_SignData::serialize(WT_File & file) const
{
    if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        if(m_block_guid_list.count() >0) {
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Byte)  +               // for signalling the presence/absence of a block list
                                             ((WT_SignData *)this)->m_block_guid_list.get_total_opcode_binary_size() +   // data description
                                             sizeof(WT_Integer32) +          // the data size
                                             m_data_size +                   // the sign data
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));

        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_SIGNDATA));

            WD_CHECK (file.write((WT_Byte) '1'));
            WD_CHECK (m_block_guid_list.serialize(file));
        }
        else {
            WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Byte)  +               // for signalling the presence/absence of a block list
                                             sizeof(WT_Integer32) +          // the data size
                                             m_data_size +                   // the sign data
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));

            WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_SIGNDATA));
            WD_CHECK (file.write((WT_Byte) '0'));
        }

        WD_CHECK (file.write(m_data_size));
        if(m_data_size) {
            WD_CHECK (file.write(m_data_size, m_data));
        }
        WD_CHECK (file.write((WT_Byte) '}'));

    }
    else
    {
        // Extended ASCII output

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(SignData "));

        if(m_block_guid_list.count() >0) {
            WD_CHECK (file.write((WT_Byte) '1'));
            WD_CHECK (m_block_guid_list.serialize(file));
        }
        else
            WD_CHECK (file.write((WT_Byte) '0'));

        WD_CHECK (file.write((WT_Byte) ' '));

        WD_CHECK (file.write_ascii(m_data_size));
        WD_CHECK (file.write((WT_Byte) ' '));

        if(m_data_size) {
            WD_CHECK (file.write_hex(m_data_size, m_data));
        }
        WD_CHECK (file.write((WT_Byte) ' '));

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_SignData::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_SignData::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Opcode optioncode;
    WT_Byte block_guid_list_flag_ascii = '0';
    WT_Integer32 block_guid_list_flag_binary = 0;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Block_List_Flag;
            // No break;

        case Getting_Block_List_Flag:
            WD_CHECK (file.read(block_guid_list_flag_ascii));
            if(block_guid_list_flag_ascii == '0') {
                m_stage = Getting_Data_Size;
                goto Hop_Getting_Data_Size_ASCII;
            }
            m_stage = Getting_Block_List;


        case Getting_Block_List:
            WD_CHECK (optioncode.get_opcode(file));
            WD_CHECK (m_block_guid_list.materialize(optioncode, file));
            m_stage = Getting_Data_Size;
            //No Break

Hop_Getting_Data_Size_ASCII:
        case Getting_Data_Size:
            WD_CHECK (file.read(m_data_size));
            m_stage = Getting_Data;
            // No break

        case Getting_Data:
            if ((!m_data) && (m_data_size > 0))
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            if(m_data_size > 0) {
                WD_CHECK (file.read(m_data_size, m_data));
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
        } // switch (m_stage) for Binary
        break;

    case WT_Opcode::Extended_ASCII:

        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Block_List_Flag;
            // No break;

        case Getting_Block_List_Flag:
            WD_CHECK (file.read_ascii(block_guid_list_flag_binary));
            if(!block_guid_list_flag_binary) {
                m_stage = Getting_Data_Size;
                goto Hop_Getting_Data_Size_Binary;
            }
            m_stage = Getting_Block_List;


        case Getting_Block_List:
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (optioncode.get_opcode(file));
            WD_CHECK (m_block_guid_list.materialize(optioncode, file));
            m_stage = Getting_Data_Size;
            //No Break

Hop_Getting_Data_Size_Binary:
        case Getting_Data_Size:
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(m_data_size));
            m_stage = Getting_Data;

            // No break
        case Getting_Data:
            WD_CHECK(file.eat_whitespace());
            if (!m_data)
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read_hex(m_data_size, m_data));

            m_stage = Getting_Close;

            // No break

        case Getting_Close:

            // There will be two close parens: one for the data,
            // one for the opcode
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

///////////////////////////////////////////////////////////////////////////
WT_Result WT_SignData::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_SignData::process(WT_File & file)
{
    WD_Assert (file.signdata_action());
    return (file.signdata_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_SignData::default_process(WT_SignData &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_SignData::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
//helper function to get the blockrefs from block ids for every block
//that was part of the signature data.
WT_BlockRef_List WT_SignData::get_blockref_list(WT_Guid_List & guid_list,
                                                WT_File &file)
{
    WD_Assert(file.stream_user_data());
    WD_Assert( (file.file_mode() == WT_File::File_Read) ||
               (file.file_mode() == WT_File::Block_Read));

    WT_BlockRef_List blockref_list;
    WT_Integer32    list_count = guid_list.count();
    WT_Integer32    item_count = file.get_directory().count();
    if((list_count <= 0) || (item_count <= 0))
        return blockref_list;
    WT_Guid *currentguid = (WT_Guid *) guid_list.get_head();
    while (currentguid) {
        WT_BlockRef *   currentnode
            = (WT_BlockRef *) file.get_directory().get_head();
        while (currentnode)
        {
            if(currentnode->get_block_guid() == *currentguid)
                blockref_list.add(*currentnode);
            currentnode = (WT_BlockRef *) currentnode->next();
        }
        currentguid = (WT_Guid *) currentguid->next();
    }
    return blockref_list;
}
