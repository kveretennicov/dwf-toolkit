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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/file.cpp 6     8/29/05 5:53p Gopalas $


#include "whiptk/pch.h"
#include <ctype.h>

#if !defined WD_WIN32_SYSTEM
#include "whiptk/convert_utf.h"
#endif

//WT_File  _WT_File_g_none;


#ifdef WD_NEED_DEBUG_PRINTF
    #include <stdio.h>
    #include <stdarg.h>
    FILE *    WD_debug_fp = 0;

    int WD_dprintf (char const * control, ...) {
        auto    va_list         va;
        auto    int             status;
        auto    char            buf[1024];

    if (!WD_debug_fp)
        WD_debug_fp = fopen("c:\\whiptk.txt","wb");

        va_start (va, control);
        status = vsprintf (buf, control, va);
        va_end (va);
        fprintf (WD_debug_fp, "%s", buf);
        fflush(WD_debug_fp);
        return status;
    }
#endif

#define CALLBACK_MACRO(class_name, class_lower)    \
    ,m_##class_lower##_action (WT_##class_name::default_process)

//////////////////////////////////////////////////////////////////////////////////
WT_File::WT_File() throw(WT_Result)
  : m_stream_user_data(WD_Null)
  CALLBACK_LIST
  , m_stored_stream_end_seek_action()
  , m_stored_stream_read_action(WD_Null)
  , m_stored_stream_seek_action(WD_Null)
  , m_stream_close_action(default_close)
  , m_stream_end_seek_action(default_end_seek)
  , m_stream_open_action(default_open)
  , m_stream_read_action(default_read)
  , m_stream_seek_action(default_seek)
  , m_stream_tell_action(default_tell)
  , m_stream_write_action(default_write)
  , m_filename()
  , m_mode (File_Inactive)
  , m_heuristics()
  , m_desired_rendition()
  , m_rendition()
  , m_opcode()
  , m_current_object(WD_Null)
  , m_layer_list()
  , m_object_node_list()
  , m_dpat_list()
  , m_have_read_first_opcode(WD_False)
  , m_current_point(0,0)
  , m_next_incarnation (1)
  , m_actual_file_position(0)
  , m_paren_count(0)
  , m_tab_level(0)
  , m_read_fifo()
  , m_quote (0)
  , m_processing_quoted_string(WD_False)
  , m_processing_escaped_literal(WD_False)
  , m_string_accumulator()
  , m_compressor(WD_Null)
  , m_decompressor(WD_Null)
  , m_data_decompression(0)
  , m_decomp_leftover_data_buffer()
  , m_delayed_drawable(WD_Null)
  , m_currently_dumping_delayed_drawable(WD_False)
  , m_current_file_position(0)
  , m_this_opcode_start_position(0)
  , m_number_of_opcodes_read(0)
  , m_file_stats(NULL)
  , m_read_colors_state(0)
  , m_read_ascii_value(0)
  , m_read_ascii_sign(1)
  , m_read_ascii_double_value()
  , m_read_ascii_double_sign()
  , m_read_ascii_double_divisor()
  , m_read_ascii_logical_point_value()
  , m_temp_index()
  , m_temp_rgba()
  , m_temp_box()
  , m_directory()
  , m_skip_byte_length(0)
  , m_read_count_state(Getting_Count_Byte)
  , m_read_ascii_integer32_state(Eating_Initial_Whitespace)
  , m_read_ascii_double_state(Eating_Initial_Whitespace)
  , m_read_ascii_logical_point_state(Getting_X_Coordinate)
  , m_read_ascii_logical_box_state(Getting_Min_Point)
  , m_read_ascii_color_state(Getting_First_Value)
  , m_read_hex_multiple_state(0)
  , m_read_hex_state(Eating_Whitespace)
{
    // We allocate the rendition so that when the file is closed and a new
    // one is opened we can easily have the constructor called with the
    // default values.

    m_rendition = new WT_Rendition;
    if( !m_rendition )
        throw WT_Result::Out_Of_Memory_Error;
    m_heuristics.set_target_version(toolkit_decimal_revision(), this);

	strcpy(m_write_ascii_format, "%.11g");
}

#undef CALLBACK_MACRO

//////////////////////////////////////////////////////////////////////////////////
WT_File::~WT_File()
{
    close();
    delete m_rendition;
    delete m_decompressor;
    delete m_delayed_drawable;
    delete m_file_stats;
}
//////////////////////////////////////////////////////////////////////////////////
void WT_File::set_filename(char const * name)
{
    m_filename = (unsigned char const*)name;
}

//////////////////////////////////////////////////////////////////////////////////
void WT_File::set_filename(WT_Unsigned_Integer16 const * name)
{
    m_filename.set(WT_String::wcslen(name),name);
}

//////////////////////////////////////////////////////////////////////////////////
void WT_File::set_filename(int length, WT_Unsigned_Integer16 const * name)
{
    m_filename.set(length,name);
}

//////////////////////////////////////////////////////////////////////////////////
WT_String const & WT_File::filename() const
{
    return m_filename;
}

//////////////////////////////////////////////////////////////////////////////////
void WT_File::set_file_mode(WT_File_mode mode)
{
    m_mode = mode;

    if (heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS
        && (m_mode == Block_Append))
        throw WT_Result::Toolkit_Usage_Error;
}

//////////////////////////////////////////////////////////////////////////////////
WT_File::WT_File_mode WT_File::file_mode() const
{
    return m_mode;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::open()
{
    m_number_of_opcodes_read = 0;
    m_currently_dumping_delayed_drawable = WD_False;
    m_delayed_drawable = WD_Null;
    m_current_point = WT_Logical_Point(0,0);
    set_have_read_first_opcode(WD_False);
    m_read_count_state = Getting_Count_Byte;
    m_read_ascii_integer32_state = Eating_Initial_Whitespace;
    m_read_ascii_double_state = Eating_Initial_Whitespace;
    m_read_ascii_logical_point_state = Getting_X_Coordinate;
    m_read_ascii_logical_box_state = Getting_Min_Point;
    m_read_ascii_color_state = Getting_First_Value;
    m_read_hex_multiple_state = 0;
    m_read_hex_state = Eating_Whitespace;

    m_read_colors_state = 0;
    m_read_ascii_value = 0;
    m_read_ascii_sign = 1;

    WD_CHECK ((m_stream_open_action)(*this));

    if ((file_mode() == File_Write) ||
        (file_mode() == Block_Write))
    {
        WT_DWF_Header   file_header;

        WD_CHECK (file_header.serialize(*this));
    }
    else if ((file_mode() == Block_Append) ||
             (file_mode() == Block_Read) )
    {
        // Mode is Block_Append
        if (m_file_stats)
            delete m_file_stats;
        m_file_stats = new WT_File_Stats(this);
        if (!m_file_stats)
            return WT_Result::Out_Of_Memory_Error;

        //In read-write mode the file pointer would always
        //be set to the begining of the Directory opcode information
        //after reading the existing directory opcode information in
        //to the m_directory instance member variable of the WT_File object.
        //This is to enable adding new blocks/opcodes/drawables at the end.

        if(file_mode() == Block_Read) {
            WD_CHECK( initialize_open_for_block_read_mode() );
        }
        else { //Block_Append mode
            WD_CHECK( initialize_open_for_block_append_mode() );
        }
    }
    else
    {
        // Mode is File_Read
        if (m_file_stats)
            delete m_file_stats;
        m_file_stats = new WT_File_Stats(this);
        if (!m_file_stats)
            return WT_Result::Out_Of_Memory_Error;
    }

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::initialize_open_for_block_read_mode()
{
    WD_Assert (stream_user_data());
    WD_Assert (file_mode() == Block_Read);

    // We make sure that we have read dwf header, for simple
    // reason that the file in question need not be a dwf file
    // (in which case we return file open error).
    // Also, later, we try to materialize directory as part
    // of file open initialization (when opened dwf file in
    // read-write mode). As we already know, before we materialize
    // any other opcode, it is a must that we materialize the
    // dwf header opcode(look WT_Opcode::get_opcode member function).

    WD_CHECK(get_next_object());

    WD_Assert(heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED);
    if (heuristics().target_version() < REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED)
        return WT_Result::File_Open_Error;

    int END_BYTE_LENGTH = 0;
    WT_Boolean bBinaryFileType = WD_True;
    WD_CHECK( is_file_type_binary(bBinaryFileType, END_BYTE_LENGTH) );
    if(bBinaryFileType)
        heuristics().set_allow_binary_data(WD_True);
    else
        heuristics().set_allow_binary_data(WD_False);

    WD_CHECK((stream_end_seek_action())(*this));

    int distance_seeked = -END_BYTE_LENGTH;
    int amount_seeked = 0;

    WD_CHECK((stream_seek_action())(*this, distance_seeked, amount_seeked));
    if(amount_seeked != distance_seeked)
        return WT_Result::File_Open_Error;

    WT_Unsigned_Integer32 directory_file_offset;
    if(heuristics().allow_binary_data()) {
        WD_CHECK( read( directory_file_offset) );
    }
    else {
        WD_CHECK( read_ascii( directory_file_offset) );
        //read_ascii always reads a extra character and subsequently
        //we put_back that character, which is held in m_read_fifo
        //for subsequent reads. Since we are going to seek back and
        //materialize directory opcode information, we want to make
        //sure that we ignore this extra character in the m_read_fifo
        //buffer, so that we do actually start getting characters from
        //the actual file stream starting from the directory's begining
        //file offset (to materialize directory).
        WD_CHECK( skip(1) );
    }

    WD_CHECK((stream_end_seek_action())(*this));

    unsigned long current_file_offset = (unsigned long)(-1L);
    WD_CHECK((stream_tell_action())(*this, &current_file_offset));
    if(current_file_offset == (unsigned long)(-1L))
        return WT_Result::File_Open_Error;

    distance_seeked = directory_file_offset - current_file_offset;
    WD_CHECK((stream_seek_action())(*this, distance_seeked, amount_seeked));
    if(amount_seeked != distance_seeked)
        return WT_Result::File_Open_Error;

    WT_Opcode option_code;
    WD_CHECK (option_code.get_opcode(*this));
    WD_CHECK (m_directory.materialize(option_code, *this));
    WD_CHECK( get_to_the_directory() );

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::initialize_open_for_block_append_mode()
{
    WT_Result result;
    WD_Assert (stream_user_data());
    WD_Assert (file_mode() == Block_Append);

    if(file_mode() != Block_Append)
        return WT_Result::Toolkit_Usage_Error;

    do {
        result = process_next_object();
    } while (result == WT_Result::Success);

    WD_Assert(heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED);
    if (heuristics().target_version() < REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED)
        return WT_Result::File_Open_Error;

    WD_CHECK((stream_end_seek_action())(*this));

    WD_CHECK( get_to_the_directory() );

    int END_BYTE_LENGTH = 0;
    WT_Boolean bBinaryFileType = WD_True;
    WD_CHECK( is_file_type_binary(bBinaryFileType, END_BYTE_LENGTH) );
    if(bBinaryFileType)
        heuristics().set_allow_binary_data(WD_True);
    else
        heuristics().set_allow_binary_data(WD_False);

    WD_CHECK( get_to_the_directory() );

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::is_file_type_binary(WT_Boolean & bBinaryFileType, int & end_byte_length)
{

    WD_Assert (stream_user_data());
    WD_Assert ((file_mode() == Block_Append) ||
               (file_mode() == Block_Read));

    bBinaryFileType = WD_True;
    unsigned long begining_file_offset = (unsigned long)(-1L);
    WD_CHECK((stream_tell_action())(*this, &begining_file_offset));
    if(begining_file_offset == (unsigned long)(-1L))
        return WT_Result::File_Open_Error;

    WT_String sAsciiEndString[3] = {")\r\n(EndOfDWF)\r\n", ")\r\r\n(EndOfDWF)\r\r\n", ")(EndOfDWF)\r\n"};
    WT_String sBinaryEndString[3] = {"}\r\n(EndOfDWF)\r\n", "}\r\r\n(EndOfDWF)\r\r\n", "}(EndOfDWF)\r\n"};

    int distance_seeked = -1;
    int amount_seeked = -1L;
    int read_count = -1L;
    WT_Byte buffer[20]; //includes a byte for null space too.

    for(int i=0; i<3; i++) {
        //We now reverse skip EndOfDWF tag to determine the file
        //heuristics (to determine binary or ascii).
        WD_CHECK((stream_end_seek_action())(*this));
        distance_seeked = -1;
        amount_seeked = -1L;
        read_count = -1L;
        distance_seeked *= sAsciiEndString[i].length();
        WD_CHECK((stream_seek_action())(*this, distance_seeked, amount_seeked));
        if(amount_seeked != distance_seeked)
            return WT_Result::File_Open_Error;

        memset((void *) &buffer, '\0', sizeof(WT_Byte) * 20);
        WD_CHECK((stream_read_action())(*this, sAsciiEndString[i].length(), read_count, &buffer));

        if(read_count != sAsciiEndString[i].length())
            return WT_Result::File_Open_Error;

        if(memcmp((void *) buffer, (void *) sAsciiEndString[i].ascii(),
            sAsciiEndString[i].length()) == 0)
        {
            bBinaryFileType = WD_False;
            end_byte_length = (sAsciiEndString[i].length() + 11) * sizeof(WT_Byte);
            goto LOOP_BREAK;
        }
        else if(memcmp((void *) buffer, (void *) sBinaryEndString[i].ascii(),
            sBinaryEndString[i].length()) == 0)
        {
            bBinaryFileType = WD_True;
            end_byte_length = (sizeof(WT_Byte) * sBinaryEndString[i].length()) +
                sizeof(WT_Unsigned_Integer32);
            goto LOOP_BREAK;
        }
    }

LOOP_BREAK:
    unsigned long current_file_offset = (unsigned long)(-1L);
    WD_CHECK((stream_tell_action())(*this, &current_file_offset));
    if(current_file_offset == (unsigned long)(-1L))
        return WT_Result::File_Open_Error;

    distance_seeked = 0;
    amount_seeked   = 0;
    distance_seeked = begining_file_offset - current_file_offset;
    WD_CHECK((stream_seek_action())(*this, distance_seeked, amount_seeked));
    if(amount_seeked != distance_seeked)
        return WT_Result::File_Open_Error;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::get_to_the_directory()
{
    WD_Assert (stream_user_data());
    WD_Assert ((file_mode() == Block_Append) ||
               (file_mode() == Block_Read));

    WD_CHECK((stream_end_seek_action())(*this));
    unsigned long current_file_offset = (unsigned long)(-1L);
    WD_CHECK((stream_tell_action())(*this, &current_file_offset));
    if(current_file_offset == (unsigned long)(-1L))
        return WT_Result::Internal_Error;

    int distance_seeked = 0;
    int amount_seeked = 0;
    distance_seeked = m_directory.get_file_offset() - current_file_offset;
    WD_CHECK((stream_seek_action())(*this, distance_seeked, amount_seeked));
    if(amount_seeked != distance_seeked)
        return WT_Result::Internal_Error;

    WD_CHECK((stream_tell_action())(*this, &current_file_offset));
    if(current_file_offset != m_directory.get_file_offset())
        return WT_Result::Internal_Error;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::close()
{
    WT_Result   result;
    WT_Result   first_error;

    FILE *    fp = (FILE *) stream_user_data();
    if(!fp)
        return WT_Result::Success;

    // Note: this routine should be written such that it can be called at
    // any time.
    // For example, it should be OK to call even after an error has occured,
    // or even if we previously closed the file.
    // This routine may be called from the WT_File destructor.

    if (m_mode == File_Inactive)
        return WT_Result::Success;

    // We don't want to have data compression going on when we are closing out the
    // file.  Remember if it was allowed, and turn it off.
    WT_Boolean    compression_allowed = m_heuristics.allow_data_compression();

    if( (m_mode == Block_Append) ||
        (m_mode == Block_Write)||
        (m_mode == Block_Read) ){
        WD_Assert(heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED);
        if (heuristics().target_version() < REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED)
            return WT_Result::File_Close_Error;
    }

    if ((m_mode == File_Write) || (m_mode == Block_Append) || (m_mode == Block_Write))
    {
        first_error = dump_delayed_drawable();

        if (heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED
        &&  heuristics().target_version() < REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        {
            result = m_directory.serialize((WT_File &) *this);
            if (result != WT_Result::Success && first_error == WT_Result::Success)
                first_error = result;
        }

        m_heuristics.set_allow_data_compression(WD_False);

        if (m_compressor)
        {
            result = m_compressor->is_compression_started() ? m_compressor->stop() : WT_Result::Success;

            if (result != WT_Result::Success && first_error == WT_Result::Success)
                first_error = result;
            delete m_compressor;
            m_compressor = WD_Null;
        }

        WT_End_Of_DWF end_of_dwf;
        result = end_of_dwf.serialize(*this);
        if (result != WT_Result::Success && first_error == WT_Result::Success)
            first_error = result;

        /*The story is that:
        First we start with a null blockref (rendition attribute set by
        default during the file open time). The moment blockref rendition
        attribute changes to some other blockref, we set the blocksize for
        the earlier blockref instance (which would get collected as part of
        the m_directory member variable of the WT_File object). The story
        goes on till we are done setting the block size of the last-but-one
        blockref instance. For the last one blockref, the block size gets
        decided during the file close time when we start serializing the
        directory information. Then we write the directory. Soon after that
        we seek back to the original places of where all the blockrefs were
        originally serialized and re-serialize them with the updated block
        size information. The following code precisely does that with a helper
        function.*/
        result = rewrite_block_sizes_for_all_blockrefs();
        if (result != WT_Result::Success && first_error == WT_Result::Success)
            first_error = result;

    }

    result = (m_stream_close_action)(*this);
    if (result != WT_Result::Success && first_error == WT_Result::Success)
        first_error = result;

    // Reset this file object's settings
    m_mode = File_Inactive;
    m_current_file_position = 0;
    m_this_opcode_start_position = 0;
    m_actual_file_position = 0;
    m_paren_count = 0;
    if (!heuristics().deferred_delete())
    {
        delete m_current_object;
    }
    m_current_object = WD_Null;
    delete m_rendition;
    delete m_file_stats;
    m_file_stats = NULL;
    m_rendition = new WT_Rendition;
    if (m_rendition==NULL && first_error == WT_Result::Success)
        first_error = WT_Result::Out_Of_Memory_Error;

    // Set the "allow compression" state back to what it was at the beginning of this call.
    m_heuristics.set_allow_data_compression(compression_allowed);
    m_data_decompression = 0;
    m_processing_quoted_string = WD_False;
    m_processing_escaped_literal = WD_False;

    m_directory.remove_all();

    if (m_stored_stream_read_action)
    {
        if (m_decomp_leftover_data_buffer.size() > 0)
            m_decomp_leftover_data_buffer.clear_all();

        // We are done with this temporary buffer, return to normal.
        set_stream_read_action(m_stored_stream_read_action);
        m_stored_stream_read_action = WD_Null;
        set_stream_seek_action(m_stored_stream_seek_action);
        m_stored_stream_seek_action = WD_Null;
        set_stream_end_seek_action(m_stored_stream_end_seek_action);
        m_stored_stream_end_seek_action = WD_Null;
    }

	m_layer_list.remove_all();
	m_object_node_list.remove_all();
	m_dpat_list.remove_all();

    return first_error;
}

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE (WHIP_OUTPUT)
WT_Result WT_File::compress_write(int count, void const * data)
{

    if (m_heuristics.allow_data_compression())
    {
        if (!m_compressor)
        {
            if (heuristics().target_version() < REVISION_WHEN_ZLIB_COMPRESSION_IS_SUPPORTED)
                m_compressor = new WT_LZ_Compressor(*this);
            else
                m_compressor = new WT_ZLib_Compressor(*this);

            if (!m_compressor)
                return WT_Result::Out_Of_Memory_Error;

            // Output compression opcode token.
            WD_CHECK (m_compressor->start());
        }
        if (m_compressor->is_compression_started())
            return m_compressor->compress(count, data);
        else
            return m_stream_write_action(*this, count, data);
    }
    else
    {
        if (m_compressor)
        {
            // We've been compressing data, and now are being told to stop.
            // Flush any previously stored data, and fixup the compression run length count.
            WT_Result result = m_compressor->is_compression_started() ? m_compressor->stop() : WT_Result::Success;

            delete m_compressor;
            m_compressor = WD_Null;
            if (result != WT_Result::Success)
                return result;
        }
        return m_stream_write_action(*this, count, data);
    }
}
#else
WT_Result WT_File::compress_write(int, void const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::decompress_read(int count, int & num_read, void * data)
{
    if (m_data_decompression)
    {
        if (!m_decompressor)
        {
            // We have not been decompressing, and are now being asked to do so.
            // So create a new decompression object and start it up...
            if (m_data_decompression == WD_ZLIB_COMPRESSION_EXT_OPCODE)
                m_decompressor = new WT_ZLib_DeCompressor(*this);
            else
                m_decompressor = new WT_LZ_DeCompressor(*this);

            if (!m_decompressor)
                return WT_Result::Out_Of_Memory_Error;

            WD_CHECK(m_decompressor->start());
        }

        // Actually decompress some more data...
        WT_Result result = m_decompressor->decompress(count, num_read, data);

        if (result == WT_Result::Decompression_Terminated)
        {
            // If the read request was fully satisfied, then we return now.
            if (num_read >= count)
                return WT_Result::Success;

            // Try and continue reading since there still may be uncompressed data
            // available that could satisfy the read request and we *must* exhaust
            // all available data before returning Waiting_For_Data. (.. or else bad things
            // happen..)
            result = shut_down_decompression(); // Tries to read the close brace symbol.
            if (result == WT_Result::Waiting_For_Data)
                return (num_read > 0 ? WT_Result::Success : result);
            else if (result != WT_Result::Success)
                return result;  // Some form of error;

            // Wow, we still have data available, so try and do a normal read and
            // merge it with the data that we have read so far from decompression.
            int additional_read = 0;
            result = (stream_read_action()) ( *this,
                                            (count - num_read),
                                            additional_read,
                                            (void *)((WT_Byte *)data + num_read) );
            num_read += additional_read;
            return result;
        }
        else
            return result;
    }
    else
    {
        if (m_decompressor)
        {
            // We've been decompressing data, and now are being told to stop.
            // We must get the close-brace marker that is at the end of the
            // compressed data.
            WD_CHECK (shut_down_decompression());
        }
        return (stream_read_action())(*this, count, num_read, data);
    }
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::shut_down_decompression()
{
    int     read_count;
    WT_Byte close_brace;

    WD_CHECK((stream_read_action())(*this, 1, read_count, &close_brace));
    if (read_count != 1)
        return WT_Result::Waiting_For_Data;

    if (close_brace != '}')
        return WT_Result::Corrupt_File_Error;

    // We succesfully got the close brace, now we can shut down the decompressor.
    delete m_decompressor;
    m_decompressor = WD_Null;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::handle_leftover_decompressor_stream_read_data(int avail_in, WT_Byte const * next_in)
{
    // This routine deals with the issue that when some decompression engines finish,
    // they have read too much data from the raw read stream (and they can't stuff this
    // extra data back into the stream).  Thus, when the decompressor is shutting down,
    // any extra data it read is handed off to this routine.

    // We store the extra data away, and do a sneaky trick of temporarily replacing the
    // normal raw-read routine (supplied by the parent application) with our own routine
    // that will stream out this leftover data and the resotre the original action routine.

    //No left over data left, so let us not use the stored stream routines.
    //Returning here would mean that we will fall back using the regular
    //WT_File supplied stream actions.
    if(avail_in <= 0)
        return WT_Result::Success;

    m_decomp_leftover_data_buffer.add(avail_in, next_in);   // store the leftover data.
    WD_Assert(!m_stored_stream_read_action);
    WD_Assert(!m_stored_stream_seek_action);
    m_stored_stream_read_action = stream_read_action();     // remember the original raw-read action routine
    m_stored_stream_seek_action = stream_seek_action();     // remember the original raw-read action routine
    m_stored_stream_end_seek_action = stream_end_seek_action();     // remember the original raw-read action routine
    set_stream_read_action(temporary_decomp_leftover_read);      // replace with our special temporary routine
    set_stream_seek_action(temporary_decomp_leftover_seek);      // replace with our special temporary routine
    set_stream_end_seek_action(temporary_decomp_leftover_end_seek);      // replace with our special temporary routine

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::decompress_seek(int count, int & num_seeked)
{
    if (m_data_decompression)
    {
        if (!m_decompressor)
        {
            if (m_data_decompression == WD_ZLIB_COMPRESSION_EXT_OPCODE)
                m_decompressor = new WT_ZLib_DeCompressor(*this);
            else
                m_decompressor = new WT_LZ_DeCompressor(*this);

            if (!m_decompressor)
                return WT_Result::Out_Of_Memory_Error;

            WD_CHECK(m_decompressor->start());
        }

        WT_Byte    *    data;

        // TODO: speed this section up by one of two methods:
        //  a) don't keep new'ing/delete'ing this memory
        // or b) add a skip function to the decompressor
        data = new WT_Byte[count];
        if (data == NULL)
            return WT_Result::Out_Of_Memory_Error;
        WT_Result result = m_decompressor->decompress(count, num_seeked, data);
        delete[] data;
        return result;
    }
    else
    {
        if (m_decompressor)
        {
            // We've been decompressing data, and now are being told to stop.
            delete m_decompressor;
            m_decompressor = WD_Null;
        }
        return m_stream_seek_action(*this, count, num_seeked);
    }
}

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE (WHIP_OUTPUT)
WT_Result WT_File::write(char const * str)
{
    return compress_write((int)strlen(str), str);
}
#else
WT_Result WT_File::write(char const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_quoted_string(char const * str, WT_Boolean force_quotes)
{
    char const *    pos;
    WT_Boolean      need_quotes = force_quotes;

    if (!str || !str[0])
    {
        // Null string
        return write("\'\'");
    }

    if (!force_quotes)
    {
        if (*str == '"')
            // " char is the opcode for
            // begining a unicode string.
            // this check is an alternative
            // to escaping the " char.
            need_quotes = WD_True;
        else
            for (pos = str; *pos; pos++)
            {
                if (*pos == ' '  ||
                    *pos == '('  ||
                    *pos == ')'  ||
                    *pos == '{'  ||
                    *pos == '}'  ||
                    *pos == '\\' ||
                    *pos == '\'')
                {
                    need_quotes = WD_True;
                    break;
                }
            }
    }

    if (need_quotes)
    {
        WD_CHECK (write((WT_Byte) '\''));

        for (pos = str; *pos; pos++)
        {
            if (*pos == '\\' || *pos == '\'')
                WD_CHECK (write((WT_Byte) '\\'));

            WD_CHECK (write((WT_Byte) *pos));
        }

        WD_CHECK (write((WT_Byte) '\''));
    }
    else
        WD_CHECK (write (str));

    return WT_Result::Success;
}
#else
WT_Result WT_File::write_quoted_string(char const *, WT_Boolean)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_quoted_string(WT_Unsigned_Integer16 const * string)
{
    if( heuristics().allow_binary_data() )
    {
        WD_CHECK( write((WT_Byte)'{') );
        WT_Integer32 l = WT_String::wcslen(string);
        WD_CHECK( write(l) );
        WD_CHECK( write(l,string) );
        WD_CHECK( write((WT_Byte)'}') );
    }
    else
    {   WD_CHECK( write((WT_Byte)'"') );
        const WT_Unsigned_Integer16* pRead = string;
        if( pRead )
            for(; *pRead; pRead++ )
            {   WT_Unsigned_Integer16 i =
                    WT_Endian::adjust(*pRead);
                WD_CHECK( write_hex((WT_Byte)((i>>8)&0xff)) );
                WD_CHECK( write_hex((WT_Byte)(i&0xff)) );
            }
        WD_CHECK( write((WT_Byte)'"') );
    }
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_quoted_string(WT_Unsigned_Integer16 const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(int count, WT_Byte const * data)
{
    return compress_write(count, data);
}
#else
WT_Result WT_File::write(int, WT_Byte const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(int count, WT_Unsigned_Integer16 const * data)
{

#if WD_BIGENDIAN
    for( int i=0; i<count; i++ )
        WD_CHECK( write(data[i]) );
    return WT_Result::Success;
#else
    return compress_write(sizeof(WT_Unsigned_Integer16)*count, data);
#endif
}
#else
WT_Result WT_File::write(int, WT_Unsigned_Integer16 const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_Integer32 a_word)
{
    WT_Integer32    l_word = WT_Endian::adjust(a_word);

    return compress_write(sizeof(WT_Integer32), &l_word);
}
#else
WT_Result WT_File::write(WT_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_Integer16 a_word)
{
    return write_ascii((WT_Integer32) a_word);
}
#else
WT_Result WT_File::write_ascii(WT_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_Unsigned_Integer16 a_word)
{
    return write_ascii((WT_Integer32) a_word);
}
#else
WT_Result WT_File::write_ascii(WT_Unsigned_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_Unsigned_Integer32 a_dword)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%u", (unsigned int)a_dword);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_ascii(WT_Unsigned_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_Integer32 a_word)
{
    char    ascii_str[40];
    sprintf(ascii_str, "%d", (int)a_word);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_ascii(WT_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(WT_Integer32 a_word)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%011d", (int)a_word);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_padded_ascii(WT_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(WT_Unsigned_Integer32 a_word)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%011u", (unsigned int)a_word);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_padded_ascii(WT_Unsigned_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(double a_double)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%011g", a_double);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_padded_ascii(double)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(WT_Integer16 a_word)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%6d", a_word);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_padded_ascii(WT_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(WT_Unsigned_Integer16 a_word)
{
    char    ascii_str[40];

    sprintf(ascii_str, "%6d", a_word);
    return write(ascii_str);
}
#else
WT_Result WT_File::write_padded_ascii(WT_Unsigned_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////

void WT_File::set_write_ascii_format(char const *strFormat)
{
	strcpy(m_write_ascii_format, strFormat);
}

#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(double val)
{
    char    ascii_str[40];

    sprintf(ascii_str, m_write_ascii_format, val);
    char *  comma_pos;

    comma_pos = strchr(ascii_str, ',');
    if (comma_pos)
        *comma_pos = '.';

    return write(ascii_str);
}
#else
WT_Result WT_File::write_ascii(double)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_Unsigned_Integer32 a_dword)
{
    WT_Unsigned_Integer32    l_dword = WT_Endian::adjust(a_dword);

    return compress_write(sizeof(WT_Unsigned_Integer32), &l_dword);
}
#else
WT_Result WT_File::write(WT_Unsigned_Integer32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_Unsigned_Integer16 a_word)
{
    WT_Unsigned_Integer16    l_word = WT_Endian::adjust(a_word);

    return compress_write(sizeof(WT_Unsigned_Integer16), &l_word);
}
#else
WT_Result WT_File::write(WT_Unsigned_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_Integer16 a_word)
{
    WT_Integer16    l_word = WT_Endian::adjust(a_word);

    return compress_write(sizeof(WT_Integer16), &l_word);
}
#else
WT_Result WT_File::write(WT_Integer16)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_Byte a_byte)
{
    WT_Byte            l_byte = a_byte;
    return compress_write(sizeof(WT_Byte), &l_byte);
}
#else
WT_Result WT_File::write(WT_Byte)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(int count, WT_Logical_Point const * points)
{
#if WD_BIGENDIAN
    WT_Logical_Point const *  pos = points;

    for (int loop = 0; loop < count; loop++)
    {
        WD_CHECK (write(pos->m_x));
        WD_CHECK (write(pos++->m_y));
    }
    return WT_Result::Success;

#else
    return compress_write(sizeof(WT_Logical_Point) * count, points);
#endif
}
#else
WT_Result WT_File::write(int, WT_Logical_Point const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(int count, WT_Logical_Point const * points)
{
    WD_Assert(count > 0);

    for (int loop = 0; loop < count; loop++)
    {
        if (loop)
            WD_CHECK (write((WT_Byte) ' '));    // No space before the very first point.
        WD_CHECK (write_ascii(points[loop].m_x));
        WD_CHECK (write((WT_Byte) ','));
        WD_CHECK (write_ascii(points[loop].m_y));
    }
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_ascii(int, WT_Logical_Point const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(int count, WT_Logical_Point const * points)
{
    WD_Assert(count > 0);

    for (int loop = 0; loop < count; loop++)
    {
        if (loop)
            WD_CHECK (write((WT_Byte) ' '));    // No space before the very first point.
        WD_CHECK (write_padded_ascii(points[loop].m_x));
        WD_CHECK (write((WT_Byte) ','));
        WD_CHECK (write_padded_ascii(points[loop].m_y));
    }
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_padded_ascii(int, WT_Logical_Point const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_Logical_Point const & point)
{
    WT_Logical_Point    tmp(point);

    return write_ascii(1, &tmp);
}
#else
WT_Result WT_File::write_ascii(WT_Logical_Point const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_padded_ascii(WT_Logical_Point const & point)
{
    WT_Logical_Point    tmp(point);

    return write_padded_ascii(1, &tmp);
}
#else
WT_Result WT_File::write_padded_ascii(WT_Logical_Point const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_hex(WT_Byte data)
{
    WT_Byte low_nibble = data &  0x0F;
    WT_Byte hi_nibble  = data >> 4;

    if (low_nibble < 0x0A)
        low_nibble += '0';
    else
        low_nibble += ('A' - 0x0A);

    if (hi_nibble < 0x0A)
        hi_nibble += '0';
    else
        hi_nibble += ('A' - 0x0A);

    WD_CHECK(write(hi_nibble ));
    return   write(low_nibble) ;
}
#else
WT_Result WT_File::write_hex(WT_Byte)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_hex(int count, WT_Byte const * data)
{
    WT_Byte const *   pos     = data;

    for (int loop = 0; loop < count; loop++)
    {
        if (!(loop % 30))
        {
            WD_CHECK (write_tab_level());
            WD_CHECK (write((WT_Byte) '\t'));
        }

        WD_CHECK (write_hex(*pos++));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_File::write_hex(int, WT_Byte const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(int count, WT_Logical_Point_16 const * points)
{
#if WD_BIGENDIAN
    WT_Logical_Point_16 const *  pos = points;

    int loop;
    for (loop = 0; loop < count; loop++)
    {
        WD_CHECK (write(pos->m_x));
        WD_CHECK (write(pos->m_y));
        pos++;
    }
    return WT_Result::Success;

#else
    return compress_write(sizeof(WT_Logical_Point_16) * count, points);
#endif
}
#else
WT_Result WT_File::write(int, WT_Logical_Point_16 const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write(WT_RGBA32 a_color)
{
    // Different systems store the preferred RGB order
    // in the WT_RGBA32 union differently.  We need to adjust this for
    // the file to be rgba order.

    WT_Byte  rgba_color[4];

    rgba_color[0] = a_color.m_rgb.r;
    rgba_color[1] = a_color.m_rgb.g;
    rgba_color[2] = a_color.m_rgb.b;
    rgba_color[3] = a_color.m_rgb.a;

    return compress_write(sizeof(WT_Byte) * 4, rgba_color);
}
#else
WT_Result WT_File::write(WT_RGBA32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_ascii(WT_RGBA32 a_color)
{

    WD_CHECK(write_ascii((WT_Integer32) a_color.m_rgb.r));
    WD_CHECK(write((WT_Byte) ','));
    WD_CHECK(write_ascii((WT_Integer32) a_color.m_rgb.g));
    WD_CHECK(write((WT_Byte) ','));
    WD_CHECK(write_ascii((WT_Integer32) a_color.m_rgb.b));
    WD_CHECK(write((WT_Byte) ','));
    return   write_ascii((WT_Integer32) a_color.m_rgb.a);
}
#else
WT_Result WT_File::write_ascii(WT_RGBA32)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_count (int count)
{
    WD_Assert (count <= WD_MAX_DWF_COUNT_VALUE);
    WD_Assert (count > 0);

    if (count > 255)
    {
        WD_CHECK (this->write((WT_Byte) 0));
        WD_CHECK (this->write((WT_Unsigned_Integer16) (count - 256)));
    }
    else
    {
        WD_CHECK (this->write((WT_Byte) count));
    }
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_count (int)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::write_tab_level()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (heuristics().allow_binary_data())
        return WT_Result::Success;
    else
    {
        WD_Assert(tab_level() >= 0);

        WD_CHECK(write(WD_NEWLINE));

        for (int loop = 0; loop < tab_level(); loop++)
            WD_CHECK(write((WT_Byte) ' '));
    }
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::write_uncompressed_tab_level()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    WD_Assert(tab_level() >= 0);

    WD_CHECK(m_stream_write_action(*this, (int)strlen(WD_NEWLINE), WD_NEWLINE));

    for (int loop = 0; loop < tab_level(); loop++)
        WD_CHECK(write_uncompressed(' '));

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE (WHIP_OUTPUT)
WT_Result WT_File::write_uncompressed(char const * str)
{
    WD_CHECK(m_stream_write_action(*this, (int)strlen(str), str));
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_uncompressed(char const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::write_uncompressed(WT_Byte a_byte)
{
    WT_Byte            l_byte = a_byte;
    WD_CHECK(m_stream_write_action(*this, sizeof(WT_Byte), &l_byte));
    return WT_Result::Success;
}
#else
WT_Result WT_File::write_uncompressed(WT_Byte)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::write_geom_tab_level()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (heuristics().allow_binary_data())
        return WT_Result::Success;
    else
    {
        WD_Assert(tab_level() >= 0);

        WD_CHECK(write(WD_NEWLINE));
        for (int loop = 0; loop < tab_level(); loop++)
            WD_CHECK(write((WT_Byte) ' '));
    }
    return write("   ");

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::process_next_object()
{
    WT_Result   result;

    result = get_next_object();
    if (result != WT_Result::Success)
    {
        WD_Assert(result == WT_Result::Waiting_For_Data || result == WT_Result::User_Requested_Abort);
        return result;
    }

    return object()->process(*this);
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::process_next_object(WT_File *p_uncompressed_file)
{
    WT_Result   result;

    result = get_next_object(p_uncompressed_file);
    if (result != WT_Result::Success)
    {
        WD_Assert(result == WT_Result::Waiting_For_Data);
        return result;
    }

    return object()->process(*this);
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::get_next_object()
{
    // This is a dangerous section since we may partially succeed,
    // reading the object shell, but then run out of data before
    // completing.  The problem is when the app later calls get_next_object()
    // and we don't really want the *next* object, we want to finish
    // with the *current* object.
    // If current_object is Null, then we need to get a new shell.
    // If current_object is non-null (we have at least a shell), and if we have a fully
    //    materialized object, then we must have been called because we were finished with
    //    the previous object and we are being called to get the *next* object.
    // If current_obejct is non-null (we have at least a shell), but we *only* have a shell
    //    (the shell hasn't been materialized) then we must have had a partial success and
    //    have been re-called to finish materializing this object-- in which case we
    //    don't read in a new shell.
    if (m_current_object == WD_Null || heuristics().deferred_delete() || m_current_object->materialized())
    {
        // get_next_object_shell throws away m_current_object if there is one before
        // getting the *next* object shell.
        WD_CHECK (get_next_object_shell());
    }

    return m_current_object->materialize(m_opcode, *this);
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::get_next_object(WT_File *p_uncompressed_file)
{
    // This is a dangerous section since we may partially succeed,
    // reading the object shell, but then run out of data before
    // completing.  The problem is when the app later calls get_next_object()
    // since then we don't really want the *next* object, we want to finish
    // with the *current* object.
    // If current_object is Null, then we need to get a new shell.
    // If current_object is non-null (we have at least a shell), and if we have a fully
    //    materialized object, then we must have been called because we were finished with
    //    the previous object and we are being called to get the *next* object.
    // If current_obejct is non-null (we have at least a shell), but we *only* have a shell
    //    (the shell hasn't been materialized) then we must have had a partial success and
    //    have been re-called to finish materializing this object-- in which case we
    //    don't read in a new shell.
    bool skip_serializing = false;
    if (!have_read_first_opcode())
        skip_serializing = true; //Avoid writing the begining of dwf file header info twice.
                                 //Once it is being written when the file open anyways.

    if (m_current_object == WD_Null || heuristics().deferred_delete() || m_current_object->materialized())
    {
        // get_next_object_shell throws away m_current_object if there is one before
        // getting the *next* object shell.
        WD_CHECK (get_next_object_shell());
    }

    WT_Result result = m_current_object->materialize(m_opcode, *this);

    if (!strcmp((char const *)m_opcode.token(),"(EndOfDWF"))
        skip_serializing = true; //Avoid writing the end of dwf file info twice.
                                 //Once it is being written when the file closes anyways.

    if (!strcmp((char const *)m_opcode.token(),"O")) {
        if(!p_uncompressed_file->heuristics().allow_binary_data())
        skip_serializing = true; //Origin information is not written for ASCII formats.
    }

    if((!skip_serializing) &&
        (m_current_object->object_id() != WT_Object::Compressed_Data_Moniker_ID))
        m_current_object->serialize(*p_uncompressed_file);

    return result;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::get_next_object_shell()
{
    // Destruct the previous object, if any
    if (m_current_object != WD_Null && !heuristics().deferred_delete())
    {
        // Skip the operand of the current object if we haven't already
        // processed it.
        if (!m_current_object->materialized())
            WD_CHECK (m_current_object->skip_operand(m_opcode, *this));

        delete m_current_object;
        m_current_object = WD_Null;
    }

    m_this_opcode_start_position = m_current_file_position;
    WD_CHECK (m_opcode.get_opcode(*this));
    m_number_of_opcodes_read++;

    WT_Result   result;
    m_current_object = m_opcode.object_from_opcode(rendition(), result, *this);
    return result;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read(int size, WT_Byte * read_buffer)
{
    WT_Byte    *    buffer = (WT_Byte *) read_buffer;

    WD_Assert (m_current_file_position >= m_actual_file_position);
    WD_Assert (size > 0);
    WD_Assert (buffer);
    WD_Assert ((file_mode() == File_Read) || (file_mode() == Block_Append) ||
               (file_mode() == Block_Read));
    WD_Assert (stream_user_data());

    // First, see if we were told to skip forward in the file.
    if (m_current_file_position != m_actual_file_position)
    {
        // Yes, we need to skip forward in the file before starting to read.

        // int    skip_amount = m_current_file_position - m_actual_file_position;

        // We first skip forward in the read_buffer, if any, then in the actual file.
        if (m_read_fifo.size() > 0)
        {
            // Throw away the data.
            if ( (m_current_file_position - m_actual_file_position) <= m_read_fifo.size())
            {
                // The entire skip occurs within the read_buffer.
                m_read_fifo.remove(m_current_file_position - m_actual_file_position);
                m_actual_file_position = m_current_file_position;
            }
            else
            {
                // We need to skip by more than what is in the read buffer.
                // We just throw away the entire read buffer.
                m_actual_file_position += m_read_fifo.size();
                m_read_fifo.clear_all();
            }
        }

        WD_Assert (m_current_file_position >= m_actual_file_position);

        // Now start skipping in the real file.
        if (m_current_file_position != m_actual_file_position)
        {
            int    amount_seeked = 0;

            // The following routine may return that it is Waiting_For_Data.
            // We must be able to be recalled and pick up where we left off.
            WT_Result    result = decompress_seek (m_current_file_position - m_actual_file_position,
                                                  amount_seeked);
            m_actual_file_position += amount_seeked;
            if(m_current_file_position == m_actual_file_position)
                return WT_Result::Success;
            if (result != WT_Result::Success)
                return result;
            if (m_current_file_position > m_actual_file_position)
                return WT_Result::Waiting_For_Data;
        }
    }

    WD_Assert(m_current_file_position == m_actual_file_position);

    // OK, now we start reading.

    // First read out of the read_buffer, if it isn't empty, and then
    // read out of the actual file.

    if (size > m_read_fifo.size())
    {
        // We have been requested to read more than is in the read_buffer, so we'll
        // read from the actual file to make up the difference.

        // We want to effectively read out of the read_buffer (if it isn't empty) and
        // then read out of the actual file.  However, reading out of the actual file may
        // cause a Waiting_For_Data condition, in which case we'd have to put back the data
        // in the FIFO.  So instead we'll do the operations in reverse order.
        int num_read;

        WD_CHECK (decompress_read(size - m_read_fifo.size(), num_read, buffer + m_read_fifo.size()));
        // Note that the WI_stream_read_action routine can return Success even if it *didn't*
        // read all of the requested bytes.

        if (!num_read)
            return WT_Result::Waiting_For_Data;

        if (num_read != (size - m_read_fifo.size()))
        {
            // This read operation didn't give us all the requested data.  The input
            // stream must be waiting for data, since it *didn't* give us an End_Of_File
            // error.
            // With the partial data that was read, we put it at the end of the read FIFO
            // buffer for a future read operation to receive.
            m_read_fifo.add(num_read, buffer + m_read_fifo.size());
            return WT_Result::Waiting_For_Data;
        }

        // Great, we got all of the data that we needed from the actual file.
        // Now go back to the beginning of the user's buffer and transfer the entire
        // contents of the read_buffer.
        if (m_read_fifo.size() > 0)
            m_read_fifo.remove(m_read_fifo.size(), buffer);
    }
    else
    {
        // To get here the following is true: (size <= m_read_fifo.size() )
        // meaning that we don't need to read from the actual file, we can just
        // get all the requested data directly from the read_buffer.
        m_read_fifo.remove(size, buffer);
    }

    m_current_file_position += size;
    m_actual_file_position = m_current_file_position;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_Byte & a_byte)
{
    return read(sizeof(WT_Byte), (WT_Byte *) &a_byte);
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_Integer32 & a_word)
{
#if WD_BIGENDIAN
    WT_Integer32    tmp_word;

    WD_CHECK (read(sizeof(WT_Integer32), (WT_Byte *) &tmp_word));
    a_word = WT_Endian::adjust(tmp_word);
    return WT_Result::Success;
#else
    return read(sizeof(WT_Integer32), (WT_Byte *) &a_word);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_Integer16 & a_word)
{
#if WD_BIGENDIAN
    WT_Integer16    tmp_word;

    WD_CHECK (read(sizeof(WT_Integer16), (WT_Byte *) &tmp_word));
    a_word = WT_Endian::adjust(tmp_word);
    return WT_Result::Success;
#else
    return read(sizeof(WT_Integer16), (WT_Byte *) &a_word);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_Unsigned_Integer16 & a_word)
{
#if WD_BIGENDIAN
    WT_Unsigned_Integer16   tmp_word;

    WD_CHECK (read(sizeof(WT_Unsigned_Integer16), (WT_Byte *) &tmp_word));
    a_word = WT_Endian::adjust(tmp_word);
    return WT_Result::Success;
#else
    return read(sizeof(WT_Unsigned_Integer16), (WT_Byte *) &a_word);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_Unsigned_Integer32 & a_dword)
{
#if WD_BIGENDIAN
    WD_CHECK (read(sizeof(WT_Unsigned_Integer32), (WT_Byte *) &a_dword));
    a_dword = WT_Endian::adjust(a_dword);
    return WT_Result::Success;
#else
    return read(sizeof(WT_Unsigned_Integer32), (WT_Byte *) &a_dword);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(WT_RGBA32 & a_color)
{
    WT_Byte        tmp_word[4];

    WD_CHECK (read(sizeof(WT_Byte) * 4, (WT_Byte *) tmp_word));

    // Note that our WT_RGBA32 structure may have the color components
    // in a different order on different operating systems.  However,
    // the order of the components is strictly defined in the DWF file.
    // Therefore we might need to rearrange the color components that
    // we read from the file to match the current operating system.

    // Also, the order of the color components in the DWF file was
    // changed at DWF file version 0.33.  Prior to version 0.33
    // the color components were in bgra order.  Starting with
    // version 0.33 they are in rgba order (as they should have been).

    if (rendition().drawing_info().decimal_revision() < REVISION_WHEN_BINARY_COLOR_CHANGED_TO_RGBA_FROM_BGRA)
    {
        // tmp_word is now in bgra order.
        a_color.m_rgb.b = tmp_word[0];
        a_color.m_rgb.g = tmp_word[1];
        a_color.m_rgb.r = tmp_word[2];
        a_color.m_rgb.a = tmp_word[3];
    }
    else
    {
        // tmp_word is now in rgba order.
        a_color.m_rgb.r = tmp_word[0];
        a_color.m_rgb.g = tmp_word[1];
        a_color.m_rgb.b = tmp_word[2];
        a_color.m_rgb.a = tmp_word[3];
    }

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(int count, WT_RGBA32 * colors)
{
    WT_RGBA32 *    pos = &colors[m_read_colors_state];

    while (m_read_colors_state < count)
    {
        WD_CHECK (read(*pos++));
        m_read_colors_state++;
    }

    // We finished, reset the counter for the next time.
    m_read_colors_state = 0;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(int size, char * buffer)
{
    return read(size, (WT_Byte *) buffer);
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_hex(WT_Byte & byte)
{
    if (m_read_hex_state == Eating_Whitespace)
    {
        WD_CHECK (eat_whitespace());
        m_read_hex_state = Getting_Hex_Value;
    }

    WT_Byte tmp[2];

    WD_CHECK (read(2, tmp));
    m_read_hex_state = Eating_Whitespace;

    if (tmp[0] >= '0' && tmp[0] <= '9')
        byte = tmp[0] - '0';
    else if (tmp[0] >= 'A' && tmp[0] <= 'F')
        byte = tmp[0] + (0x0A - 'A');
    else if (tmp[0] >= 'a' && tmp[0] <= 'f')
        byte = tmp[0] + (0x0A - 'a');
    else
        return WT_Result::Corrupt_File_Error;

    byte <<= 4;

    if (tmp[1] >= '0' && tmp[1] <= '9')
        byte += tmp[1] - '0';
    else if (tmp[1] >= 'A' && tmp[1] <= 'F')
        byte += tmp[1] + (0x0A - 'A');
    else if (tmp[1] >= 'a' && tmp[1] <= 'f')
        byte += tmp[1] + (0x0A - 'a');
    else
        return WT_Result::Corrupt_File_Error;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_hex(int count, WT_Byte * data)
{
    while (m_read_hex_multiple_state < count)
    {
        WD_CHECK (read_hex(data[m_read_hex_multiple_state]));
        m_read_hex_multiple_state++;
    }

    // Finished, so reset the counter for the next time.
    m_read_hex_multiple_state = 0;
    return WT_Result::Success;
}


//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Integer32 & a_word)
{
    switch (m_read_ascii_integer32_state)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_integer32_state = Getting_Sign;
        // No Break here

    case Getting_Sign:
        WT_Byte    a_char;

        WD_CHECK (read(a_char));

        if (a_char == '-')
            m_read_ascii_sign = -1;
        else
        {
            m_read_ascii_sign = 1;
            if (a_char != '+')
                put_back(a_char);
        }

        m_read_ascii_integer32_state = Eating_Post_Sign_Whitespace;
        // No Break here

    case Eating_Post_Sign_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_integer32_state = Getting_First_Digit;
        // No Break here

    case Getting_First_Digit:
        WD_CHECK (read(a_char));

        if (!isdigit(a_char))
            return WT_Result::Corrupt_File_Error;

        m_read_ascii_value = a_char - '0';

        m_read_ascii_integer32_state = Accumulating_Subsequent_Digits;
        //No Break here

    case Accumulating_Subsequent_Digits:
        while(1)
        {
            WD_CHECK (read(a_char));

            if (isdigit(a_char))
                m_read_ascii_value = (m_read_ascii_value * 10) + (a_char - '0');
            else
            {
                put_back(a_char);
                break;
            }
        }
        m_read_ascii_integer32_state = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }

    a_word = m_read_ascii_value * m_read_ascii_sign;
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Unsigned_Integer32 & a_word)
{
    WT_Unsigned_Integer32        read_ascii_value = 0;
    switch (m_read_ascii_integer32_state)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_integer32_state = Getting_First_Digit;
        // No Break here

    case Getting_First_Digit:
        WT_Byte    a_char;
        WD_CHECK (read(a_char));

        if (!isdigit(a_char))
            return WT_Result::Corrupt_File_Error;

        read_ascii_value = a_char - '0';

        m_read_ascii_integer32_state = Accumulating_Subsequent_Digits;
        //No Break here

    case Accumulating_Subsequent_Digits:
        while(1)
        {
            WD_CHECK (read(a_char));

            if (isdigit(a_char))
                read_ascii_value = (read_ascii_value * 10) + (a_char - '0');
            else
            {
                put_back(a_char);
                break;
            }
        }
        m_read_ascii_integer32_state = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }

    a_word = read_ascii_value;

    /*
      // These statements are tempting, but ineffective -
      // HT_Integer32 is an unsigned 32 bit integer,
      // which by definition is already in this range
    WD_Assert(a_word >= 0 && a_word <= 0xFFFFFFFF);
    if (a_word < 0 || a_word > 0xFFFFFFFF)
        return WT_Result::Corrupt_File_Error;
    */

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(double & val)
{
    switch (m_read_ascii_double_state)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_double_state = Getting_Sign;
        // No Break here

    case Getting_Sign:
        WT_Byte    a_char;

        WD_CHECK (read(a_char));

        if (a_char == '-')
            m_read_ascii_double_sign = -1;
        else
        {
            m_read_ascii_double_sign = 1;
            if (a_char != '+')
                put_back(a_char);
        }

        m_read_ascii_double_state = Eating_Post_Sign_Whitespace;
        // No Break here

    case Eating_Post_Sign_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_double_state = Getting_First_Digit;
        // No Break here

    case Getting_First_Digit:
        WD_CHECK (read(a_char));


        if (a_char == '.' || a_char == ',')
        {
            put_back(a_char);
            a_char = '0';
        }

        if (!isdigit(a_char))
            return WT_Result::Corrupt_File_Error;

        m_read_ascii_double_value = a_char - '0';

        m_read_ascii_double_state = Accumulating_Subsequent_Digits;
        //No Break here

    case Accumulating_Subsequent_Digits:
        while(1)
        {
            WD_CHECK (read(a_char));

            if (isdigit(a_char))
                m_read_ascii_double_value = (m_read_ascii_double_value * 10) + (a_char - '0');
            else if (a_char == '.' || a_char == ',')
            {
                m_read_ascii_double_divisor = 0.1;
                m_read_ascii_double_state = Accumulating_Fractional_Digits;
                break; // get out of while loop
            }
            else if (a_char == 'e' || a_char == 'E')
            {
                put_back(a_char);
                m_read_ascii_double_divisor = 0.1;
                m_read_ascii_double_state = Accumulating_Fractional_Digits;
                break; // get out of while loop
            }
            else
            {
                // We have finished, clean up and return.
                put_back(a_char);
                m_read_ascii_double_state = Eating_Initial_Whitespace;
                val = m_read_ascii_double_value * m_read_ascii_double_sign;
                return WT_Result::Success;
            }
        } // while (1)

    case Accumulating_Fractional_Digits:
        while(1)
        {
            WD_CHECK (read(a_char));

            if (isdigit(a_char))
            {
                m_read_ascii_double_value += (a_char - '0') * m_read_ascii_double_divisor;
                m_read_ascii_double_divisor *= 0.1;
            }
            else if (a_char == 'e' || a_char == 'E')
            {
                m_read_ascii_double_state = Accumulating_Exponent;
                break; // get out of while loop
            }
            else
            {
                // We have finished, clean up and return.
                put_back(a_char);
                m_read_ascii_double_state = Eating_Initial_Whitespace;
                val = m_read_ascii_double_value * m_read_ascii_double_sign;
                return WT_Result::Success;
            }
        } // while (1)

    case Accumulating_Exponent:
        WT_Integer32    exponent;

        WD_CHECK (read_ascii(exponent));

        m_read_ascii_double_value *= pow(10.0,(double)exponent);
        // We are done!!
        break;
    default:
        return WT_Result::Internal_Error;
    }

    m_read_ascii_double_state = Eating_Initial_Whitespace;
    val = m_read_ascii_double_value * m_read_ascii_double_sign;
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Unsigned_Integer16 & a_word)
{
    WT_Integer32    tmp;

    WD_CHECK (read_ascii(tmp));

    WD_Assert(tmp >= 0 && tmp <= 0xFFFF);

    if (tmp < 0 || tmp > 0xFFFF)
        return WT_Result::Corrupt_File_Error;

    a_word = (WT_Unsigned_Integer16) tmp;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Integer16 & a_word)
{
    WT_Integer32    tmp;

    WD_CHECK (read_ascii(tmp));

    WD_Assert(tmp >= -0x7FFF && tmp <= 0x7FFF);

    if (tmp < -0x7FFF || tmp > 0x7FFF)
        return WT_Result::Corrupt_File_Error;

    a_word = (WT_Integer16) tmp;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Logical_Point & a_point)
{
    switch (m_read_ascii_logical_point_state)
    {
    case Getting_X_Coordinate:
        WD_CHECK (read_ascii(m_read_ascii_logical_point_value.m_x));
        m_read_ascii_logical_point_state = Getting_Comma;
        // No Break here

    case Getting_Comma:
        WT_Byte    a_char;

        WD_CHECK (read(a_char));
        if (a_char != ',')
        {
            m_read_ascii_logical_point_state = Getting_X_Coordinate;
            return WT_Result::Corrupt_File_Error;
        }
        m_read_ascii_logical_point_state = Getting_Y_Coordinate;
        // No Break here

    case Getting_Y_Coordinate:
        WD_CHECK (read_ascii(m_read_ascii_logical_point_value.m_y));
        break;

    default:
        return WT_Result::Internal_Error;
    }  // Switch (stage)

    // We finished, so reset the state variable for the next time
    m_read_ascii_logical_point_state = Getting_X_Coordinate;
    a_point = m_read_ascii_logical_point_value;
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_Logical_Box & a_box)
{
    switch (m_read_ascii_logical_box_state)
    {
    case Getting_Min_Point:
        WD_CHECK (read_ascii(m_temp_box.m_min));
        m_read_ascii_logical_box_state = Eating_Middle_Whitespace;
        // No Break here

    case Eating_Middle_Whitespace:
        WD_CHECK (eat_whitespace());
        m_read_ascii_logical_box_state = Eating_Middle_Whitespace;
        // No Break here

    case Getting_Max_Point:
        WD_CHECK (read_ascii(m_temp_box.m_max));
        a_box = m_temp_box;
        break;

    default:
        return WT_Result::Internal_Error;
    }  // Switch (stage)

    // We finished, so reset the state variable for the next time
    m_read_ascii_logical_box_state = Getting_Min_Point;
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_ascii(WT_Color & a_color, WT_Boolean allow_indexed_color)
{
    switch (m_read_ascii_color_state)
    {
    case Getting_First_Value:
        WD_CHECK (read_ascii(m_temp_index));
        m_read_ascii_color_state = Getting_Separator;
        // No Break here

    case Getting_Separator:
        WT_Byte a_separator;

        WD_CHECK (read(a_separator));
        if (a_separator == ',')
        {
            // We have a full RGBA color to deal with (not an indexed color)
            // Thus, the first value we read was the red component.
            if (m_temp_index < 0 || m_temp_index > 255)
                return WT_Result::Corrupt_File_Error;
            m_temp_rgba.m_rgb.r = (WT_Byte) m_temp_index;
            m_read_ascii_color_state = Getting_Green;
        }
        else
        {
            if (!allow_indexed_color)
                return WT_Result::Corrupt_File_Error;

            // We have just a color index into the current color map.
            if (m_temp_index < 0 || m_temp_index >= rendition().color_map().size())
                return WT_Result::Corrupt_File_Error;
            a_color = WT_Color(m_temp_index, rendition().color_map());
            put_back(a_separator);
            break;
        }
        // No Break here

    case Getting_Green:
        WD_CHECK (read_ascii(m_temp_index));
        if (m_temp_index < 0 || m_temp_index > 255)
            return WT_Result::Corrupt_File_Error;

        m_temp_rgba.m_rgb.g = (WT_Byte) m_temp_index;
        m_read_ascii_color_state = Getting_Green_Comma;
        // No break here

    case Getting_Green_Comma:
        WD_CHECK (read(a_separator));
        if (a_separator != ',')
            return WT_Result::Corrupt_File_Error;
        m_read_ascii_color_state = Getting_Blue;

        // No break here

    case Getting_Blue:
        WD_CHECK (read_ascii(m_temp_index));
        if (m_temp_index < 0 || m_temp_index > 255)
            return WT_Result::Corrupt_File_Error;

        m_temp_rgba.m_rgb.b = (WT_Byte) m_temp_index;
        m_read_ascii_color_state = Getting_Blue_Comma;
        // No break here

    case Getting_Blue_Comma:
        WD_CHECK (read(a_separator));
        if (a_separator != ',')
            return WT_Result::Corrupt_File_Error;
        m_read_ascii_color_state = Getting_Alpha;

        // No break here

    case Getting_Alpha:
        WD_CHECK (read_ascii(m_temp_index));
        if (m_temp_index < 0 || m_temp_index > 255)
            return WT_Result::Corrupt_File_Error;

        m_temp_rgba.m_rgb.a = (WT_Byte) m_temp_index;
        a_color = WT_Color(m_temp_rgba);
        break;

    default:
        return WT_Result::Internal_Error;
    }  // Switch (stage)

    // We finished, so reset the state variable for the next time
    m_read_ascii_color_state = Getting_First_Value;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(WT_RGBA32 & a_color)
{
    WT_Color    tmp_color;

    WD_CHECK (read_ascii(tmp_color, WD_False));
    a_color = tmp_color.rgba();

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read_ascii(int count, WT_RGBA32 * colors)
{
    WT_RGBA32 *    pos = &colors[m_read_colors_state];

    while (m_read_colors_state < count)
    {
        WD_CHECK (read_ascii(*pos++));
        m_read_colors_state++;
    }

    // We finished, reset the counter for the next time.
    m_read_colors_state = 0;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(int count, WT_Logical_Point    * points)
{
    WD_Assert(points);
    WD_Assert(count > 0);

    WT_Logical_Point * pos = NULL;
    WT_Integer32 arrCoords[8];
    int i = 0;
    while (i <= count - 4)
    {
        WD_CHECK (read(sizeof(arrCoords), (WT_Byte *)arrCoords));

        WT_Integer32 *p = arrCoords;
        pos = &points[i];
        pos[0].m_x = *p++;
        pos[0].m_y = *p++;

        pos[1].m_x = *p++;
        pos[1].m_y = *p++;

        pos[2].m_x = *p++;
        pos[2].m_y = *p++;

        pos[3].m_x = *p++;
        pos[3].m_y = *p;

        i += 4;
    }
    while (i < count)
    {
        WD_CHECK (read(points[i].m_x));
        WD_CHECK (read(points[i].m_y));
        pos++;
        i++;
    }

#if WD_BIGENDIAN
    pos = points;
    for (int loop = 0; loop < count; loop++)
    {
        pos->m_x = WT_Endian::adjust(pos->m_x);
        pos->m_y = WT_Endian::adjust(pos->m_y);
        pos++;
    }
#else
    // NOP
#endif

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result    WT_File::read(int count, WT_Logical_Point_16 * points)
{
    WD_Assert(points);
    WD_Assert(count > 0);

    WT_Logical_Point_16 * pos = NULL;
    WT_Integer16 arrCoords[8];
    int i = 0;
    while (i <= count - 4)
    {
        WD_CHECK (read(sizeof(arrCoords), (WT_Byte *)arrCoords));

        WT_Integer16 *p = arrCoords;
        pos = &points[i];
        pos[0].m_x = *p++;
        pos[0].m_y = *p++;

        pos[1].m_x = *p++;
        pos[1].m_y = *p++;

        pos[2].m_x = *p++;
        pos[2].m_y = *p++;

        pos[3].m_x = *p++;
        pos[3].m_y = *p;

        i += 4;
    }
    while (i < count)
    {
        WD_CHECK (read(points[i].m_x));
        WD_CHECK (read(points[i].m_y));
        pos++;
        i++;
    }

#if WD_BIGENDIAN
    pos = points;
    for (int loop = 0; loop < count; loop++)
    {
        pos->m_x = WT_Endian::adjust(pos->m_x);
        pos->m_y = WT_Endian::adjust(pos->m_y);
        pos++;
    }
#else
    // NOP
#endif

   return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read( int count, WT_Unsigned_Integer16* points )
{
#if WD_BIGENDIAN
    WD_CHECK (read(sizeof(WT_Unsigned_Integer16) * count, (WT_Byte *) points));
    WT_Unsigned_Integer16 *  pos = points;
    for (int loop = 0; loop < count; loop++, pos++)
        *pos = WT_Endian::adjust(*pos);
    return WT_Result::Success;
#else
    return read(sizeof(WT_Unsigned_Integer16) * count, (WT_Byte *) points);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_count (WT_Integer32 & count)
{
    // Find out how many vertices there will be.
    switch (m_read_count_state)
    {
    case Getting_Count_Byte:

        WT_Byte byte_count;

        WD_CHECK (read(byte_count));

        if (byte_count != 0)
        {
            count = byte_count;
            return WT_Result::Success;
        }

        m_read_count_state = Getting_Extended_Count_Short;

        // No break here!
    case Getting_Extended_Count_Short:

        WT_Unsigned_Integer16    tmp_short;

        WD_CHECK (read(tmp_short));

        count = 256 + tmp_short;

        m_read_count_state = Getting_Count_Byte;  // Reset the state for the next time we need to read a count
        break;

    default:
        WD_Assert(WD_False);
        return WT_Result::Internal_Error;
    }  // Switch

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_index_tree(/*WT_Index_List index_list,*/int/*max_size*/,WT_Boolean/* allow_initial_open_paren*/)
{
/*
    If this is ever uncommented the following static variable needs to become
    unstatic.  The problem will be that there is already a data member m_quote
    that is used in the method WT_File::read.
    static WT_Byte quote = 0;

    // Get rid of any preceeding whitespace before
    // accumulating the string.  How do we know if we are "preceeding"
    // the string?  Well, if we haven't accumulated any characters
    // is a good indication, but be carefull in the case where we
    // get a starting quote mark, but then run out of data.  For this
    // we look at m_processing_quoted_string.
    if (!m_string_accumulator.size() && !m_processing_quoted_string)
        WD_CHECK (eat_whitespace());

    while (1)
    {
        WT_Byte    a_byte;

        WD_CHECK (read(a_byte));

        m_string_accumulator.add(1, &a_byte);

        if (m_string_accumulator.size() > max_size)
            return WT_Result::Corrupt_File_Error;

        if (m_processing_escaped_literal)
            // If the previous character was a "\" then this character
            // it to be treated literally and not interpreted.
            m_processing_escaped_literal = WD_False;
        else
        {
            switch (a_byte)
            {
            case '"': //unicode
            case '\'':
                {


                    if (a_byte==quote)
                        if (m_processing_quoted_string)
                        {   // This is the end-quote
                            // that terminates the string.

                            int string_length =
                                m_string_accumulator.size();
                            if (!(string = new char[string_length]))
                                return WT_Result::Out_Of_Memory_Error;
                            m_string_accumulator.remove(
                                string_length,
                                (WT_Byte *)string);

                            // The end-quote mark isn't
                            // actually part of the string,
                            // so remove it and terminate
                            // the string too.
                            string[string_length-1] = '\0';
                            m_processing_quoted_string = WD_False;
                            quote = 0;

                            return WT_Result::Success;
                        }


                    if (m_string_accumulator.size()==1)
                    {
                        if (!quote)
                            quote = a_byte;
                        if (a_byte==quote)
                        {   // We are starting a quoted string,
                            // and we don't want the quote itself
                            // to be part of the stored string,
                            // so remove it.
                            m_string_accumulator.clear_all();
                            m_processing_quoted_string = WD_True;
                        }
                    }


                    if (!quote && a_byte=='\'' && m_string_accumulator.size()>1)
                        // We were accumulating an
                        // unquoted string, when we just
                        // got a quote.  This should have
                        // been escaped, or the entire
                        // string should have been quoted.
                        return WT_Result::Corrupt_File_Error;


                } break;
            case ' ':
            case ')':
                {
                    if (!m_processing_quoted_string)
                    {
                        // The string wasn't quoted, and thus it is
                        // terminated by the close paren.

                        int string_length = m_string_accumulator.size();

                        string = new char[string_length];
                        if (!string)
                            return WT_Result::Out_Of_Memory_Error;

                        m_string_accumulator.remove(string_length, (WT_Byte *)string);
                        // The close-paren isn't actually part of the
                        // string, so remove it and terminate the string too.
                        string[string_length - 1] = '\0';

                        // Let the caller handle the close paren or space so that
                        // the paren counters can be updated.
                        put_back(a_byte);
                        return WT_Result::Success;
                    }
                } break;
            case '(':
                {
                    // There shouldn't be nesting here when we are trying
                    // to accumulate a string.  These characters should
                    // only be found when they are literals contained in a
                    // *quoted* string.  (Unless allow_initial_open_paren is true).
                    if (!m_processing_quoted_string &&
                        (!allow_initial_open_paren || m_string_accumulator.size() != 1))
                        return WT_Result::Corrupt_File_Error;
                } break;
            case '{':
                {
                    // There shouldn't be nesting here when we are trying
                    // to accumulate a string.  These characters should
                    // only be found when they are literals contained in a
                    // *quoted* string.
                    if (!m_processing_quoted_string)
                        return WT_Result::Corrupt_File_Error;
                } break;
            case '\\':
                {
                    // TODO: take care of escaped characters such as
                    // quotes and parens.

                    // Note that to get a literal "\" character, the user
                    // would have entered "\\".  We would not arrive here
                    // for the second "\" because it would have been
                    // handled above already.  Thus we only get here when
                    // we are starting a "literal" sequence.

                    // remove the added escape character
                    m_string_accumulator.pop();

                    m_processing_escaped_literal = WD_True;
                } break;
            default:
                {
                    // Do nothing, since the character was already added
                    // to the FIFO accumulation buffer.
                } break;
            } // switch (a_byte)
        } // else
    } // while (1)
    */
    return WT_Result::Success;

}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read(char * & string, int max_size, WT_Boolean allow_initial_open_paren)
{
    // Get rid of any preceeding whitespace before
    // accumulating the string.  How do we know if we are "preceeding"
    // the string?  Well, if we haven't accumulated any characters
    // is a good indication, but be carefull in the case where we
    // get a starting quote mark, but then run out of data.  For this
    // we look at m_processing_quoted_string.
    if (!m_string_accumulator.size() && !m_processing_quoted_string)
        WD_CHECK (eat_whitespace());

    while (1)
    {
        WT_Byte    a_byte;

        WD_CHECK (read(a_byte));

        m_string_accumulator.add(1, &a_byte);

        if (m_string_accumulator.size() > max_size)
            return WT_Result::Corrupt_File_Error;

        if (m_processing_escaped_literal)
            // If the previous character was a "\" then this character
            // it to be treated literally and not interpreted.
            m_processing_escaped_literal = WD_False;
        else
        {
            switch (a_byte)
            {
            case '"'/*unicode*/:
            case '\'':
                {
                    if (a_byte==m_quote)
                        if (m_processing_quoted_string)
                        {   // This is the end-quote
                            // that terminates the string.

                            int string_length =
                                m_string_accumulator.size();
                            string = new char[string_length];
                            if (!string)
                                return WT_Result::Out_Of_Memory_Error;
                            m_string_accumulator.remove(
                                string_length,
                                (WT_Byte *)string);

                            // The end-quote mark isn't
                            // actually part of the string,
                            // so remove it and terminate
                            // the string too.
                            string[string_length-1] = '\0';
                            m_processing_quoted_string = WD_False;
                            m_quote = 0;

                            return WT_Result::Success;
                        }


                    if (m_string_accumulator.size()==1)
                    {
                        if (!m_quote)
                            m_quote = a_byte;
                        if (a_byte==m_quote)
                        {   // We are starting a quoted string,
                            // and we don't want the quote itself
                            // to be part of the stored string,
                            // so remove it.
                            m_string_accumulator.clear_all();
                            m_processing_quoted_string = WD_True;
                        }
                    }


                    if (!m_quote && a_byte=='\'' && m_string_accumulator.size()>1)
                        // We were accumulating an
                        // unquoted string, when we just
                        // got a quote.  This should have
                        // been escaped, or the entire
                        // string should have been quoted.
                        return WT_Result::Corrupt_File_Error;


                } break;
            case ' ':
            case ')':
            case (0x09):  // white space (Tab)
            case (0x0A):  // white space (Line Feed)
            case (0x0D):  // white space (Carriage Return)
                {
                    if (!m_processing_quoted_string)
                    {
                        // The string wasn't quoted, and thus it is
                        // terminated by the close paren.

                        int string_length = m_string_accumulator.size();

                        string = new char[string_length];
                        if (!string)
                            return WT_Result::Out_Of_Memory_Error;

                        m_string_accumulator.remove(string_length, (WT_Byte *)string);
                        // The close-paren isn't actually part of the
                        // string, so remove it and terminate the string too.
                        string[string_length - 1] = '\0';

                        // Let the caller handle the close paren or space so that
                        // the paren counters can be updated.
                        put_back(a_byte);
                        return WT_Result::Success;
                    }
                } break;
            case '(':
                {
                    // There shouldn't be nesting here when we are trying
                    // to accumulate a string.  These characters should
                    // only be found when they are literals contained in a
                    // *quoted* string.  (Unless allow_initial_open_paren is true).
                    if (!m_processing_quoted_string &&
                        (!allow_initial_open_paren || m_string_accumulator.size() != 1))
                        return WT_Result::Corrupt_File_Error;
                } break;
            case '{':
                {
                    // There shouldn't be nesting here when we are trying
                    // to accumulate a string.  These characters should
                    // only be found when they are literals contained in a
                    // *quoted* string.
                    if (!m_processing_quoted_string)
                        return WT_Result::Corrupt_File_Error;
                } break;
            case '\\':
                {
                    // TODO: take care of escaped characters such as
                    // quotes and parens.

                    // Note that to get a literal "\" character, the user
                    // would have entered "\\".  We would not arrive here
                    // for the second "\" because it would have been
                    // handled above already.  Thus we only get here when
                    // we are starting a "literal" sequence.

                    // remove the added escape character
                    m_string_accumulator.pop();

                    m_processing_escaped_literal = WD_True;
                } break;
            default:
                {
                    // Do nothing, since the character was already added
                    // to the FIFO accumulation buffer.
                } break;
            } // switch (a_byte)
        } // else
    } // while (1)
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::put_back(int size, WT_Byte const * buffer)
{
    // It would be stupid to call skip, and then put back data without reading
    // new data first.  This would totaly confuse the logic, so do an Assert here.
    WD_Assert(m_current_file_position == m_actual_file_position);

    WD_CHECK (m_read_fifo.add(size, buffer));

    m_current_file_position -= size;
    m_actual_file_position = m_current_file_position;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::skip(int size)
{
    m_current_file_position += size;

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::skip_past_matching_paren(int terminating_paren_level)
{
    m_skip_byte_length = 0;
    while (1)
    {
        WT_Byte    a_byte;

        WD_CHECK (read(a_byte));

        //We keep accumulating unknown extended ascii opcode's length
        //till we are done skipping the opcode information entirely.
        //For now, this member variable is used only for passing through
        //unknown extended ascii opcode information (as part of
        //opague-pass-thru functionality), although it gets computed
        //literally for all opcodes.
        m_skip_byte_length++;

        if (m_processing_escaped_literal)
            // If the previous character was a "\" then this character
            // it to be treated literally and not interpreted.
            m_processing_escaped_literal = WD_False;
        else
        {
            switch (a_byte)
            {
            case '\'':
                {
                    m_processing_quoted_string = !m_processing_quoted_string;
                } break;
            case '(':
                {
                    if (!m_processing_quoted_string)
                        increment_paren_count();
                } break;
            case ')':
                {
                    if (!m_processing_quoted_string)
                    {
                        if (decrement_paren_count() <= terminating_paren_level)
                            return WT_Result::Success;
                    }
                } break;
            case '{':
                {
                    if (!m_processing_quoted_string)
                    {
                        WT_Integer32    count;

                        WT_Result    result = read(count);
                        if (result != WT_Result::Success)
                        {
                            // Put the "{" back so that we return to
                            // this spot again when we get more data.
                            put_back(a_byte);
                            return result;
                        }

                        // A count of zero means that the run of this extended
                        // binary data is unknown (the writing application didn't provide
                        // us with the info.  Since we don't know how to skip it, we die here...
                        if (!count)
                            return WT_Result::Corrupt_File_Error;

                        WD_CHECK (skip(count));
                    }
                } break;
            case '\\':
                {
                    m_processing_escaped_literal = WD_True;
                } break;
            default:
                {
                    // Do nothing here since we are skipping...
                } break;
            } // switch (a_byte)
        } // else
    } // while (1)
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::eat_whitespace()
{
    WT_Byte        a_byte;

    while (1)
    {
        WD_CHECK (read(a_byte));

        if (!WU_is_whitespace(a_byte))
        {
            put_back(a_byte);
            return WT_Result::Success;
        }
    } // while (1)
}

//////////////////////////////////////////////////////////////////////////////////
WT_Logical_Point WT_File::update_current_point(WT_Logical_Point const & new_point)
{
    WT_Logical_Point    delta;

    delta.m_x = new_point.m_x - m_current_point.m_x;
    delta.m_y = new_point.m_y - m_current_point.m_y;

    m_current_point = new_point;

    return delta;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Logical_Point WT_File::de_update_current_point(WT_Logical_Point const & delta)
{
    WT_Logical_Point    new_point;

    new_point.m_x = m_current_point.m_x + delta.m_x;
    new_point.m_y = m_current_point.m_y + delta.m_y;

    m_current_point = new_point;

    return new_point;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_open(WT_File & file)
{
    FILE *    fp = (FILE *) file.stream_user_data();

    // Make sure a file isn't already open.
    if (fp)
        return WT_Result::File_Already_Open_Error;

    WT_Result result = WT_Result::Success;
    WT_String mode;
    switch (file.file_mode())
    {
    case WT_File::File_Read:
    case WT_File::Block_Read:
        {
            mode = "rb";
        } break;
    case WT_File::Block_Append:
        {
            mode = "r+b";
        } break;
    case WT_File::File_Write:
    case WT_File::Block_Write:
        {
            if (file.heuristics().allow_binary_data())
            {
                mode = "wb";
            }
            else
            {
                mode = "w";
            }
        } break;
    default:
        {
            result = WT_Result::File_Open_Error;
            return result;
        }
    }  // switch


    if (file.filename().is_ascii())
    {
        //
        // if ASCII just use fopen
        //
        fp = fopen(file.filename().ascii(), mode.ascii());
    }
    else
    {
#if defined WD_WIN32_SYSTEM
        //
        // get the unicode version and call _wfopen which is Win32 specific
        //
        fp = _wfopen((wchar_t*)file.filename().unicode(), (wchar_t*)mode.unicode());
#else
        //
        // on other platforms, convert the string to UTF8 and then use fopen
        //

        size_t utf8length = file.filename().length() * 6 + 1; //max length of string in UTF8
        char* str = new char[utf8length];
        UTF8* target = (UTF8*)str;
        const UTF16* source = file.filename().unicode();
        ConversionResult cresult = ConvertUTF16toUTF8(&source,
            source + file.filename().length() + 1,
            &target, target + utf8length, lenientConversion);
        if (cresult != conversionOK)
        {
            delete [] str;
            return WT_Result::Toolkit_Usage_Error;
        }
        fp = fopen(str, mode.ascii());
        delete [] str;
#endif
    }

    if (!fp || fp == (FILE *)-1)
        return WT_Result::File_Open_Error;
    file.set_stream_user_data(fp);

    return result;
}


//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_close(WT_File & file)
{
    WT_Result result = WT_Result::Success;
    FILE *    fp = (FILE *) file.stream_user_data();

    // Make sure a file is already open.
    if (!fp)
        return WT_Result::No_File_Open_Error;

    // We may be asked to close a file that we can't write to.
    // For example, if we open a file, and later find it to be read only,
    // we should still be able to close the file.
    fclose (fp);
    file.set_stream_user_data(WD_Null);
    return result;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_read(WT_File & file,
                                int desired_bytes,
                                int & bytes_read,
                                void * buffer)
{
    FILE *    fp = (FILE *) file.stream_user_data();

    WD_Assert(desired_bytes > 0);
    WD_Assert(fp);
    WD_Assert((file.file_mode() == WT_File::File_Read) ||
              (file.file_mode() == WT_File::Block_Append)||
              (file.file_mode() == WT_File::Block_Read));
    WD_Assert(buffer);

    bytes_read = 0;

#if defined WD_STRESS_TEST
    // To help debugging of possible network effects, we randomly have this
    // read file "run out of data".

    if (feof(fp))
        return WT_Result::End_Of_File_Error;

    int phony_read = desired_bytes & rand();
    if (!phony_read)
        return WT_Result::Waiting_For_Data;
    bytes_read = fread(buffer, sizeof(WT_Byte), phony_read, fp);

    if (!bytes_read)
        return WT_Result::Waiting_For_Data;
#else
    if (feof(fp))
        return WT_Result::End_Of_File_Error;

    bytes_read = (int)fread(buffer, sizeof(WT_Byte), desired_bytes, fp);

    if (!bytes_read)
        return WT_Result::Unknown_File_Read_Error;
#endif

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::temporary_decomp_leftover_read(WT_File & file,
                                int desired_bytes,
                                int & bytes_read,
                                void * buffer)
{
    // The purpose of this routine is to temporarilly override
    // the normal raw read action supplied by the parent application
    // when we have just finished with compressed data in the file
    // but where the decompressor had accidentally read too far.
    // The decompressor that just finished will have handed us its
    // leftover buffer of uncompressed data for us to keep streaming
    // out.  Once the buffer is exhausted, we'll reinstate the parent
    // application's raw read action routine.

    WD_Assert(desired_bytes > 0);
    WD_Assert( (file.file_mode() == WT_File::File_Read) ||
               (file.file_mode() == WT_File::Block_Append)||
               (file.file_mode() == WT_File::Block_Read));
    WD_Assert(buffer);
    WD_Assert(file.m_decomp_leftover_data_buffer.size() > 0);

    if (desired_bytes > file.m_decomp_leftover_data_buffer.size())
        bytes_read = file.m_decomp_leftover_data_buffer.size();
    else
        bytes_read = desired_bytes;

    file.m_decomp_leftover_data_buffer.remove(bytes_read, (WT_Byte *) buffer);

    if (file.m_decomp_leftover_data_buffer.size() <= 0)
    {
        // We are done with this temporary buffer, return to normal.
        file.set_stream_read_action(file.m_stored_stream_read_action);      // replace with our special temporary routine
        file.m_stored_stream_read_action = WD_Null;
        file.set_stream_seek_action(file.m_stored_stream_seek_action);      // replace with our special temporary routine
        file.m_stored_stream_seek_action = WD_Null;
        file.set_stream_end_seek_action(file.m_stored_stream_end_seek_action);      // replace with our special temporary routine
        file.m_stored_stream_end_seek_action = WD_Null;

        // Now it is very important never to return WAITING_FOR_DATA if we aren't actually waiting.
        // We've just used up the buffer of data that was leftover when the decompression engine shut
        // down, but that doesn't mean that we've run out of data from the actual file.
        // If we didn't get all the data that we wanted, then we should try to read from the
        // file...
        if (bytes_read < desired_bytes)
        {
            int         addl_bytes_read = 0;
            WT_Result   reslt;
            WT_Byte *   place;

            place = (WT_Byte *)buffer + bytes_read;

            reslt = (file.stream_read_action())(file,
                                                (desired_bytes - bytes_read),
                                                addl_bytes_read,
                                                (void *)place);

            bytes_read += addl_bytes_read;

            //if the stream is waiting for more data then return success
            //we want to process what we have in the buffer already
                if (WT_Result::Waiting_For_Data==reslt)
                    reslt = WT_Result::Success;

            return reslt;
        }
    }

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::temporary_decomp_leftover_seek(WT_File & file,
                                int desired_bytes,
                                int & bytes_seeked)
{
    // The purpose of this routine is to temporarilly override
    // the normal raw seek action supplied by the parent application
    // when we have just finished with compressed data in the file
    // but where the decompressor had accidentally read too far.
    // The decompressor that just finished will have handed us its
    // leftover buffer of uncompressed data for us to keep streaming
    // out.  Once the buffer is exhausted, we'll reinstate the parent
    // application's raw seek action routine.

    WD_Assert(desired_bytes > 0);
    WD_Assert( (file.file_mode() == WT_File::File_Read) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Read));
    WD_Assert(file.m_decomp_leftover_data_buffer.size() > 0);

    if (desired_bytes > file.m_decomp_leftover_data_buffer.size())
        bytes_seeked = file.m_decomp_leftover_data_buffer.size();
    else
        bytes_seeked = desired_bytes;


    file.m_decomp_leftover_data_buffer.remove(bytes_seeked);

    if (file.m_decomp_leftover_data_buffer.size() <= 0)
    {
        // We are done with this temporary buffer, return to normal.
        file.set_stream_read_action(file.m_stored_stream_read_action);      // replace with our special temporary routine
        file.m_stored_stream_read_action = WD_Null;
        file.set_stream_seek_action(file.m_stored_stream_seek_action);      // replace with our special temporary routine
        file.m_stored_stream_seek_action = WD_Null;
        file.set_stream_end_seek_action(file.m_stored_stream_end_seek_action);      // replace with our special temporary routine
        file.m_stored_stream_end_seek_action = WD_Null;

        // Now it is very important never to return WAITING_FOR_DATA if we aren't actually waiting.
        // We've just used up the buffer of data that was leftover when the decompression engine shut
        // down, but that doesn't mean that we've run out of data from the actual file.
        // If we didn't get all the data that we wanted, then we should try to read from the
        // file...
        if (bytes_seeked < desired_bytes)
        {
            int         addl_bytes_read = 0;
            WT_Result   reslt;

            reslt = (file.stream_seek_action())(file,
                                                (desired_bytes - bytes_seeked),
                                                addl_bytes_read);

            bytes_seeked += addl_bytes_read;

            //if the stream is waiting for more data then return success
            //we want to process what we have in the buffer already
                if (WT_Result::Waiting_For_Data==reslt)
                    reslt = WT_Result::Success;

            return reslt;
        }
    }

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::temporary_decomp_leftover_end_seek(WT_File & file)
{
    // The purpose of this routine is to temporarilly override
    // the normal raw end seek action supplied by the parent application
    // when we have just finished with compressed data in the file
    // but where the decompressor had accidentally read too far.
    // The decompressor that just finished will have handed us its
    // leftover buffer of uncompressed data for us to keep streaming
    // out.  Once the buffer is exhausted, we'll reinstate the parent
    // application's raw end seek action routine.

    WT_Result reslt = WT_Result::Success;

    WD_Assert( (file.file_mode() == WT_File::File_Read) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Read));
    WD_Assert(file.m_decomp_leftover_data_buffer.size() >= 0);

    file.m_decomp_leftover_data_buffer.remove(file.m_decomp_leftover_data_buffer.size());

    // We are done with this temporary buffer, return to normal.
    file.set_stream_read_action(file.m_stored_stream_read_action);      // replace with our special temporary routine
    file.m_stored_stream_read_action = WD_Null;
    file.set_stream_seek_action(file.m_stored_stream_seek_action);      // replace with our special temporary routine
    file.m_stored_stream_seek_action = WD_Null;
    file.set_stream_end_seek_action(file.m_stored_stream_end_seek_action);      // replace with our special temporary routine
    file.m_stored_stream_end_seek_action = WD_Null;

    reslt = (file.stream_end_seek_action())(file);

    //if the stream is waiting for more data then return success
    //we want to process what we have in the buffer already
    if (WT_Result::Waiting_For_Data==reslt)
        reslt = WT_Result::Success;

        return reslt;
}


//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::default_write(WT_File & file, int size, void const * buffer)
{
    int bytes_writen;

    // Check that we have a file open for writing.
    WD_Assert(file.stream_user_data());
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));
    WD_Assert(size > 0);
    WD_Assert(buffer);

    if (!file.stream_user_data())
        return WT_Result::File_Write_Error;

    bytes_writen = (int)fwrite(buffer, 1, size, (FILE *)file.stream_user_data());
    if (bytes_writen != size)
        return WT_Result::File_Write_Error;
    else
        return WT_Result::Success;
}
#else
WT_Result WT_File::default_write(WT_File &, int, void const *)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_seek(WT_File & file, int distance, int & amount_seeked)
{
    WD_Assert(file.stream_user_data());

    if (fseek((FILE *)file.stream_user_data(), distance, SEEK_CUR) == 0)
    {
        amount_seeked = distance;
        return WT_Result::Success;
    }
    else
    {
        amount_seeked = 0;
        return WT_Result::End_Of_File_Error;
    }
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_end_seek(WT_File & file)
{
    WD_Assert(file.stream_user_data());

    if (fseek((FILE *)file.stream_user_data(), 0L, SEEK_END) == 0)
    {
        return WT_Result::Success;
    }
    else
    {
        return WT_Result::End_Of_File_Error;
    }
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::default_tell(WT_File & file, unsigned long *current_file_pointer_position)
{
    WD_Assert(file.stream_user_data());

    (*current_file_pointer_position) = ftell((FILE *)file.stream_user_data());
    if ((*current_file_pointer_position) != (unsigned long)(-1L))
    {
        return WT_Result::Success;
    }
    else
    {
        return WT_Result::End_Of_File_Error;
    }
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::dump_delayed_drawable()
{
#if DESIRED_CODE(WHIP_OUTPUT)

    if (delayed_drawable() && !m_currently_dumping_delayed_drawable)
    {
        m_currently_dumping_delayed_drawable = WD_True; // Prevents recursion
        WD_CHECK (delayed_drawable()->dump(*this));
        m_currently_dumping_delayed_drawable = WD_False;
        delete m_delayed_drawable;
        m_delayed_drawable = WD_Null;
    }

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

//////////////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_File::merge_or_delay_drawable(WT_Drawable const & current)
{
    if (!heuristics().allow_drawable_merging())
        return current.dump(*this);

    if (delayed_drawable())
    {
        if (delayed_drawable()->object_id() != current.object_id())
        {
            // The two drawables aren't the same type and can't be merged
            // so dump the delayed drawable before proceeding to delay
            // the new drawable.
            WD_CHECK (dump_delayed_drawable());
        }
        else
        {
            // Attempt to merge the two drawables.
            // If they are merged, they can be delayed further.
            // If not, then we'll dump the old drawable and delay the new one.
            if (delayed_drawable()->merge(current))
                return WT_Result::Success;
            else
                WD_CHECK (dump_delayed_drawable());
        }
    }

    // Delay this new drawable by storing a copy of it.
    return current.delay(*this);
}
#else
WT_Result WT_File::merge_or_delay_drawable(WT_Drawable const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


/*First we start with a null blockref (rendition attribute set by
default during the file open time). The moment blockref rendition
attribute changes to some other blockref, we set the blocksize for
the earlier blockref instance (which would get collected as part of
the m_directory member variable of the WT_File object) just before
serializing the newly set blockref rendition attribute using the
helper function set_block_size_for_tail_blockref.*/

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::set_block_size_for_tail_blockref(WT_Unsigned_Integer32 current_blockref_file_offset)
{
    WT_Integer32    item_count = m_directory.count();
    WT_BlockRef *tail_blockref;
    if(item_count) {
        tail_blockref = (WT_BlockRef *) m_directory.get_tail();
        if(tail_blockref == WD_Null)
            return WT_Result::Internal_Error;
        tail_blockref->set_block_size(current_blockref_file_offset - tail_blockref->get_file_offset());
        return WT_Result::Success;
    }
    return WT_Result::Success;
}

    /*Since the block size for a already serialized block is not decided
    up until the next blockref gets serialized (as mentioned above in the
    comments for set_block_size_for_tail_blockref function), we wait till
    the file close time (when the block size for all blocks is pretty much
    decided) and seek back to the blockref serialized instances and
    re-serialize them with the updated block size information. */

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::rewrite_blockref_instance(WT_BlockRef *blockref)
{
    WT_Integer32    item_count = m_directory.count();
    if(item_count >0) {
        WT_Unsigned_Integer32 current_file_offset = 0;
        WD_CHECK((stream_tell_action())(*this, (unsigned long *)&current_file_offset));
        int seek_distance = blockref->get_file_offset() - current_file_offset;
        int seek_amount_seeked = 0;
        WD_CHECK((stream_seek_action())(*this, seek_distance
            ,seek_amount_seeked));
        if(seek_amount_seeked != seek_distance)
            return WT_Result::Internal_Error;
        WD_CHECK(blockref->serialize(*this, WD_False, WD_True));
        WD_CHECK((stream_end_seek_action())(*this));
    }
    return WT_Result::Success;
}

    /*rewrite_blockref_instance function updates an (one) already serialized
    blockref instance (which is not part of the directory instance).
    rewrite_block_sizes_for_all_blockrefs loops through and does the rewriting
    business for all of the previously serialized blockref instances.*/

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::rewrite_block_sizes_for_all_blockrefs()
{
    WT_Integer32    item_count = m_directory.count();
    if(item_count > 0) {
        WT_BlockRef *   currentitem     = (WT_BlockRef *) m_directory.get_head();
        while (currentitem)
        {
            WD_CHECK(rewrite_blockref_instance(currentitem));
            currentitem = (WT_BlockRef *) currentitem->next();
        }
    }
    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::write_block(WT_File & file)
{
    WT_Result result;
    const WT_Object *object;
    WT_File block_file;

    block_file.set_filename(file.filename());
    WD_Assert(stream_user_data());
    WD_Assert( (file_mode() == WT_File::File_Write) ||
               (file_mode() == WT_File::Block_Append) ||
               (file_mode() == WT_File::Block_Write));

    if(! ((file_mode() == WT_File::File_Write) ||
          (file_mode() == WT_File::Block_Append) ||
          (file_mode() == WT_File::Block_Write)) )
        return WT_Result::Toolkit_Usage_Error;

    WD_Assert(heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED);
    if (heuristics().target_version() < REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED)
        return WT_Result::Internal_Error;

    block_file.set_file_mode(WT_File::File_Read);
    WD_CHECK(block_file.open());

    do {
        result = block_file.process_next_object();
        object = block_file.current_object();
        if((object->object_id() != WT_Object::Directory_ID)  &&
           (object->object_id() != WT_Object::DWF_Header_ID) &&
           (object->object_id() != WT_Object::End_Of_DWF_ID) &&
           (object->object_id() != WT_Object::Compressed_Data_Moniker_ID) ) {
            WD_CHECK(object->serialize(*this));
            rendition() = block_file.rendition();
            desired_rendition() = block_file.rendition();
        }
        if(object->object_id() == WT_Object::End_Of_DWF_ID)
            break;
    } while (result == WT_Result::Success);

    WD_CHECK(block_file.close());

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////
WT_Result WT_File::read_block(WT_File & block_file, WT_BlockRef & blockref)
{
    WD_Assert(stream_user_data());
    WD_Assert( (file_mode() == WT_File::Block_Read) ||
               (file_mode() == WT_File::Block_Append) );

    if(! ((file_mode() == WT_File::Block_Read) ||
         (file_mode() == WT_File::Block_Append)) )
        return WT_Result::Toolkit_Usage_Error;

    WD_Assert(heuristics().target_version() >= REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED);
    if (heuristics().target_version() < REVISION_WHEN_READ_WRITE_MODE_WAS_SUPPORTED)
        return WT_Result::Internal_Error;

    if (m_directory.is_empty())
        return WT_Result::Success;

    WT_BlockRef *current = WD_Null;
    current = (WT_BlockRef *) m_directory.get_head();

    while (current != WD_Null)
    {
        if(blockref == *current) {
            WT_Unsigned_Integer32 stored_file_offset = 0;
            WT_Unsigned_Integer32 shifted_file_offset = 0;

            WD_CHECK((stream_tell_action())(*this, (unsigned long *)&stored_file_offset));
            int seek_distance = current->get_file_offset() - stored_file_offset;
            int seek_amount_seeked = 0;
            WD_CHECK((stream_seek_action())(*this, seek_distance
                ,seek_amount_seeked));
            if(seek_amount_seeked != seek_distance)
                return WT_Result::Internal_Error;

            //now dump the block contents to the block file.
            block_file.set_file_mode(WT_File::File_Write);
            WD_CHECK(block_file.open());
            WT_Byte *dump = WD_Null;
            dump = new WT_Byte[current->get_block_size()];
            if(!dump)
                return WT_Result::Out_Of_Memory_Error;
            read(current->get_block_size(), dump);
            (block_file.stream_write_action())(block_file, current->get_block_size(), dump);
            delete[] dump;
            WD_CHECK(block_file.close());

            WD_CHECK((stream_tell_action())(*this, (unsigned long *)&shifted_file_offset));
            seek_distance = stored_file_offset - shifted_file_offset;
            seek_amount_seeked = 0;
            WD_CHECK((stream_seek_action())(*this, seek_distance
                ,seek_amount_seeked));
            if(seek_amount_seeked != seek_distance)
                return WT_Result::Internal_Error;
        }
        current = (WT_BlockRef *) current->next();
    }

    return WT_Result::Success;
}
