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


#include "whiptk/pch.h"


///////////////////////////////////////////////////////////////////////////
WT_User_Fill_Pattern::WT_User_Fill_Pattern(
            WT_Integer16   pattern_num, 
            WT_Unsigned_Integer16   rows,
            WT_Unsigned_Integer16   columns,
            WT_Unsigned_Integer32   data_size,
            const WT_Byte *         data)
            : m_pattern_num(pattern_num)
            , m_is_scale_used(WD_False)
            , m_fill_pattern(WD_Null)
{
    WD_Assert (rows && columns && data_size && data);
    // copy the bitmap data 
    m_fill_pattern =  WT_User_Fill_Pattern::Fill_Pattern::Construct(
                            rows,columns,data_size,data);
    m_fill_pattern->increment();
}   

///////////////////////////////////////////////////////////////////////////
WT_User_Fill_Pattern::WT_User_Fill_Pattern( 
            WT_Integer16    pattern_num,
            Fill_Pattern * fill_pattern)
            : m_pattern_num(pattern_num)
            , m_is_scale_used(WD_False)
            , m_fill_pattern(WD_Null)
{
    if (fill_pattern != WD_Null)
    {
        // increase the ref count 
        fill_pattern->increment();
        m_fill_pattern = fill_pattern;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_User_Fill_Pattern::WT_User_Fill_Pattern(WT_User_Fill_Pattern const & pattern)
{
    this->m_pattern_num = pattern.pattern_number();
    this->m_is_scale_used = pattern.m_is_scale_used;
    this->m_pattern_scale = pattern.pattern_scale();
    Fill_Pattern * fill_ref = pattern.m_fill_pattern;
    
    if (fill_ref != WD_Null)
        fill_ref->increment();
    
    this->m_fill_pattern = fill_ref;
}

///////////////////////////////////////////////////////////////////////////
WT_User_Fill_Pattern::~WT_User_Fill_Pattern()
{
    if (m_fill_pattern != WD_Null)
    {
        m_fill_pattern->decrement();
        if (m_fill_pattern->count() == 0)
        {
            m_fill_pattern->destroy();
            m_fill_pattern = WD_Null;
        }
    }
}
///////////////////////////////////////////////////////////////////////////
WT_User_Fill_Pattern const & WT_User_Fill_Pattern::operator =
                                        (WT_User_Fill_Pattern const & pattern)
{
    this->m_pattern_num = pattern.pattern_number();
    this->m_is_scale_used = pattern.m_is_scale_used;
    this->m_pattern_scale = pattern.pattern_scale();
    Fill_Pattern * fill_ref = pattern.m_fill_pattern;
    
    if (fill_ref != WD_Null)
        fill_ref->increment();
    
    if (m_fill_pattern != WD_Null)  
    {
        m_fill_pattern->decrement();
        if (m_fill_pattern->count() == 0)
        {
            m_fill_pattern->destroy();
            m_fill_pattern = WD_Null;
        }
    }
    m_fill_pattern = fill_ref;
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID   WT_User_Fill_Pattern::object_id() const
{
   return User_Fill_Pattern_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_User_Fill_Pattern::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Success;
    WT_Byte close,open;
    WT_Boolean end = WD_True;
    WT_Unsigned_Integer16   rows;
    WT_Unsigned_Integer16   columns;
    WT_Unsigned_Integer32  data_size;
    WT_Byte * data = WD_Null;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        {       
            WD_CHECK (file.read(m_pattern_num));                    // read fill pat num
            WD_CHECK (file.read(close));   
            end = (close == '}');                                   // read possible close
            if (!end)
            {
                WD_CHECK (file.put_back(close));
                WT_Unsigned_Integer32 val;
                WD_CHECK (file.read(columns));                      // read columns
                WD_CHECK (file.read(rows));                         // read rows
                WD_CHECK (file.read(val));                          // read FixedL 
                m_pattern_scale = (double)val * 0.0000152587890625; // convert FixedL to double
                WD_CHECK (file.read(data_size));                    // read data size
                data = new WT_Byte[data_size];
                WD_CHECK (file.read(data_size,data));               // read bitonal bitmap. 
                WD_CHECK (file.read(close));                        // read close again
                if (close != '}')                                                
                {
                    result = WT_Result::Corrupt_File_Error;                           
                    break;
                }
                m_fill_pattern = WT_User_Fill_Pattern::Fill_Pattern::Construct(
                                    rows,columns,data_size,data);
                m_fill_pattern->increment();
            }
        }break;
    case WT_Opcode::Extended_ASCII:
        {   
            WD_CHECK (file.read_ascii(m_pattern_num));              // read fill pat num
            WD_CHECK (file.read(close));   
            end = (close == ')');                                   // read possible close
            if (!end)
            {   
                WD_CHECK (file.put_back(close));
                WD_CHECK (file.eat_whitespace());                   // whitespace
                WD_CHECK (file.read_ascii(columns));                // read columns
                WD_CHECK (file.read(open));                         // read ',' 
                if (open != ',')                                    // enforce format 
                {
                    result = WT_Result::Corrupt_File_Error;      
                    break;
                }
                WD_CHECK (file.read_ascii(rows));                   // read rows
                WD_CHECK (file.eat_whitespace());
                
                WT_Byte arr[2];
                WD_CHECK (file.read(2,arr));
                bool try_pattern_scale = arr[0] == '(' && arr[1] == 'F';
                // put_back(2,arr) has a bug! Seems to reverse the order, not FIFO but LIFO (???)
                WD_CHECK (file.put_back(arr[1]));
                WD_CHECK (file.put_back(arr[0]));
                if (try_pattern_scale)
                {
                    WD_CHECK (m_optioncode.get_optioncode(file));       // read potential Pattern scale.
                    if  (m_optioncode.type() != WT_Opcode::Null_Optioncode && 
                        m_optioncode.option_id() == WT_User_Fill_Pattern_Option_Code::Pattern_Scale_Option)
                    {
                        WD_CHECK (this->m_pattern_scale.materialize(*this,m_optioncode,file));
                        this->m_is_scale_used = WD_True;
                    }
                    else 
                    {
                        result = WT_Result::Corrupt_File_Error;  
                        break;
                    }
                }
                
                WD_CHECK (file.eat_whitespace());
                WD_CHECK (file.read(open));                         // read compulsory '('  
                if (open != '(')
                {
                    result = WT_Result::Corrupt_File_Error;         
                    break;
                }

                // Should 
                WD_CHECK (file.read_ascii(data_size));              // read data size
                data  = new WT_Byte[data_size];                     // construct data 
                WD_CHECK (file.eat_whitespace());                   // Whitespace
                WD_CHECK (file.read_hex(data_size,data));           // read bitonal bitmap. 
                WD_CHECK (file.read(close));                        // read close ')' for Data 
                if (close != ')')                                        
                {
                    result = WT_Result::Corrupt_File_Error;  
                    break;
                }
                WD_CHECK (file.eat_whitespace());                   // whitespace ! not a part of the spec 
                WD_CHECK (file.read(close));                        // read close ')' for opcode
                if (close != ')')                                       
                {
                    result = WT_Result::Corrupt_File_Error;   
                    break;
                }
                // All is good 
                m_fill_pattern = WT_User_Fill_Pattern::Fill_Pattern::Construct(
                                    rows,columns,data_size,data);
                m_fill_pattern->increment();
            }
        }break;
    default:
        result = WT_Result::Opcode_Not_Valid_For_This_Object;
    }

    // ensure that allocated memory is properly cleaned up 
    if (data != WD_Null)
        delete[] data;
    
    this->m_materialized = (result == WT_Result::Success);
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Fill_Pattern::process(WT_File & file)
{
    WD_Assert( file.user_fill_pattern_action() );
    return (file.user_fill_pattern_action())(*this,file);
}


///////////////////////////////////////////////////////////////////////////
WT_Result   WT_User_Fill_Pattern::skip_operand(WT_Opcode const & , WT_File & )
{
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_User_Fill_Pattern::serialize (WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

   // Serialize out in Extended Binary 
   if (file.heuristics().allow_binary_data()) 
   {
        WD_CHECK (file.write((WT_Byte) '{'));
        // Calculate the size of the opcode.
        WT_Integer32 size = sizeof(WT_Unsigned_Integer16) + // for the opcode
                            sizeof(WT_Unsigned_Integer16) ; // Fill Pattern number
        if ( m_fill_pattern != WD_Null)
        {
            size += (sizeof(WT_Unsigned_Integer16) +        // Num columns
                     sizeof(WT_Unsigned_Integer16) +        // Num rows
                     sizeof(WT_Unsigned_Integer32) +        // Fill Pattern Scale 
                     sizeof(WT_Integer32) +                 // the data size
                     m_fill_pattern->data_size()) ;    
        }
        
        size += sizeof(WT_Byte);                            // The closing "}"
        WD_CHECK (file.write(size));                        // Size of the opcode
        WD_CHECK (file.write((WT_Unsigned_Integer16)
                        WD_EXBO_SET_USER_FILL_PATTERN));     // EXBO opcode
        WD_CHECK (file.write(m_pattern_num));               // pattern number
        if ( m_fill_pattern != WD_Null) {
            WD_CHECK (file.write(m_fill_pattern->columns()));   // columns
            WD_CHECK (file.write(m_fill_pattern->rows()));      // rows
            WT_Unsigned_Integer32 val = (WT_Unsigned_Integer32) ( (double)m_pattern_scale * 65536); 
            WD_CHECK (file.write(val));                         // Fill Pattern scale
            val = m_fill_pattern->data_size();
            WD_CHECK (file.write(val));                         // Data size 
            WD_CHECK (file.write(val, m_fill_pattern->data())); // Bitonal Bitmap Data
        }
        WD_CHECK (file.write((WT_Byte)'}'));                // Closing '}' 

   }
   else  // Extended Ascii output 
   {
        // Determine required parts of this Attribute object.
        WT_User_Fill_Pattern & style    = file.rendition().user_fill_pattern();
        WT_Boolean   is_scale_defined  = WD_False;

        if (m_is_scale_used && m_pattern_scale != style.pattern_scale())
                is_scale_defined = WD_True;

        // Serialize this pattern's attributes if this pattern has never been serialized,
        // or if one this pattern's attributes is different from its previous instance in
        // the file.
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(UserFillPattern "));
        WD_CHECK (file.write_ascii(m_pattern_num));                 // Write out the index 

        // write out the optional Bitonal bitmap info
        if (m_fill_pattern != WD_Null)
        {
            WD_CHECK (file.write((WT_Byte) ' '));                   // Space 
            WD_CHECK (file.write_ascii(m_fill_pattern->columns())); // Columns 
            WD_CHECK (file.write((WT_Byte) ','));                   // ,
            WD_CHECK (file.write_ascii(m_fill_pattern->rows()));    // Rows
            if(is_scale_defined)
            {
                WD_CHECK (file.write((WT_Byte)' '));                // Space 
                WD_CHECK (m_pattern_scale.serialize(*this,file));   // Pattern Scale
            }
            WD_CHECK (file.write(" ("));                            // (
            WT_Unsigned_Integer32 data_size = m_fill_pattern->data_size();
            WD_CHECK (file.write_ascii(data_size));                 // data size 
            WD_CHECK (file.write((WT_Byte) ' '));                   // Space 

            WD_CHECK (file.write_hex(data_size, 
                                m_fill_pattern->data()));           // write out Hex Data 
            WD_CHECK (file.write((WT_Byte)')'));                    // ) 

        }
        WD_CHECK(file.write((WT_Byte)')'));                        // ) closing for opcode 
   }
   return WT_Result::Success;
}
#else
WT_Result   WT_User_Fill_Pattern::serialize (WT_File &) const
{
     return WT_Result::Success;
}
#endif  // DESIRED_CODE(WHIP_OUTPUT)   

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_User_Fill_Pattern::sync (WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if (*this != file.rendition().user_fill_pattern())
    {   
        file.rendition().user_fill_pattern() = *this;
        WD_CHECK (serialize(file));
    }
    return WT_Result::Success;
}
#else
WT_Result   WT_User_Fill_Pattern::sync (WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_CODE(WHIP_OUTPUT)   

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_User_Fill_Pattern::operator== (WT_Attribute const & attrib) const
{
    WT_Boolean result = WD_False;
    if ( attrib.object_id() == User_Fill_Pattern_ID )
    {
        WT_User_Fill_Pattern const & fillpattern = ((WT_User_Fill_Pattern&)attrib);
        result = this->operator==(fillpattern);
        
    }
    return result; 
}
///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_User_Fill_Pattern::operator== (WT_User_Fill_Pattern const & pattern) const
{
    WT_Boolean result = WD_False;
    // Compare pattern numbers 
    if (pattern.pattern_number() == this->m_pattern_num)
    {
        // Compare Fill patterns 
        if (m_fill_pattern != WD_Null && pattern.fill_pattern() != WD_Null)
        {
            result =  m_fill_pattern->operator ==(*(pattern.fill_pattern()));
        }
        else if (m_fill_pattern == WD_Null && pattern.fill_pattern() == WD_Null)
        {
            result = WD_True;
        }
        // compare pattern scale 
        result = (result && (this->m_pattern_scale == pattern.pattern_scale()));
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Fill_Pattern::default_process(WT_User_Fill_Pattern & item, WT_File & file)
{
    file.rendition().user_fill_pattern() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
int WT_User_Fill_Pattern::WT_User_Fill_Pattern_Option_Code::option_id_from_optioncode()
{
    if( !strcmp((char const *)token(), "(FillPatternScale") )
        return WT_User_Fill_Pattern::WT_User_Fill_Pattern_Option_Code::Pattern_Scale_Option;
    return 0;
}


///////////////////////////////////////////////////////////////////////////
// Internal class Constructor 
WT_User_Fill_Pattern::Fill_Pattern::Fill_Pattern( WT_Unsigned_Integer16    rows,
                                                 WT_Unsigned_Integer16    columns, 
                                                 WT_Unsigned_Integer32    data_size,
                                                 WT_Byte const *          data)
                                                 throw()
: DWFCountedObject()
, m_rows(rows)
, m_columns(columns)
, m_data_size(0)
, m_data(WD_Null)
{
    WD_Assert(data_size > 0  && data);
    
    if( data_size > 0 && data != WD_Null)
    {
        m_data_size = data_size;
        m_data = new WT_Byte[data_size];
        for ( WT_Unsigned_Integer32 i = 0; i < data_size; i++)
        {
            m_data[i] = data[i];
        }
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_User_Fill_Pattern::Fill_Pattern::operator == ( Fill_Pattern const & pattern ) const
{
    WT_Boolean result = WD_False;
    if (m_rows == pattern.m_rows
        && m_columns == pattern.m_columns
        && m_data_size == pattern.m_data_size)
    {
        // compare internal data
        WT_Unsigned_Integer32 i = 0;
        WT_Byte const * pat_data = pattern.data();
        for (;i < m_data_size; i++)
            if (m_data[i] != pat_data[i])
                break;

        result = (i == m_data_size);
    }
    return result;
}
