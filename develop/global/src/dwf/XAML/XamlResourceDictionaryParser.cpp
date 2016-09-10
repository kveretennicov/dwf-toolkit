//
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

#include "XAML/pch.h"
#include "XAML/XamlResourceDictionaryParser.h"

WT_XAML_Resource_Dictionary_Parser::WT_XAML_Resource_Dictionary_Parser(WT_XAML_File& file) throw(WT_Result)
: _pDictionaryParser(NULL)
, _pXamlFile(&file)
{
    
    _pDictionaryParser = DWFCORE_ALLOC_OBJECT( DWFXMLParser(this) );
    if(_pDictionaryParser == NULL)
        throw WT_Result::Out_Of_Memory_Error;

}

WT_XAML_Resource_Dictionary_Parser::~WT_XAML_Resource_Dictionary_Parser()
throw()
{
    if(dictionaryParser())
        DWFCORE_FREE_OBJECT(_pDictionaryParser);
}

_DWFTK_API
void WT_XAML_Resource_Dictionary_Parser::notifyStartElement( const char*   /*zName*/,
                                                            const char**  /*ppAttributeList*/ )
                                                            throw()
{
}

_DWFTK_API
void WT_XAML_Resource_Dictionary_Parser::notifyEndElement( const char* /*zName*/ )
throw()
{
   
}

_DWFTK_API
void WT_XAML_Resource_Dictionary_Parser::notifyCharacterData( const char* /*zCData*/,
                                                             int  /*nLength*/ ) 
                                                             throw()
{
}

_DWFTK_API
void WT_XAML_Resource_Dictionary_Parser::notifyStartNamespace( const char* /*zPrefix*/,
                                              const char* /*zURI*/ )
                                              throw()
{

}

_DWFTK_API
void WT_XAML_Resource_Dictionary_Parser::notifyEndNamespace( const char*   /*zPrefix*/ )
throw()
{
}

WT_Result WT_XAML_Resource_Dictionary_Parser::parseResource()
{
    try
    {
        dictionaryParser()->parseDocument(*_pXamlFile->xamlDictionaryStreamIn());
    }
    catch(...)
    {
        return WT_Result::Internal_Error;
    }

   
    return WT_Result::Success;
}

