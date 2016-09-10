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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/compdata.cpp 1     9/12/04 8:52p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Compressed_Data_Moniker::object_id() const
{
    return Compressed_Data_Moniker_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Compressed_Data_Moniker::object_type() const
{
    return Compression;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Compressed_Data_Moniker::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    // Write the extended binary opcode for the compression object
    WT_Byte    tmp_byte = '{';

    // Note that we don't call the higer level write methods since they may try
    // and buffer and compress the compression code itself, which we don't want.
    WD_CHECK ((file.stream_write_action())(file, sizeof(WT_Byte), &tmp_byte));

    // Write a extended binary opcode length field.  A value of zero
    // means that this can't be skipped and it is up to the reader to know
    // when the opcode terminates.

    WT_Integer32    tmp_long = 0;
    WD_CHECK ((file.stream_write_action())(file, sizeof(WT_Integer32), &tmp_long));

    // Write the extended binary opcode code.
    WT_Unsigned_Integer16    tmp_short;

    if (file.heuristics().target_version() < REVISION_WHEN_ZLIB_COMPRESSION_IS_SUPPORTED)
        tmp_short = WD_LZ_COMPRESSION_EXT_OPCODE;
    else
        tmp_short = WD_ZLIB_COMPRESSION_EXT_OPCODE;

    tmp_short = WT_Endian::adjust(tmp_short);
    return (file.stream_write_action())(file, sizeof(WT_Unsigned_Integer16), &tmp_short);
}
#else
WT_Result WT_Compressed_Data_Moniker::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Compressed_Data_Moniker::materialize(WT_Opcode const & opcode, WT_File &)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        {
            // Nothing to do.  The process function for this object will set the file flags
            // such that data decompression starts.
        } break;
    case WT_Opcode::Single_Byte:
    case WT_Opcode::Extended_ASCII:
    default:
        {
            // Error, this opcode is not legal for this object.
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Compressed_Data_Moniker::skip_operand(WT_Opcode const &, WT_File &)
{
    // TODO: need to implement code that skips, to do so it would be helpfull if when the compression
    // opcode is writen it specifies an extended binary opcode skip count that is valid.
    return WT_Result::Internal_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Compressed_Data_Moniker::process(WT_File & file)
{
    WD_Assert(file.compressed_data_moniker_action());
    return (file.compressed_data_moniker_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Compressed_Data_Moniker::default_process(WT_Compressed_Data_Moniker & item,
                                                      WT_File & file)
{
    if (file.data_decompression())
        return WT_Result::Corrupt_File_Error; // Can't have nested compressors (yet).

    file.set_data_decompression(item.m_compression_format);
    return WT_Result::Success;
}
