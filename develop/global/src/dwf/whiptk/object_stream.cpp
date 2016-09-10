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
#define WT_OBJECT_STREAM_BLOCK_SIZE 5



// Constructors and destructor

///////////////////////////////////////////////////////////////////////////
WT_Object_Stream::WT_Object_Stream()
    : m_ppArray( WD_Null )
    , m_allocated( 0 )
    , m_elements( 0 )
    , m_current( -1 )
    , m_streamOwner( WD_True )
{
}


///////////////////////////////////////////////////////////////////////////
WT_Object_Stream::~WT_Object_Stream()
{
    if( m_ppArray != WD_Null )
    {
        if ( m_streamOwner )
        {
            for( unsigned int ctr = 0; ctr < m_elements; ctr++)
            {
                delete m_ppArray[ctr];
            }
        }
        delete[] m_ppArray;
    }
}

   // operations
   // Adds another item to the end of the definition stream

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Stream::addObject( const WT_Object* object )
{
    this->add_to_array(object);
    return WT_Result::Success;
}

   // Read/Write from file.

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Stream::serialize_stream( WT_File& file ) const
{
    if( m_elements > 0)
    {
        // Note : Binary mode is not supported as the offset size is incorrect due
        //        to not having the offset sizes of all their children.
        // bool bExAsciiMode = !file.heuristics().allow_binary_data();  
        
        //if (bExAsciiMode) 
        //{ 
        //    WD_CHECK (file.write((WT_Byte)'(')); // write EXAO opening
        //}        
        //else
        //{
        //    WD_CHECK (file.write((WT_Byte)'{')); // write EXBO opening
        //}
            
        WD_CHECK (file.write((WT_Byte)'(')); // write EXAO opening

        // loop over the collection and write it out .
        for( unsigned int ctr = 0; ctr < m_elements; ctr++)
        {
            m_ppArray[ctr]->serialize(file);
        }

        //if (bExAsciiMode ) 
        //{
        //    WD_CHECK (file.write((WT_Byte)')')); // write EXAO ending
        //}
        //else 
        //{
        //    WD_CHECK (file.write((WT_Byte)'}')); // write EXBO ending
        //}
        WD_CHECK (file.write((WT_Byte)')')); // write EXAO ending
    }
    
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Stream::materialize_stream( WT_Opcode const &  opcode,  
                                                WT_File & file )
{
    bool bAsciiMode = (opcode.type() == WT_Opcode::Extended_ASCII);
    
    WT_Byte param;
    WT_Result result = WT_Result::Success;
    
    //read opening
    WD_CHECK(file.read(param)); // Read opening paran
    
    
    if ((bAsciiMode && param != '(') || (!bAsciiMode && param != '{'))
    {
        result = WT_Result::Corrupt_File_Error;
    }
    else
    {
        WT_Object* pObject = WD_Null;
        WT_Opcode inner_opcode;
        bool isMacro = true;
        while (isMacro) 
        { 
            // get opcode  // there should always be atleast one opcode       
            WD_CHECK (inner_opcode.get_opcode(file));

            pObject = inner_opcode.object_from_opcode(file.rendition(),
                                                            result,
                                                            file);
            if (pObject)
            {
                WD_CHECK (pObject->materialize(inner_opcode,file));
                
                // calls the materialize subcall
                WD_CHECK (this->on_materialize(*pObject, file));     
                
                // add to the internal array 
                add_to_array(pObject);

                // skip past any extra newline, tab, carriage return or space characters
                // in ASCII mode.
                if(bAsciiMode)
                {
                    file.eat_whitespace();
                }

                // read ahead one byte;
                file.read(param);
                
                // Terminating characters for Macro differ in ExtendedBinary and Ascii modes
                if((bAsciiMode && param == ')') || (!bAsciiMode && param == '}'))
                {
                    isMacro = false;
                }
                else
                {
                    file.put_back(param);
                }
             } // result
            else    // file is corrupted
            {
                result = WT_Result::Corrupt_File_Error;
                isMacro = false;
            }
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Stream::reset() const 
{
    m_current = -1;
}


///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Object_Stream::get_next() const
{
    WT_Boolean ret_value = WD_False;
    if ((m_current + 1) < (int)m_elements)
    {
        m_current++;
        ret_value = WD_True;
    }
    return ret_value;
}


///////////////////////////////////////////////////////////////////////////
const WT_Object* WT_Object_Stream::object() const
{
    const WT_Object* ret_value = WD_Null;
    if (m_current >= 0)
    {
        ret_value = m_ppArray[m_current]; 
    }
    return ret_value;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Stream::on_materialize( WT_Object&, 
                                            WT_File&  )
{
    return WT_Result::Success;      
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Stream::add_to_array( const WT_Object* object )
{
    //add it to the internal array 
    // if no. of elements are the same as the allocated amount 
    // grow/allocate the array
    if( m_elements == m_allocated)
    {
        int allocationSize = m_allocated + WT_OBJECT_STREAM_BLOCK_SIZE; 
        const WT_Object** ppArray = WD_Null;
        ppArray = new const WT_Object*[allocationSize];
        
        if (ppArray == WD_Null)
        {
            throw WT_Result::Out_Of_Memory_Error;
        }
        // copy the elements 
        for( unsigned int ctr = 0; ctr < m_elements ; ctr++ )
        {
            ppArray[ctr] = m_ppArray[ctr];
        }

        // delete the old array 
        if( m_ppArray != WD_Null)
        {
            delete[] m_ppArray;
        }

        // copy new array to member variable 
        m_ppArray = ppArray;
        m_allocated = allocationSize;
    }

    // Add new object to the end of the array and increment m_elements
    m_ppArray[m_elements++] = object; 
        
}
// End of file. 
