//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/reader/FixedPageResourceExtractor.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//
#include "dwfcore/XMLParser.h"
#include "dwfcore/BufferOutputStream.h"
using namespace DWFCore;

#include "dwf/dwfx/reader/FixedPageResourceExtractor.h"
#include "dwf/xps/Constants.h"
#include "dwf/dwfx/Constants.h"
using namespace DWFToolkit;

_DWFTK_API
DWFXFixedPageResourceExtractor::DWFXFixedPageResourceExtractor( DWFInputStream* pFixedPagePartStream,
                                                                bool bOwnStream )
throw( DWFException )
                       : _pFixedPagePartStream( pFixedPagePartStream )
                       , _bOwnStream( bOwnStream )
                       , _pCurrentResourceOutputStream( NULL )
                       , _nCanvasElementDepth( 0 )
                       , _nResourceCanvasElementDepth( 0 )
                       , _oResourceStreamMap()
{
    _pParser = DWFCORE_ALLOC_OBJECT( DWFXMLParser( this ) );
    if (_pParser == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate parser" );
    }

    _pResourceSerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer( _oUUID ) );
    if (_pResourceSerializer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate xml serializer" );
    }
}

_DWFTK_API
DWFXFixedPageResourceExtractor::~DWFXFixedPageResourceExtractor()
throw()
{
    //
    // delete the output streams that we created
    //
    for( _tResourceLocatorToStream::iterator iStream = _oResourceStreamMap.begin();
        iStream != _oResourceStreamMap.end();
        iStream++)
    {
        DWFCORE_FREE_OBJECT( iStream->second );
    }

    DWFCORE_FREE_OBJECT( _pResourceSerializer );
    DWFCORE_FREE_OBJECT( _pParser );

    if (_pFixedPagePartStream != NULL && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pFixedPagePartStream );
    }

}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::parse()
throw( DWFException )
{
    if (_pFixedPagePartStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"No fixed page part stream to parse" );
    }

    _pParser->parseDocument( *_pFixedPagePartStream );

    if (_bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pFixedPagePartStream );
    }
}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::notifyStartElement( const char* zName,
                                                    const char** ppAttributeList )
throw()
{
    //
    // if this is the start of a Canvas element increment the depth
    //
    if (DWFCORE_COMPARE_ASCII_STRINGS( zName, XPSXML::kzElement_Canvas ) == 0)
    {
        _nCanvasElementDepth++;
    }

    //
    // if we are extracting a resource then copy this element and its attributes to our
    // output stream
    //
    if (_pCurrentResourceOutputStream)
    {
        _pResourceSerializer->startElement( zName );
        size_t iAttrib = 0;
        const char* pAttrib = NULL;

        for(; ppAttributeList[iAttrib]; iAttrib += 2)
        {
            pAttrib = ppAttributeList[iAttrib];
            _pResourceSerializer->addAttribute( pAttrib, ppAttributeList[iAttrib+1] );
        }
    }
    else
    {
        //
        // check to see if this is the start of a resource to be extracted
        //
        if (DWFCORE_COMPARE_ASCII_STRINGS( zName, XPSXML::kzElement_Canvas ) == 0)
        {
            size_t iAttrib = 0;
            const char* pAttributeName = NULL;
            const char* pAttributeValue = NULL;

            //
            // loop through the attributes looking for the name attribute
            //
            for(; ppAttributeList[iAttrib]; iAttrib += 2)
            {
                pAttributeName = ppAttributeList[iAttrib];
                pAttributeValue = ppAttributeList[iAttrib + 1];

                //
                // if this is the name attribute
                //
                if (DWFCORE_COMPARE_ASCII_STRINGS(pAttributeName, XPSXML::kzAttribute_Name) == 0)
                {
                    //
                    // Does it contain our internat id prefix?
                    //
                    if (DWFCORE_COMPARE_ASCII_STRINGS_NCHARS(
                        pAttributeValue,
                        DWFXXML::kzPrefix_ResourceInternalID,
                        DWFCORE_ASCII_STRING_LENGTH( DWFXXML::kzPrefix_ResourceInternalID ) ) == 0)
                    {
                        //
                        // this will be the locator for the resource, so remember it
                        //
                        _zLocator = pAttributeValue;

                        //
                        // also remember the canvas element depth, this will be used to track
                        // the end of this canvas element
                        //
                        _nResourceCanvasElementDepth = _nCanvasElementDepth;

                        //
                        // allocate a buffer output stream to receive this resource
                        //
                        _pCurrentResourceOutputStream = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream( 1024, -1 ) );
                        if (_pCurrentResourceOutputStream == NULL)
                        {
                            //
                            // we cannot throw here
                            //
                            return;
                        }
                        //
                        // attach this output stream to our xml serializer
                        //
                        _pResourceSerializer->attach( *_pCurrentResourceOutputStream );

                        //
                        // we are done looping through our attributes
                        //
                        break;
                    }
                }
            }
        }
    }
}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::notifyEndElement( const char* zName )
throw()
{
    //
    // check to see if this is the end of our resource
    //
    if (_pCurrentResourceOutputStream)
    {
        if (DWFCORE_COMPARE_ASCII_STRINGS( zName, XPSXML::kzElement_Canvas ) == 0)
        {
            if (_nCanvasElementDepth == _nResourceCanvasElementDepth)
            {
                //
                // detach the stream from the serializer and store it away in our map using
                // the locator as the index.
                //
                _pResourceSerializer->detach();
                _oResourceStreamMap.insert( _tResourceLocatorToStream::value_type( _zLocator, _pCurrentResourceOutputStream ) );

                //
                // we are done with this resource
                //
                _pCurrentResourceOutputStream = NULL;
            }
            else
            {
                _pResourceSerializer->endElement();
            }
            --_nCanvasElementDepth;
        }
        else
        {
            _pResourceSerializer->endElement();
        }
    }
}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::notifyStartNamespace( const char* /*zPrefix*/,
                                                      const char* /*zURI*/ )
throw()
{
    ;
}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
    ;
}

_DWFTK_API
void
DWFXFixedPageResourceExtractor::notifyCharacterData( const char* /*zCData*/, 
                                                     int         /*nLength*/ ) 
throw()
{
    ;
}

_DWFTK_API
DWFInputStream*
DWFXFixedPageResourceExtractor::getResourceStream( const DWFString& zLocator )
throw( DWFException )
{
    //
    // look for the stream in our map
    //
    _tResourceLocatorToStream::iterator iStream = _oResourceStreamMap.find( zLocator );
    if (iStream == _oResourceStreamMap.end())
    {
        return NULL;
    }

    //
    // create an input stream from it and return it
    //
    DWFBufferInputStream* pResourceInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream( iStream->second->buffer(), iStream->second->bytes() ) );
    return pResourceInputStream;
}
