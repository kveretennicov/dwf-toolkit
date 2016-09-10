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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/unknown.cpp 1     9/12/04 8:57p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Unknown::object_id() const
{
    return WT_Object::Unknown_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Unknown::object_type() const
{
    return WT_Object::Unknown;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Unknown::set_pass_thru_bytes(WT_Unsigned_Integer16 size,WT_Byte* data)
{
    //Release memory if allocated
    release_pass_thru_bytes();

    if(size) 
    {
        pass_thru_bytes = new WT_Byte[size];
        if(!pass_thru_bytes)
            return WT_Result::Out_Of_Memory_Error;
        WD_COPY_MEMORY(data,size,pass_thru_bytes);
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Unknown::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    //We simply pass through the unknown opcode field bytes without
    //having to worry about the opcode field types.

    WT_Boolean save_compress = file.heuristics().allow_data_compression();
    file.heuristics().set_allow_data_compression(WD_False);
    if(((WT_Unknown &) *this).get_pass_thru_bytes()) {
        file.write(((WT_Unknown &) *this).get_pass_thru_byte_length(),
            ((WT_Unknown &) *this).get_pass_thru_bytes());
    }
    file.heuristics().set_allow_data_compression(save_compress);

    return WT_Result::Success;
}
#else
WT_Result WT_Unknown::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Unknown::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WD_CHECK (skip_operand(opcode, file));

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Unknown::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result reslt;
    release_pass_thru_bytes();

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            reslt = opcode.skip_past_matching_paren(file);
            //At this point the WT_File pointer would have read the
            //unknown opcode information fully and would be pointing
            //to the next opcode information start byte. We gather
            //information on the skipped unknown extended ascii opcode
            //from the 'opcode' field received as parameter to this function
            //and reading the opcode field information by seeking back by
            //the length that was skippped by our earlier call to
            //opcode.skip_past_matching_paren.
            if(opcode.size()) {
                total_opcode_size = (WT_Unsigned_Integer16) opcode.size() + file.skip_byte_length();
                pass_thru_bytes = new WT_Byte[opcode.size() + file.skip_byte_length()];
                if(!pass_thru_bytes)
                    return WT_Result::Out_Of_Memory_Error;
                memcpy(pass_thru_bytes, opcode.token(), opcode.size());

                int amount_seeked = 0;
                WD_CHECK( (file.stream_seek_action())(file, -file.skip_byte_length(), amount_seeked) );
                if (amount_seeked != -file.skip_byte_length())
                    return WT_Result::Internal_Error;
                file.read(file.skip_byte_length(), &(pass_thru_bytes[opcode.size()]));
            }
            return reslt;
        } break;
    case WT_Opcode::Extended_Binary:
        {
            WT_Integer32    size;
            WT_Byte    *    size_ptr = (WT_Byte *)&size;

            size_ptr[0] = (opcode.token())[1];
            size_ptr[1] = (opcode.token())[2];
            size_ptr[2] = (opcode.token())[3];
            size_ptr[3] = (opcode.token())[4];

            //at this point we would have already read extended binary
            //opcode start byte (i.e '{), opcode identifier (2 bytes),
            //and the opcode byte length (4 bytes) which makes it 7 bytes.

            if(size) {
                pass_thru_bytes = new WT_Byte[7 + size];
                if(!pass_thru_bytes)
                    return WT_Result::Out_Of_Memory_Error;
                //we copy the initial 7 bytes that we read as part of opcode
                //read and identification process.
                memcpy(pass_thru_bytes, opcode.token(), 7);
                total_opcode_size = (WT_Unsigned_Integer16) (7 + (size - WD_EXTENDED_BINARY_OPCODE_SIZE));
            }

            //we read the rest of the unknown opcode information to just
            //simply pass through (and not try to interpret the field types).

            if (size)
                file.read(size - WD_EXTENDED_BINARY_OPCODE_SIZE, (WT_Byte *) &(pass_thru_bytes[7]));
            else
                return WT_Result::Unsupported_DWF_Extension_Error;
        } break;
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Unknown::process(WT_File & file)
{
    WD_Assert (file.unknown_action());
    return (file.unknown_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Unknown::default_process(WT_Unknown &, WT_File &)
{
    return WT_Result::Success;
}
