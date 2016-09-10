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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/lz77decp.cpp 3     5/09/05 12:42a Evansg $


#include "whiptk/pch.h"

extern WT_Byte const   WD_History_Buffer_Preload[];
extern int const       WD_History_Buffer_Preload_Size;
extern const WT_RGBA32 WD_Old_Default_Palette[256];
extern const WT_RGBA32 WD_New_Default_Palette[256];

///////////////////////////////////////////////////////////////////////////
void WT_LZ_DeCompressor::preload_recall_buffer()
{

    if (m_file.rendition().drawing_info().decimal_revision() >= REVISION_WHEN_HISTORY_BUFFER_PRELOADED)
    {
        // Starting with revision 00.23, the history buffer is preloaded with expected
        // data to increase compression.

        // Note that we preload the old default colormap in order to be backward compatible,
        // but this colormap won't get used with newer files.
        WD_Assert(((WD_History_Buffer_Preload_Size - 1) + sizeof(WD_Old_Default_Palette) + 1) < WD_LZ_HISTORY_BUFFER_SIZE);

        // Preload the recall buffer with commonly found strings.
        // Don't include the trailing null terminator.
        m_recall_buffer.add(WD_History_Buffer_Preload_Size - 1, WD_History_Buffer_Preload);
    }

}

///////////////////////////////////////////////////////////////////////////
// similar to WD_CHECK, but allows Waiting_For_Data to be considered successful
#define WD_CHECK_W(wait_ok,x) do {                                                  \
                        WT_Result       result = (x);                               \
                        if ((wait_ok) && result == WT_Result::Waiting_For_Data)     \
                            return WT_Result::Success;                              \
                        if ((!wait_ok) && result == WT_Result::End_Of_File_Error)   \
                            continue;                                               \
                        if (result != WT_Result::Success)                           \
                            return result;                                          \
                    } while (0)                                                    //

///////////////////////////////////////////////////////////////////////////
WT_Result WT_LZ_DeCompressor::decompress(int desired, int & actual, void * users_buffer)
{
    WD_Assert(desired > 0);
    WD_Assert(users_buffer);

    WT_Byte *    buffer = (WT_Byte *) users_buffer;

    int    num_read = 0;    // A dummy local variable to test that our small reads worked.

    // We haven't read any data yet.  So say so...
    actual = 0;

    while (1)
    {
        switch (m_next_byte)
        {
        case Compression_Code:
            {
                WT_Byte    compression_code;

                // Read the code
                WD_CHECK_W (actual,
                            (m_file.stream_read_action())(m_file, sizeof(WT_Byte),
                                                          num_read, &compression_code));

                if (!compression_code)
                {
                    // This is the compression termination code.
                    // Shut down compression.

                    m_file.set_data_decompression(WD_False);
                    // We may not be out of uncompressed data yet in the main file, and
                    // our parent routine will need to try and keep reading if we haven't
                    // satisfied the current read request.
                    return WT_Result::Decompression_Terminated;
                }

                m_literal_data_run = compression_code & 0x0F;
                m_compression_run  = (compression_code >> 4);

                // Set m_next_byte
                if (!m_literal_data_run)
                {
                    m_next_byte = Process_Compression_Length_Code;
                    break;
                }
                else if (m_literal_data_run == 15)
                {
                    m_next_byte = Extended_Literal_Length_Code;
                    break;
                }
                else
                    m_next_byte = Literal_Data;
            } //// No break;

        case Literal_Data:
            {
                int    amount_to_read;

                if (m_literal_data_run < desired)
                    amount_to_read = m_literal_data_run;
                else
                    amount_to_read = desired;

                // As long as we can read our literal data, copy it into the
                // user's buffer and increment our "actual" count.
                WD_CHECK_W (actual,(m_file.stream_read_action())(m_file, amount_to_read, num_read, buffer));

                // Add the literal data that we have read to the recall buffer
                if ((m_recall_buffer.size() + num_read) > WD_LZ_HISTORY_BUFFER_SIZE)
                {
                    int    amount_to_shift_recall_buffer = (num_read + m_recall_buffer.size()) - WD_LZ_HISTORY_BUFFER_SIZE;
                    m_recall_buffer.remove(amount_to_shift_recall_buffer);
                }

                m_recall_buffer.add(num_read, buffer);

                m_literal_data_run -= num_read;
                desired -= num_read;
                actual += num_read;
                buffer += num_read;


                // Possible cases:
                // (1) we can't read anything
                // (2) we read less than desired, and there is still literal left (the file ran out of data)
                // (3) we read less than desired, and there is no literal left
                // (4) we read all that was desired, and there is literal left
                // (5) we read all that was desired, and there is no literal left.


                if (!m_literal_data_run)
                    m_next_byte = Process_Compression_Length_Code;

                if (!desired)
                    return WT_Result::Success;

                // If the file ran out of data (we are waiting for the network), then
                // just return what we have so far.
                if (num_read < amount_to_read)
                    return WT_Result::Success;
            } break;

        case Extended_Literal_Length_Code:
            {
                WT_Byte    a_byte;

                // Read the Extended length
                WD_CHECK_W (actual,(m_file.stream_read_action())(m_file, sizeof(WT_Byte), num_read, &a_byte));

                m_literal_data_run += a_byte;

                m_next_byte = Literal_Data;
            } break;


        case Process_Compression_Length_Code:
            {
                if (!m_compression_run)
                {
                    m_next_byte = Compression_Code;
                    m_compression_run += WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES;
                    break;
                }
                else if (m_compression_run == 15)
                {
                    m_next_byte = Extended_Compression_Length_Code;
                    m_compression_run += WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES;
                    break;
                }

                m_next_byte = First_Offset_Code_Byte;
                m_compression_run += WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES;
            } ////break;

        case First_Offset_Code_Byte:
            {
                WT_Byte    a_byte;

                WD_CHECK_W (actual,(m_file.stream_read_action())(m_file, sizeof(WT_Byte), num_read, &a_byte));

                // Because Intel is Little-Endian, the first byte we read is actually the low
                // order bits of the offset value.
                m_offset = a_byte;
                m_next_byte = Second_Offset_Code_Byte;
            } ////break;

        case Second_Offset_Code_Byte:
            {
                WT_Byte    a_byte;

                WD_CHECK_W (actual,(m_file.stream_read_action())(m_file, sizeof(WT_Byte), num_read, &a_byte));

                // Because Intel is Little-Endian, the second byte we read is actually the high
                // order bits of the offset value.
                m_offset |= a_byte << 8;
                if (m_file.rendition().drawing_info().decimal_revision() >= REVISION_WHEN_HISTORY_BUFFER_PRELOADED)
                {
                    // Starting with revision 00.23, all offset codes are relative
                    // to the end of the historu buffer.  We need to flip this value
                    // to be useable.
                    m_offset = (m_recall_buffer.size() - 1) - m_offset;
                }

                m_next_byte = Eating_Recall_Buffer_Data;
            } ////break;

        case Eating_Recall_Buffer_Data:
            {
                int    amount_to_eat;

                if (m_compression_run < desired)
                    amount_to_eat = m_compression_run;
                else
                    amount_to_eat = desired;

                WD_Assert((amount_to_eat + m_offset) <= m_recall_buffer.size());

                m_recall_buffer.fetch(amount_to_eat, m_offset, buffer);

                // Add the data to the recall buffer.
                // Check to see if we will overflow the recall buffer.
                if ((m_recall_buffer.size() + amount_to_eat) > WD_LZ_HISTORY_BUFFER_SIZE)
                {
                    int    amount_to_shift_recall_buffer = (amount_to_eat + m_recall_buffer.size()) - WD_LZ_HISTORY_BUFFER_SIZE;

                    m_offset -= amount_to_shift_recall_buffer;
                    m_recall_buffer.remove(amount_to_shift_recall_buffer);
                }

                m_recall_buffer.add(amount_to_eat, buffer);

                m_compression_run -= amount_to_eat;
                desired -= amount_to_eat;
                actual += amount_to_eat;
                buffer += amount_to_eat;

                m_offset += amount_to_eat;

                // Possible cases:
                // (1) we fetched less than desired because there is no compressed string left
                // (2) we fetched all that was desired, and there is compressed string left
                // (3) we fetched all that was desired, and there is no compressed string left

                if (!m_compression_run)
                    m_next_byte = Compression_Code;

                if (!desired)
                    return WT_Result::Success;
            } break;

        case Extended_Compression_Length_Code:
            {
                WT_Byte    a_byte;

                // Read the Extended length
                WD_CHECK_W (actual,(m_file.stream_read_action())(m_file, sizeof(WT_Byte), num_read, &a_byte));

                m_compression_run += a_byte;
                m_next_byte = First_Offset_Code_Byte;
            } break;

        default:
            {
                WD_Assert(WD_False);
            } break;
        } // switch (m_next_byte)
    } // while (1)
}


// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ZLIB DeCompressor
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================


///////////////////////////////////////////////////////////////////////////
WT_Result WT_ZLib_DeCompressor::start()
{
    int err;

    m_zlib_stream.zalloc = (alloc_func)0;
    m_zlib_stream.zfree = (free_func)0;
    m_zlib_stream.opaque = (voidpf)0;

    m_zlib_stream.next_in   = m_compressed_data_buffer;
    m_zlib_stream.avail_in  = 0;
    m_zlib_stream.next_out  = Z_NULL;
    m_zlib_stream.avail_out = 0;

    err = inflateInit(&m_zlib_stream);
    if (err != Z_OK)
    {
        if (err == Z_MEM_ERROR)
            return WT_Result::Out_Of_Memory_Error;
        else
            return WT_Result::Internal_Error;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_ZLib_DeCompressor::decompress(int desired, int & actual, void * users_buffer)
{
    WD_Assert(desired > 0);
    WD_Assert(users_buffer);

    WT_Byte *    buffer = (WT_Byte *) users_buffer;
    int          err;

    m_zlib_stream.next_out  = buffer;
    m_zlib_stream.avail_out = desired;
    // We haven't read any data yet.  So say so...
    actual = 0;


    while (actual < desired)
    {
        // At this point we have some compressed data to work on.
        err = inflate(&m_zlib_stream, Z_SYNC_FLUSH);
        actual = desired - m_zlib_stream.avail_out;

        switch(err)
        {
            case Z_OK:
                // Do nothing if everthing went well.
                break;
            case Z_STREAM_END:
            {
                // This is the compression termination code.
                // Shut down compression.
                m_file.set_data_decompression(WD_False);
                if (inflateEnd(&m_zlib_stream) != Z_OK)
                    return WT_Result::Internal_Error;

                // We wont be out of uncompressed data yet in the main file, and
                // our parent routine will need to try and keep reading if we haven't
                // satisfied the current read request.

                // Note that our "aval_in/next_in" buffer might not be empty since
                // we read from the raw file a chunk at a time.  Since not all of
                // the chunk may have been used, we'll have to stream this leftover
                // data to the higher level routines.
                // To do this, we are getting a bit tricky here: we're going to
                // temporarily set aside the normal raw-read routine and replace
                // it with our own temporary routine that will stream the remaining
                // data.

                // Store the leftover data in the main file object (since it is the
                // only thing that sticks around).
                WD_CHECK(m_file.handle_leftover_decompressor_stream_read_data(m_zlib_stream.avail_in,
                                                                     m_zlib_stream.next_in));

                return WT_Result::Decompression_Terminated;
            }
                break;
            case Z_NEED_DICT:
            {
                err = inflateSetDictionary( &m_zlib_stream,
                                            (const Bytef*) WD_History_Buffer_Preload,
                                            WD_History_Buffer_Preload_Size - WD_PORTION_OF_PRELOAD_DATA_NOT_USED_FOR_ZLIB);
                if (err != Z_OK)
                    return WT_Result::Internal_Error;
            }
            break;
            case Z_DATA_ERROR: // Data corruption
                return WT_Result::Corrupt_File_Error;
            case Z_MEM_ERROR:
                return WT_Result::Out_Of_Memory_Error;
            case Z_BUF_ERROR:
            {
                // Do nothing, hope more data comes later.
                if (m_zlib_stream.avail_in)
                    return WT_Result::Internal_Error; // We couldn't make progress, but there is data available-- something is wrong.



                int num_read = 0;
                WD_CHECK_W (actual,
                            (m_file.stream_read_action())(  m_file,
                                                            sizeof(m_compressed_data_buffer),
                                                            num_read,
                                                            m_compressed_data_buffer));

                m_zlib_stream.next_in  = m_compressed_data_buffer;
                m_zlib_stream.avail_in = num_read;

                // Only return Waiting_for_data if there weren't *any* bytes available.
                if (!m_zlib_stream.avail_in)
                {
                    if (actual)
                        return WT_Result::Success;
                    else
                        return WT_Result::Waiting_For_Data;
                }

            }
                break;
            case Z_STREAM_ERROR:
            case Z_VERSION_ERROR:
            default:
                return WT_Result::Internal_Error;
        }
    } // while (actual < desired)

    // To get to this point, we must have decompressed the desired number of bytes.
    return WT_Result::Success;
}
