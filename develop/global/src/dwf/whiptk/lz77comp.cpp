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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/lz77comp.cpp 3     2/02/05 12:14p Evansg $


#include "whiptk/pch.h"

#include "whiptk/preload.h"

extern const WT_RGBA32 WD_Old_Default_Palette[256];
extern const WT_RGBA32 WD_New_Default_Palette[256];

///////////////////////////////////////////////////////////////////////////
void WT_LZ_Compressor::preload_history_buffer()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    // Preload the history buffer with commonly found strings.
    // Don't include the trailing  null terminator.
    unsigned int loop = 0;

    if (m_file.heuristics().target_version() <= WHIP31_DWF_FILE_VERSION)
        loop += sizeof(WHIP_40_PRELOAD_ADDITIONS) - 1;

    while (loop < (sizeof(WD_History_Buffer_Preload) - 1))
    {
        add_to_history_buffer(WD_History_Buffer_Preload[loop++], WD_False);
    }

#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
// This is just the inline support functions.
// See the "Compress" function below for the main loop.
inline int gen_hash_key(
    WT_Byte first,
    WT_Byte second,
    WT_Byte third,
    WT_Byte fourth)
{
    return (first ^ (second << 3) ^ (third << 5) ^ (fourth << 8));
}

///////////////////////////////////////////////////////////////////////////
inline void WT_LZ_Compressor::extend_best_match_length()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert(m_compression_started);

    // Compute where in the history buffer this string extends from
    int hist_pos = m_history.pointer_to_index(m_best_match) + m_best_match_length;

    // See how long a match we get.
    while (m_best_match_length < m_candidate.size())
    {
        if (hist_pos >= m_history.size() ||
            m_best_match_length >= WD_MAXIMUM_MATCH_LENGTH ||
            (m_candidate.item(m_best_match_length) != m_history.item(hist_pos).value()) )
        {
            // No matter how much input data we have, this string can't be made any
            // longer than what we just got.
            m_best_match_extendible = WD_False;
            return;
        }

        hist_pos++;
        m_best_match_length++;
    } // while

#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
// Here is the main loop...
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_LZ_Compressor::compress(int in_size, void const * in_buf)
{
    WD_Assert(m_compression_started);
    WD_Assert(!in_size || (in_size && in_buf));

    // Add the incomming data to the candidate buffer for processing.
    if (in_size)
    {
        m_candidate.add(in_size, (WT_Byte *)in_buf);
    }

    while(1)
    {
        // Are we continuing from a previous match, or are we trying to
        // start a new match?
        if (!m_best_match)
        {
            // If our candidate isn't big enough to worry about, then
            // wait until it is long enough to process.
            if (m_candidate.size() <= WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES)
                return WT_Result::Success;

            // Generate hash value.
            WD_Assert (m_candidate.size() >= WD_LZ_BYTES_USED_FOR_HASH);

            // TODO: should use a different hashing function if the WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES
            // value is less than 3.
            WT_Integer32 hash = gen_hash_key(    m_candidate.item(0),
                                                m_candidate.item(1),
                                                m_candidate.item(2),
                                                m_candidate.item(3)    );

            WD_Assert(hash < WD_LZ_HASH_TABLE_SIZE);

            // See if we've possibly seen this string before.
            if (!m_hash_table[hash])
            {
                // Never seen this string before, so peel one byte from
                // the candidate buffer and put it into the history buffer, and output
                // the byte to the real file.
                WT_Byte one_byte;
              m_candidate.remove(1, &one_byte);
                WD_CHECK (add_to_history_buffer(one_byte,WD_True));

                continue;
            }  // If (we can't find this string in the hash table)
            else
            {
                // We *might* have found this string in the hash table (at least we found
                // a matching hash key).

                // Keep track of the best match we can get, walking down the hash table list
                // of matching keys...

                // We are just starting to match on this string.  To get here we know
                // that m_best_match is null, and m_best_match_length is one less than the
                // minimum size string we can compress.

                m_best_match_length = WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES;

                m_hash_entries_searched = 0;
                find_better_match(m_hash_table[hash]);

                if (!m_best_match)
                {
                    // This string has never occured in the past, so we can't compress it.
                    WT_Byte one_byte;
                  m_candidate.remove(1, &one_byte);
                    WD_CHECK (add_to_history_buffer(one_byte, WD_True));

                    continue;
                }

                // We have the longest match possible, and it isn't extendible.
            }
        }  // if (trying to start)
        else
        {
            // We have a single extendible match, but we haven't looked at every possible match
            // in the hash table's list yet.
            // With the single extendible match that we have, we'll first try to extend it,
            // and if we can't, set it to non-extendible and continue through the hash table list.

            extend_best_match_length();

            if (!m_best_match_extendible)
                find_better_match(m_best_match->next());

        }  // else (we were just extending)

        if (m_best_match_extendible)
        {
            // OK, we've found at least one matching string, and it may be extended
            // further by more input data.  To get more data, we'll have to return
            // to the caller and get called again later.
            return WT_Result::Success;
        }

        // We have the longest match possible, and it isn't extendible.
        WD_CHECK (output_match());

        // Clean up, and get ready to start searching with a new string...
        m_best_match = WD_Null;

        // There may be more stuff in the candidate buffer yet to be compressed.
    } // While
}
#else
WT_Result WT_LZ_Compressor::compress(int, void const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_LZ_Compressor::output_match()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert(m_compression_started);

    WT_Result    result;
    WT_Byte        tmp_buf[WD_MAX_LITERAL_DATA_STREAM];


    // First, flush the literal data output FIFO.
    // To do this, we need to encode the run length info for the compression code
    // in the upper 4 bits, and the literal data length in the lower 4 bits.
    WT_Byte code;
    int        count;
    int        run_length_fixed;


    // Our compression code is made of two 4-bit halves.  The lower four bits are the
    // length of the literal data run, and the upper 4 bits are the size of the
    // LZ match that we got.

    count    = m_literal_output_buf.size();
    WD_Assert(count < WD_MAX_LITERAL_DATA_STREAM);
    if (count > 14)
        // Our literal data's length wont fit into four bits. 15 is used to indicate
        // that another byte will follow.
        code = 0x0F;
    else
        code = (WT_Byte) count;

    run_length_fixed = m_best_match_length - WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES;
    if (run_length_fixed <= 14)
        // We are encoding the length of the compression string in 4 bits only, for a max size of 17.
        code |= (WT_Byte)(run_length_fixed << 4);
    else
        // We are encoding the length of the compression string in 4 bits plus an additional byte.
        code |= (WT_Byte)0xF0;

    result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
    // If result failed, we've got a big problem.
    if (result != WT_Result::Success)
        return result;

    if (count)
    {
#ifdef DEBUG_ASSERTIONS
        m_total_literal_bytes += count;
        m_total_literal_strings++;
#endif

        if (count > 14)
        {
            // We couldn't specify the length of the literal data in just 4 bits, so
            // we output an additional byte that represents the range 15 to (15 + 255).
            code = (WT_Byte)(count - 15);
            result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
            // TODO: if result failed, we've got a big problem.
            if (result != WT_Result::Success)
                return result;
        }

        m_literal_output_buf.remove(count, tmp_buf);
        result = (m_file.stream_write_action())(m_file, count, tmp_buf);
        // TODO: if we don't get success here, we need to free all the memory we have...
        if (result != WT_Result::Success)
            return result;
    }

    // If we have a real long compression string, so rather than just using 4 bits to encode
    // the length of the compression string, we output an additional byte which lets us
    // go up to a string length of 273 (adjusted).
    if (run_length_fixed > 14)
    {
        code = (WT_Byte)(run_length_fixed - 15);
        result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
        // TODO: if result failed, we've got a big problem.
        if (result != WT_Result::Success)
            return result;
    }

    // First output a two byte offset code
    unsigned short    offset = (unsigned short)((m_history.size() - 1) - m_history.pointer_to_index(m_best_match));
    WD_Assert((offset < m_history.size()) && (offset >= WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES));

    result = (m_file.stream_write_action())(m_file, sizeof(unsigned short), &offset);
    // TODO: if we don't get success here, we need to free all the memory we have...
    if (result != WT_Result::Success)
        return result;

    // Get ready to start the process over with a new string.
    m_best_match = WD_Null;

#ifdef DEBUG_ASSERTIONS
    m_total_compressed_bytes += m_best_match_length;
    m_total_compression_codes++;
    if (m_best_match_length > m_biggest_compressed_string)
        m_biggest_compressed_string = m_best_match_length;
#endif

    // Next, remove the matched string from the candidate buffer and add to the history
    // buffer-- without outputting any characters.
    for (int loop = 0; loop < m_best_match_length; loop++)
    {
        WT_Byte one_byte;
        m_candidate.remove(1, &one_byte);
        result = add_to_history_buffer(one_byte, WD_False);
        // TODO: if we don't get success here, we need to free all the memory we have...
        if (result != WT_Result::Success)
            return result;
    }

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_LZ_Compressor::find_better_match(WT_History_Item * hist_item)
{
    WD_Assert(m_compression_started);

    while (hist_item)
    {
        // We have the input string, and we are given a string in the history buffer that
        // has a matching hash key.  See how long a matching string we can get between these
        // two strings.

        // Compute where in the history buffer this string starts
        int hist_pos = m_history.pointer_to_index(hist_item);

        // See how long a match we get.
        int match_length = 0;
        WT_Boolean extendible = WD_True;
        while (match_length < m_candidate.size())
        {
            if (hist_pos >= m_history.size() ||
                match_length >= WD_MAXIMUM_MATCH_LENGTH ||
                (m_candidate.item(match_length) != m_history.item(hist_pos).value()) )
            {
                // No matter how much input data we have, this string can't be made any
                // longer than what we just got.
                extendible = WD_False;
                break;
            }

            match_length++;
            hist_pos++;
        }

    #ifdef DEBUG_ASSERTIONS
        if (!match_length)
            m_hash_misses++;
        else
            m_hash_hits++;
    #endif

        if (match_length > m_best_match_length ||
            extendible &&
            (match_length >= m_best_match_length) &&
            match_length > WD_LZ_COMPRESSION_AND_OFFSET_CODE_BYTES)
        {
            // We can compress this string, and it is our best bet so far.

            m_best_match = hist_item;
            m_best_match_length = match_length;
            m_best_match_extendible = extendible;
            if (extendible)
                return;
        }

        // See if we want to give up even though we could possibly get
        // even better compression.  We may want to give in order to speed
        // up compression, at the expense of some compression.
        if (++m_hash_entries_searched > WD_MAX_HASH_ENTRIES_TO_SEARCH)
                return;

        hist_item = hist_item->next();
    } // while
    }
#else
void WT_LZ_Compressor::find_better_match(WT_History_Item *)
{}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_LZ_Compressor::start()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert(!m_compression_started);

    for (int loop = 0; loop < WD_LZ_HASH_TABLE_SIZE; loop++)
        m_hash_table[loop] = WD_Null;

    // Output an opcode indicating data compression will follow.
    WT_Compressed_Data_Moniker    moniker;
    WD_CHECK(moniker.serialize(m_file));
    m_hash_entries_searched = 0;
    m_compression_started = WD_True;

#ifdef DEBUG_ASSERTIONS
        m_total_literal_bytes = 0;
        m_total_literal_strings = 0;
        m_num_literal_string_overflows = 0;
        m_total_compressed_bytes = 0;
        m_total_compression_codes = 0;
        m_biggest_compressed_string = 0;
        m_hash_misses = 0;
        m_hash_hits = 0;
#endif

    preload_history_buffer();
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_LZ_Compressor::stop()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WT_Byte        code;
    WT_Result    result;

    WD_Assert(m_compression_started);

    if (m_best_match)
    {
        // We've been trying to extend some matches, but since
        // we've been told to flush, don't try and extend them any
        // more, just output the first one as a compressed object.
        // (All of them will be the same size).
        result = output_match();
        if (result != WT_Result::Success)
            return result;
    }

    // Next, empty the candidate buffer and add to the history
    // buffer-- while outputing the characters.
    while(m_candidate.size())
    {
        WT_Byte one_byte;
        m_candidate.remove(1, &one_byte);
        result = add_to_history_buffer(one_byte, WD_True);
        // TODO: figure out how to clean up this mess if something failed.
        if (result != WT_Result::Success)
            return result;
    }

    // Flush out the output_buf.
    WT_Byte tmp_buf[WD_MAX_LITERAL_DATA_STREAM];
    int        count = m_literal_output_buf.size();

    if (count)
    {
        if (count < 15)
            code = (WT_Byte) count;
        else
            code = 15;

        result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
        // TODO: if result failed, we've got a big problem.
        if (result != WT_Result::Success)
            return result;

        if (count > 14)
        {
            // We couldn't specify the length of the literal data in just 4 bits, so
            // we output an additional byte that represents the range 15 to (15 + 255).
            code = (WT_Byte)(count - 15);
            result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
            // TODO: if result failed, we've got a big problem.
            if (result != WT_Result::Success)
                return result;
        }

        m_literal_output_buf.remove(count, tmp_buf);
        result = (m_file.stream_write_action())(m_file, count, tmp_buf);
        // TODO: if we don't get success here, we need to free all the memory we have...
        if (result != WT_Result::Success)
            return result;
    }


    // Write a compression terminator to the file.
    code = 0x00;
    result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &code);
    // TODO: if we don't get success here, we need to free all the memory we have...
    if (result != WT_Result::Success)
        return result;

    // Write an extended binary opcode terminator.
    WT_Byte    a_byte = '}';
    WD_CHECK ((m_file.stream_write_action())(m_file, 1, &a_byte));

    // TODO: Now seek back in the file and record the length of the compression stream

    // Remove and delete everything in the history buffer.
    m_history.clear_all();
    m_best_match = WD_Null;
    m_compression_started = WD_False;

    return result;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_LZ_Compressor::add_to_history_buffer(WT_Byte a_byte, WT_Boolean output)
{
    WT_History_Item    a_hist_item;

    // Nope, haven't seen this string before, so pull the first byte
    // off the front of the candidate buffer and put it into the history
    // buffer.

    // Is the history buffer full?
    WD_Assert(m_history.size() <= WD_LZ_HISTORY_BUFFER_SIZE);
    if (m_history.size() == WD_LZ_HISTORY_BUFFER_SIZE)
    {
        // The history buffer is full, so adding the new byte on the
        // end will pop off a byte at the beginning.
        m_history.remove(1, &a_hist_item);

        // We must also remove this old item from the hash table.
        a_hist_item.unlink();
    }

    // Add the recent byte on the end of the history buffer.  We can't generate
    // a hash value for this byte until a few other bytes are added after this one at
    // a later time.
    a_hist_item.set_value(a_byte);
    m_history.add(1, &a_hist_item);

    // Create a hash entry for the byte that is WD_LZ_BYTES_USED_FOR_HASH units back,
    // but only if our history buffer is large enough.
    if (m_history.size() >= WD_LZ_BYTES_USED_FOR_HASH)
    {
        // We've got a big enough history buffer such that the addition of
        // this new byte to the history buffer will allow us to compute
        // a hash table entry for the byte that is WD_LZ_BYTES_USED_FOR_HASH bytes
        // earlier in the history buffer.

        int    h_size = m_history.size();

        WT_Integer32 hash = gen_hash_key(    m_history.item(h_size - 4).value(),
                                            m_history.item(h_size - 3).value(),
                                            m_history.item(h_size - 2).value(),
                                            a_byte                );

        WD_Assert(hash < WD_LZ_HASH_TABLE_SIZE);

        WT_History_Item *    cur_item = &m_history.item(h_size -4);

        cur_item->set_next(m_hash_table[hash]);

        // Insert the new item into the hash table.
        if (m_hash_table[hash])
            m_hash_table[hash]->set_backlink(&cur_item->next());
        cur_item->set_backlink(&m_hash_table[hash]);
        m_hash_table[hash] = cur_item;
    }  // If (we have to add a hash entry for an older item)


    // Now output this byte to the file, if told to do so.
    if (output)
    {
        m_literal_output_buf.add(1, &a_byte);

        if (m_literal_output_buf.size() >= WD_MAX_LITERAL_DATA_STREAM)
        {
#ifdef DEBUG_ASSERTIONS
            m_total_literal_bytes += WD_MAX_LITERAL_DATA_STREAM;
            m_total_literal_strings++;
            m_num_literal_string_overflows++;
#endif

            // Our output buffer has overflowed, so we output it to the real file.

            // Since we have overflowed the literal data buffer, we need to
            // output two bytes: a compression code of 0x0F (meaning that there is no compressed
            // data [the upper 4 bits] and that there is more than 14 bytes of litteral data
            // [the lower 4 bits].  Then we output a second byte of 0XFF which is a count of the literal
            // data that follows.  This 255 value actually indicates (15 + 255) length, which is
            // equal to WD_MAX_LITERAL_DATA_STREAM.

            WT_Byte my_byte = 0x0F;
            WT_Result result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &my_byte);
            // TODO: if we don't get success here, we need to free all the memory we have...
            if (result != WT_Result::Success)
                return result;

            my_byte = 0xFF;
            result = (m_file.stream_write_action())(m_file, sizeof(WT_Byte), &my_byte);
            // TODO: if we don't get success here, we need to free all the memory we have...
            if (result != WT_Result::Success)
                return result;

            WT_Byte    tmp_buf[WD_MAX_LITERAL_DATA_STREAM];
            m_literal_output_buf.remove(WD_MAX_LITERAL_DATA_STREAM, tmp_buf);
            result = (m_file.stream_write_action())(m_file, WD_MAX_LITERAL_DATA_STREAM, tmp_buf);
            // TODO: if we don't get success here, we need to free all the memory we have...
            if (result != WT_Result::Success)
                return result;
        }
    }  // If (output)

    return WT_Result::Success;
}
#else
WT_Result WT_LZ_Compressor::add_to_history_buffer(WT_Byte, WT_Boolean)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ZLIB Compressor
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================
// ======================================================================================


///////////////////////////////////////////////////////////////////////////
WT_Result WT_ZLib_Compressor::start()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert(!m_compression_started);

    // Output an opcode indicating data compression will follow.
    WT_Compressed_Data_Moniker    moniker;
    WD_CHECK(moniker.serialize(m_file));
    m_compression_started = WD_True;

    int err;

    m_zlib_stream.zalloc = (alloc_func)0;
    m_zlib_stream.zfree  = (free_func)0;
    m_zlib_stream.opaque = (voidpf)0;

    // err = deflateInit(&m_zlib_stream, Z_DEFAULT_COMPRESSION);

    err = deflateInit2 ( &m_zlib_stream,                // The stream
                         WD_ZLIB_DEFLATE_LEVEL,         // The compression level (0 to 9 with 6 as standard default)
                         Z_DEFLATED,                    // Only Z_DEFLATED is supported in ZLIB 1.1.3
                         WD_ZLIB_DEFLATE_WINDOW_BITS,   // The LZ77 window bits (size of the history buffer) (8 to 15 max) 15 gives 32K history.
                         WD_ZLIB_DEFLATE_MEM_LEVEL,     // Amount of memory to use to speed compression (max is 9, default is 8).
                         Z_DEFAULT_STRATEGY);           // Strategy to use when compressing.


    if (err != Z_OK)
    {
        if (err == Z_MEM_ERROR)
            return WT_Result::Out_Of_Memory_Error;
        else
            return WT_Result::Internal_Error;
    }

    m_zlib_stream.next_out  = m_zlib_output_buffer;
    m_zlib_stream.avail_out = (uInt)sizeof(m_zlib_output_buffer);

    return preload_history_buffer();

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_ZLib_Compressor::preload_history_buffer()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    // Preload the history buffer with commonly found strings.
    // Don't include the trailing  null terminator.
    int err;

    err = deflateSetDictionary( &m_zlib_stream,
                                (const Bytef*)WD_History_Buffer_Preload,
                                sizeof(WD_History_Buffer_Preload) - WD_PORTION_OF_PRELOAD_DATA_NOT_USED_FOR_ZLIB);
    if (err != Z_OK)
    {
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_ZLib_Compressor::compress(int in_size, void const * in_buf)
{
    int err;

    WD_Assert(m_compression_started);
    WD_Assert(!in_size || (in_size && in_buf));
    WD_Assert(m_zlib_stream.next_out >= m_zlib_output_buffer);
    WD_Assert(m_zlib_stream.next_out <  (m_zlib_output_buffer + sizeof(m_zlib_output_buffer)));
    WD_Assert(m_zlib_stream.avail_out > 0);

    m_zlib_stream.next_in  = (Bytef*)in_buf;
    m_zlib_stream.avail_in = (uInt)in_size;

    while (m_zlib_stream.avail_in)
    {
        err = deflate(&m_zlib_stream, Z_NO_FLUSH);
        switch(err)
        {
        case Z_OK:
            // Returned if some progress has been made
            // (more input processed or more output produced)
            // At this point avail_in will normally be zero, and thus we'll exit the main loop.

            // No Break; goes here, we'll be hitting the break in the next section:
        case Z_BUF_ERROR:
            // returned if no progress is possible
            // (for example avail_in or avail_out was zero).

            break;

        case Z_STREAM_END:
            // returned if all input has been consumed and all output has been produced
            // (only when flush is set to Z_FINISH)
            // Normally we shouldn't get this result, since Z_FINISH isn't given until the
            // stop() method.

            // No Break; goes here, we'll be returning an internal error code.
        case Z_STREAM_ERROR:
            // returned if the stream state was inconsistent
            // (for example if next_in or next_out was NULL)

            // No Break; goes here, we'll be returning an internal error code.
        default:
            // Got an illegal error code from ZLIB.
            return WT_Result::Internal_Error;
        } // switch (err)

        if (m_zlib_stream.avail_out == 0)
        {
            // The output buffer has filled.  Dump it to the stream and reset.
            WT_Result   result;

            result = (m_file.stream_write_action())(m_file, sizeof(m_zlib_output_buffer), m_zlib_output_buffer);
            if (result != WT_Result::Success)
                return result;

            m_zlib_stream.next_out  = m_zlib_output_buffer;
            m_zlib_stream.avail_out = (uInt)sizeof(m_zlib_output_buffer);
        }

    } // while (avail_in)

    return WT_Result::Success;
}
#else
WT_Result WT_ZLib_Compressor::compress(int, void const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_ZLib_Compressor::stop()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    int          err;
    WT_Result    result;

    //WD_Assert(m_compression_started);
    WD_Assert(m_zlib_stream.next_out >= m_zlib_output_buffer);
    WD_Assert(m_zlib_stream.next_out <  (m_zlib_output_buffer + sizeof(m_zlib_output_buffer)));
    WD_Assert(m_zlib_stream.avail_out > 0);

    // Flush out any data still in the compressor

    err = Z_OK;
    while (err == Z_OK)
    {
        err = deflate(&m_zlib_stream, Z_FINISH);

        // Write it to disk
        result = (m_file.stream_write_action())(m_file,
                                                sizeof(m_zlib_output_buffer) - m_zlib_stream.avail_out,
                                                m_zlib_output_buffer);
        if (result != WT_Result::Success)
            return result;

        m_zlib_stream.next_out  = m_zlib_output_buffer;
        m_zlib_stream.avail_out = (uInt)sizeof(m_zlib_output_buffer);
    }

    if (err != Z_STREAM_END)
        return WT_Result::Internal_Error;

    // Free up the Zlib memory/resources...
    if (deflateEnd(&m_zlib_stream) != Z_OK)
        return WT_Result::Internal_Error;

    // Write an extended binary opcode terminator.
    WT_Byte    a_byte = '}';
    WD_CHECK ((m_file.stream_write_action())(m_file, 1, &a_byte));

    // TODO: Now seek back in the file and record the length of the compression stream

    m_compression_started = WD_False;
    return result;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}
