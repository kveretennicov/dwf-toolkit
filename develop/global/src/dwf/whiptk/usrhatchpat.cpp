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
 
#define DOUBLE_TO_LONG_BIT_SHIFT_16(x)    (long) ((float)x * 65536)
#define LONG_TO_DOUBLE_BIT_SHIFT_16(x) (double) ((double)x * 0.0000152587890625) 
/////////////////////////////////////////////////////////////////////////// 
WT_User_Hatch_Pattern::WT_User_Hatch_Pattern(WT_User_Hatch_Pattern const & pattern)
{
    // copy all the data
    this->m_hashpatnum = pattern.pattern_number();
    this->m_xsize = pattern.xsize();
    this->m_ysize = pattern.ysize();
    Hatch_Pattern * hatch_pattern = WD_Null;
    
    // Increment the reference counted data 
    for ( WT_Unsigned_Integer32 i = 0; i < pattern.pattern_count(); i++)
    {
        hatch_pattern = (Hatch_Pattern*) pattern.pattern(i);
        WD_Assert (hatch_pattern);
        hatch_pattern->increment();  
        
    }
    // clean up all existing heap data 
    std::vector<Hatch_Pattern*>::iterator itr = m_patterns.begin();
    for (; itr != m_patterns.end(); itr++)
    {
        (*itr)->decrement();
        if ((*itr)->count() == 0)
            (*itr)->destroy();
    }
    m_patterns.clear();
    this->m_patterns = pattern.m_patterns;
}
/////////////////////////////////////////////////////////////////////////// 
WT_User_Hatch_Pattern const & WT_User_Hatch_Pattern::operator= 
                                        (WT_User_Hatch_Pattern const & pattern)
{
    // copy all the data
    this->m_hashpatnum = pattern.pattern_number();
    this->m_xsize = pattern.xsize();
    this->m_ysize = pattern.ysize();
    Hatch_Pattern * hatch_pattern = WD_Null;
    
    // Increment the reference counted data 
    for (WT_Unsigned_Integer32 i = 0; i < pattern.pattern_count(); i++)
    {
        hatch_pattern = (Hatch_Pattern*) pattern.pattern(i);
        WD_Assert (hatch_pattern);
        hatch_pattern->increment();
    }
    // clean up all existing heap data 
    std::vector<Hatch_Pattern*>::iterator itr = m_patterns.begin();
    for (; itr != m_patterns.end(); itr++)
    {
        (*itr)->decrement();
        if ((*itr)->count() == 0)
            (*itr)->destroy();
    }
    m_patterns.clear();

    this->m_patterns = pattern.m_patterns;

    return *this;
}

/////////////////////////////////////////////////////////////////////////// 
WT_User_Hatch_Pattern::~WT_User_Hatch_Pattern()
{
    std::vector<Hatch_Pattern*>::const_iterator itr;  
    for (itr = m_patterns.begin(); itr != m_patterns.end() ; itr++)
    {
        (*itr)->decrement();
        if ((*itr)->count() == 0)
            (*itr)->destroy();
    }
    m_patterns.clear();
}

/////////////////////////////////////////////////////////////////////////// 
WT_Integer16    WT_User_Hatch_Pattern::add_pattern(Hatch_Pattern & pattern)
{
    // checks to see if the same pattern is not being added twice
    std::vector<Hatch_Pattern*>::const_iterator itr;  
    WT_Integer16 ret_index = -1;
    WT_Boolean exists = WD_False;
    for (itr = m_patterns.begin(); itr != m_patterns.end() ; itr++)
    {
        if (*itr == &pattern)
        {
            exists = WD_True;
            break;
        }
    }
    if (!exists)
    {   
        pattern.increment(); // increase the ref count
        m_patterns.push_back(&pattern);
        ret_index = (WT_Integer16)m_patterns.size();
    }
    return ret_index;
}

/////////////////////////////////////////////////////////////////////////// 
const WT_User_Hatch_Pattern::Hatch_Pattern*     WT_User_Hatch_Pattern::pattern(
                                                WT_Unsigned_Integer32 index) const 
{
    Hatch_Pattern * ret_pattern = WD_Null;
    if (index < (WT_Unsigned_Integer32)m_patterns.size() && index >= 0 )
        ret_pattern = m_patterns.at(index);
    return ret_pattern;
}

 
/////////////////////////////////////////////////////////////////////////// 
WT_Object::WT_ID    WT_User_Hatch_Pattern::object_id() const
{
    return User_Hatch_Pattern_ID;
}

/////////////////////////////////////////////////////////////////////////// 
WT_Result   WT_User_Hatch_Pattern::materialize(WT_Opcode const & opcode, 
                                              WT_File & file)
{
    WT_Result result;
    switch (opcode.type())
    {
        
        case WT_Opcode::Extended_ASCII:
            {
                result = materialize_ascii(opcode,file);
            } break;
        case WT_Opcode::Extended_Binary:
            {
                result = materialize_binary(opcode,file);
            } break;
        default:
            result = WT_Result::Opcode_Not_Valid_For_This_Object;
    }
    
    this->m_materialized = (result == WT_Result::Success);
    return result;
}

/////////////////////////////////////////////////////////////////////////// 
WT_Result   WT_User_Hatch_Pattern::process(WT_File& file )
{
    WD_Assert (file.user_hatch_pattern_action());
    return (file.user_hatch_pattern_action())(*this,file);
}

/////////////////////////////////////////////////////////////////////////// 
WT_Result   WT_User_Hatch_Pattern::skip_operand(WT_Opcode const &, 
                                               WT_File &)
{
    return WT_Result::Success;
}

 /////////////////////////////////////////////////////////////////////////// 
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_User_Hatch_Pattern::serialize(WT_File & file) const
{
   WD_CHECK (file.dump_delayed_drawable());
    
   // Serialize out in Extended Binary 
   if (file.heuristics().allow_binary_data()) 
   {
        WD_CHECK (serialize_binary(file));
   }
   else // Extended ASCII output
   {
        WD_CHECK (serialize_ascii(file));
   }
   return WT_Result::Success;
}
# else
WT_Result   WT_User_Hatch_Pattern::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_CODE(WHIP_OUTPUT)

/////////////////////////////////////////////////////////////////////////// 
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_User_Hatch_Pattern::sync( WT_File & file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().user_hatch_pattern() )
    {   
        file.rendition().user_hatch_pattern() = *this;
        WD_CHECK (serialize(file));
    }
    return WT_Result::Success;
}
#else
WT_Result WT_User_Hatch_Pattern::sync( WT_File & ) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()
/////////////////////////////////////////////////////////////////////////// 
WT_Boolean  WT_User_Hatch_Pattern::operator== 
                                        (WT_Attribute const & attrib) const
{
    WT_Boolean ret_value = WD_False;
    if (attrib.object_id() == WT_Object::User_Hatch_Pattern_ID)
    {
        ret_value = this->operator ==((WT_User_Hatch_Pattern const &)attrib); 
    }
    return ret_value;
}
 
/////////////////////////////////////////////////////////////////////////// 
WT_Boolean WT_User_Hatch_Pattern::operator== 
                                (WT_User_Hatch_Pattern const & pattern) const
{
    WT_Boolean result = WD_False;
    if (pattern.pattern_number() == this->pattern_number()
        && pattern.xsize() == this->xsize()
        && pattern.ysize() == this->ysize()
        && pattern.m_patterns.size() == this->m_patterns.size())
    {
        std::vector<Hatch_Pattern*>::const_iterator itr = m_patterns.begin();
        std::vector<Hatch_Pattern*>::const_iterator pat_itr = pattern.m_patterns.begin();
        for (; itr != m_patterns.end(); itr++, pat_itr++)
        {
            if (*itr != *pat_itr 
                || ((*itr)->operator!=( **pat_itr)))
            {
                break;
            }
        }
        if (itr == m_patterns.end())
            result = WD_True;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////// 
WT_Boolean WT_User_Hatch_Pattern::operator!= 
                                (WT_User_Hatch_Pattern const & pattern) const
{
    return !(operator==(pattern));
}
///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Hatch_Pattern::serialize_ascii(WT_File & file) const
{
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(UserHatchPattern "));            // write out the opcode
    WD_CHECK (file.write_ascii(m_hashpatnum));              // Write out the index 
    
    if (!this->m_patterns.empty())
    {
        WD_CHECK (file.write((WT_Byte)' '));                // Space
        WD_CHECK (file.write_ascii(m_xsize));               // XSize
        WD_CHECK (file.write((WT_Byte)','));                // ',' delimeter between x and y 
        WD_CHECK (file.write_ascii(m_ysize));               // YSize
        WD_CHECK (file.write((WT_Byte)' '));                // Space
        WD_CHECK (file.write_ascii((WT_Unsigned_Integer16)this->m_patterns.size())); // count
        std::vector<Hatch_Pattern*>::const_iterator itr;  
       
        for (itr = m_patterns.begin(); itr != m_patterns.end(); itr++)
        {
            const double * data = WD_Null; 
            WD_CHECK (file.write((WT_Byte)' '));            // Space
            WD_CHECK (file.write((WT_Byte)'('));            // '('
            WD_CHECK (file.write_ascii((*itr)->x()));       // x
            WD_CHECK (file.write((WT_Byte)' '));            // Space
            WD_CHECK (file.write_ascii((*itr)->y()));       // y
            WD_CHECK (file.write((WT_Byte)' '));            // Space
            WD_CHECK (file.write_ascii((*itr)->angle()));   // angle
            WD_CHECK (file.write((WT_Byte)' '));            // Space
            WD_CHECK (file.write_ascii((*itr)->spacing())); // spacing
            
            // optional Skew and Dash Pattern 
            // Note : The opcode will only write out the skew component 
            //        if the data size is greater then zero. (DESIGN)
            if ((*itr)->data_size() > 0)
            {
                WD_CHECK (file.write((WT_Byte)' '));            // Space
                WD_CHECK (file.write_ascii((*itr)->skew()));    // skew
                WD_CHECK (file.write((WT_Byte)' '));            // Space
                WT_Unsigned_Integer32 size = (*itr)->data_size();
                WD_CHECK (file.write_ascii(size));              // pattern size
                data = (*itr)->data();
                for( WT_Unsigned_Integer32 i = 0; i < size; i++)// pat data 
                {
                    WD_CHECK (file.write((WT_Byte)' '));        // Space
                    WD_CHECK (file.write_ascii(data[i]));       // double 
                }
            }
            WD_CHECK (file.write((WT_Byte)')'));            // ')'
        }
    }
    WD_CHECK (file.write((WT_Byte)')'));                    // closing bracket
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Hatch_Pattern::serialize_binary(WT_File & file) const
{

    WD_CHECK (file.write((WT_Byte) '{'));
    WT_Integer32 size = sizeof(WT_Unsigned_Integer16) + // for the opcode
                        sizeof(WT_Unsigned_Integer16) ; // Hash Pattern number
    
    if (!this->m_patterns.empty())
    {
        std::vector<Hatch_Pattern*>::const_iterator itr;
        size += (sizeof(WT_Unsigned_Integer16) + // X size
                    sizeof(WT_Unsigned_Integer16) + // Y size
                    sizeof(WT_Unsigned_Integer16)); // count 

        WT_Integer32 Hatch_default_size =   sizeof(WT_Unsigned_Integer32) + // x
                                            sizeof(WT_Unsigned_Integer32) + // y
                                            sizeof(WT_Unsigned_Integer32) + // angle
                                            sizeof(WT_Unsigned_Integer32) + // spacing
                                            sizeof(WT_Unsigned_Integer32) + // skew
                                            sizeof(WT_Unsigned_Integer32) ; // pattern data size
        
        for (itr = m_patterns.begin(); itr != m_patterns.end(); itr++)
        {
            size += (Hatch_default_size + ((*itr)->data_size()));
        }
    }
    // write out the opcode 
    WD_CHECK (file.write(size));               // Size
    WD_CHECK (file.write((WT_Unsigned_Integer16)WD_EXBO_SET_USER_HATCH_PATTERN));  
    WD_CHECK (file.write(m_hashpatnum));                     // Hash Pattern number
    
    if (!this->m_patterns.empty())
    {
        WD_CHECK (file.write(m_xsize));                     // XSize
        WD_CHECK (file.write(m_ysize));                     // YSize
        WD_CHECK (file.write((WT_Unsigned_Integer16)this->m_patterns.size())); // count
        WT_Integer32 val;    
        std::vector<Hatch_Pattern*>::const_iterator itr;
        for (itr = m_patterns.begin(); itr != m_patterns.end(); itr++)
        {
            val = DOUBLE_TO_LONG_BIT_SHIFT_16((*itr)->x());    
            WD_CHECK (file.write(val));                     // x
            val = DOUBLE_TO_LONG_BIT_SHIFT_16((*itr)->y());    
            WD_CHECK (file.write(val));                     // y
            val = DOUBLE_TO_LONG_BIT_SHIFT_16((*itr)->angle());
            WD_CHECK (file.write(val));                     // angle
            val = DOUBLE_TO_LONG_BIT_SHIFT_16((*itr)->spacing());
            WD_CHECK (file.write(val));                     // spacing
            val = DOUBLE_TO_LONG_BIT_SHIFT_16((*itr)->skew());
            WD_CHECK (file.write(val));                     // skew
            val = (*itr)->data_size();
            WD_CHECK (file.write(val));                     // pat count
            double const * data = (*itr)->data();        
            for (WT_Integer32 i = 0; i < val; i++)// pat data 
                WD_CHECK (file.write(DOUBLE_TO_LONG_BIT_SHIFT_16(data[i])));        
        }
    }   
    WD_CHECK (file.write((WT_Byte) '}'));                   // Closing tag

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Hatch_Pattern::materialize_ascii(WT_Opcode const & , 
                                                  WT_File & file)
{
    
    WT_Byte open,close;
    WT_Unsigned_Integer16 set_count;
    WD_CHECK (file.eat_whitespace());                       // move to the hash pattern number
    WD_CHECK (file.read_ascii(m_hashpatnum));
    WD_CHECK (file.eat_whitespace());
    WD_CHECK (file.read(close));                            // read possible close
    if (close != ')')                                       //  Has extended data
    {
        WD_CHECK (file.put_back(close));                    // put back the byte   
        WD_CHECK (file.read_ascii(m_xsize));                // read X size
        WD_CHECK (file.read(close));                        // read ','
        
        if (close != ',')
            return  WT_Result::Corrupt_File_Error;
        
        WD_CHECK (file.read_ascii(m_ysize));                // read Y size 
        WD_CHECK (file.eat_whitespace());
        WD_CHECK (file.read_ascii(set_count));              // read set count
        
        
        WT_User_Hatch_Pattern::Hatch_Pattern * hatch_pattern = WD_Null;
        // temp holding variables
        double x,y,angle,spacing,skew;
        WT_Unsigned_Integer32 data_size;
        double* data = WD_Null;
        for (int i = 0; i < set_count ; i++)
        {
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read(open));                     // read opening
            
            if (open != '(')
                return WT_Result::Corrupt_File_Error;

            WD_CHECK (file.read_ascii(x));                  // read x (double)
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read_ascii(y));                  // read y (double)
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read_ascii(angle));              // read angle (double)
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read_ascii(spacing));            // read spacing (double)
            WD_CHECK (file.eat_whitespace());   
            
            // Possible Hatch Pattern ending 
            WD_CHECK (file.read(close));                    // read possible close 
            
            // Set the optional parameters to their default values.
            skew = 0.0;
            data_size = 0;
            data = WD_Null;
            // Hatch Pattern has optional skew + dash pattern
            if (close != ')')
            {
                WD_CHECK (file.put_back(close));            // put back the close byte    
                WD_CHECK (file.read_ascii(skew));           // read skew (double)
                WD_CHECK (file.eat_whitespace());       
                WD_CHECK (file.read_ascii(data_size));      // read pattern count
                
                WD_Assert (data_size > 0);                  // data size must not be zero
                
                if (data_size > 0)                          // safe code  
                {
                    data = new double[data_size];
                    for (WT_Unsigned_Integer32 cnt = 0; cnt < data_size ; cnt++)
                    {
                        WD_CHECK (file.eat_whitespace());           
                        WD_CHECK (file.read_ascii(data[cnt]));  // read data 
                    }
                }
                else                                     
                  return WT_Result::Corrupt_File_Error;     // file is corrupt 

                WD_CHECK (file.eat_whitespace());           // read whitespace
                WD_CHECK (file.read(close));                // read closing
              
                if (close != ')')
                    return WT_Result::Corrupt_File_Error;
            }
              
            // construct the hatch pattern 
            // We don't increment/decrement the ref count here
            hatch_pattern = WT_User_Hatch_Pattern::Hatch_Pattern::Construct(
                                                             x,
                                                             y,
                                                             angle,
                                                             spacing,
                                                             skew,
                                                             data_size,
                                                             data);
            // add the new object to the collection
            hatch_pattern->increment();
            this->m_patterns.push_back(hatch_pattern);
            hatch_pattern = WD_Null;
            
            // clean up 
            if (data)
                delete[] data;
            
        } // end of sets
        // If materialization of all the Hatch pattern data is successful
       
        WD_CHECK (file.eat_whitespace());                   // eat whitespace
        WD_CHECK (file.read(close));                        // read close
        
        if (close != ')')
            return WT_Result::Corrupt_File_Error; 
        
    } 

    return WT_Result::Success;
    
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Hatch_Pattern::materialize_binary(WT_Opcode const & , 
                                                     WT_File & file)
 {
    WT_Byte close;
    WD_CHECK (file.read(m_hashpatnum));                     // read hash pat num
    WD_CHECK (file.read(close));                            // read possible close
    if (close != '}')
    {
        WT_Unsigned_Integer16 set_count;
        WD_CHECK (file.put_back(close));                    // put back the byte   
        WD_CHECK (file.read(m_xsize));                      // read xsize
        WD_CHECK (file.read(m_ysize));                      // read ysize
        WD_CHECK (file.read(set_count));                    // read set count
        
        //temp data 
        WT_User_Hatch_Pattern::Hatch_Pattern * hatch_pattern = WD_Null;
        double x,y,angle,spacing,skew;
        WT_Integer32 temp;
        WT_Unsigned_Integer32 data_size = 0;
        double* data = WD_Null;
            
        for (int i=0; i< set_count; i++)
        {
            WD_CHECK (file.read(temp));                     // read x
            x =  LONG_TO_DOUBLE_BIT_SHIFT_16(temp);          
            WD_CHECK (file.read(temp));                     // read y
            y =  LONG_TO_DOUBLE_BIT_SHIFT_16(temp);          
            WD_CHECK (file.read(temp));                     // read angle
            angle =  LONG_TO_DOUBLE_BIT_SHIFT_16(temp);          
            WD_CHECK (file.read(temp));                     // read spacing
            spacing =  LONG_TO_DOUBLE_BIT_SHIFT_16(temp);          
            WD_CHECK (file.read(temp));                     // read skew
            skew =  LONG_TO_DOUBLE_BIT_SHIFT_16(temp);          
            WD_CHECK (file.read(data_size));                // read pat_count
        
            // Data size can be zero 
            if (data_size > 0)
            {
                data = new double[data_size];
                for (unsigned int ctr = 0; ctr < data_size; ctr++)
                {
                    WD_CHECK (file.read(temp));                 // read data
                    data[ctr] = LONG_TO_DOUBLE_BIT_SHIFT_16(temp);// convert data to double
                }
            }
            
            // construct the hatch pattern 
            hatch_pattern = WT_User_Hatch_Pattern::Hatch_Pattern::Construct(
                                                             x,
                                                             y,
                                                             angle,
                                                             spacing,
                                                             skew,
                                                             data_size,
                                                             data);
            hatch_pattern->increment();
            this->m_patterns.push_back(hatch_pattern);
            
            // cleanup
            if (data)
            {
                delete[] data; 
                data = WD_Null;
            }
        }
        WD_CHECK (file.read(close));                        // read close
        
        if (close != '}')
            return WT_Result::Corrupt_File_Error; 
        
        
    }
    return WT_Result::Success;
 }

///////////////////////////////////////////////////////////////////////////
WT_Result WT_User_Hatch_Pattern::default_process(WT_User_Hatch_Pattern & item, WT_File & file)
{
    file.rendition().user_hatch_pattern() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_User_Hatch_Pattern::Hatch_Pattern::Hatch_Pattern( const double & x, 
                                                    const double & y,
                                                    const double & angle,
                                                    const double & spacing,
                                                    const double & skew,
                                                    WT_Unsigned_Integer32 data_size,
                                                    const double * data )
                                                    throw()
    : DWFCountedObject()
    , m_x(x)
    , m_y(y)
    , m_angle(angle)
    , m_spacing(spacing)
    , m_skew(skew)
    , m_data_size(data_size)
    , m_data(WD_Null)
 {
    // Note : the internal data will be constructed even if the data is null as the correllation 
    //        between the data size and data is important.
    if (data_size > 0)
    {
        m_data = new double[data_size];
        WD_Assert (data);       
        if (data != WD_Null)
        {
            for (WT_Unsigned_Integer32 i = 0; i < data_size; i++)
            {
                m_data[i] = data[i]; 
            }
        }
    }
 }

 ///////////////////////////////////////////////////////////////////////////
 WT_Boolean WT_User_Hatch_Pattern::Hatch_Pattern::operator== (Hatch_Pattern const & pattern) const
 {
     WT_Boolean result = WD_False;
     if (   m_x == pattern.x()
         && m_y == pattern.y()
         && m_angle == pattern.angle()
         && m_spacing == pattern.spacing()
         && m_skew == pattern.skew() 
         && m_data_size == pattern.data_size())
     {
       const double * pat_data = pattern.data();
       WT_Unsigned_Integer32 i = 0;
       // loop the entire double array 
       for (; i < m_data_size; i++)
           if (pat_data[i] != m_data[i]) 
               break;
             
       result = (i == m_data_size);
     }
     return result;
 }
