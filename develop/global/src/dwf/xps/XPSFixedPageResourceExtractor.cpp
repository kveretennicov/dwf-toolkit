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
//
#include "dwfcore/XMLParser.h"
#include "dwfcore/BufferOutputStream.h"
using namespace DWFCore;

#include "dwf/xps/XPSFixedPageResourceExtractor.h"
#include "dwf/xps/Constants.h"
#include "dwf/opc/Constants.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/opc/ZipFileReader.h"
using namespace DWFToolkit;

_DWFTK_API
XPSFixedPageResourceExtractor::XPSFixedPageResourceExtractor( DWFInputStream* pInputStream,
                                                                bool bOwnStream )
throw( DWFException )
                       : _pInputStream( pInputStream )
                       , _bOwnStream( bOwnStream )
                       , _pCurrentResourceOutputStream( NULL )
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
XPSFixedPageResourceExtractor::~XPSFixedPageResourceExtractor()
throw()
{
    DWFCORE_FREE_OBJECT( _pResourceSerializer );
    DWFCORE_FREE_OBJECT( _pParser );

	DWFCORE_FREE_OBJECT( _pCurrentResourceOutputStream );

    if (_bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pInputStream );
    }
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::parse()
throw()
{
    _pParser->parseDocument( *_pInputStream );
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::notifyStartElement( const char* zName,
                                                    const char** ppAttributeList )
throw()
{
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
			if (DWFCORE_COMPARE_ASCII_STRINGS( pAttrib, /*NOXLATE*/"FontUri" ) == 0)
			{
				DWFString zFontURI(ppAttributeList[iAttrib+1]);
				off_t iOffset = zFontURI.findLast( /*NOXLATE*/L'/' );

				DWFString fileName;
				if (iOffset != -1 &&
					size_t(iOffset+1) != zFontURI.chars())
				{
					size_t iExtLen = zFontURI.chars()-(iOffset+1);
					fileName = zFontURI.substring( iOffset+1, iExtLen);
				}

				char* value = NULL;
				fileName.getUTF8(&value);
				_pResourceSerializer->addAttribute( pAttrib, value );
                DWFCORE_FREE_OBJECT(value);
			}
			else
				_pResourceSerializer->addAttribute( pAttrib, ppAttributeList[iAttrib+1] );
        }
    }

	if (DWFCORE_COMPARE_ASCII_STRINGS( zName, XPSXML::kzElement_FixedPage ) == 0)
	{
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
        return;
	}	
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::notifyEndElement( const char* zName )
throw()
{
	if (DWFCORE_COMPARE_ASCII_STRINGS( zName, XPSXML::kzElement_FixedPage ) == 0)
	{
		_pResourceSerializer->detach();
        //
        // we are done with this resource
        //
        //_pCurrentResourceOutputStream = NULL;
	}
	else
	{
		//
		// check to see if this is the end of our resource
		//
		if (_pCurrentResourceOutputStream)
		{
			_pResourceSerializer->endElement();
		}
	}
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::notifyStartNamespace( const char* /*zPrefix*/,
                                                      const char* /*zURI*/ )
throw()
{
    ;
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
    ;
}

_DWFTK_API
void
XPSFixedPageResourceExtractor::notifyCharacterData( const char* /*zCData*/, 
                                                     int         /*nLength*/ ) 
throw()
{
    ;
}

_DWFTK_API
DWFInputStream*
XPSFixedPageResourceExtractor::getResourceStream( )
throw( DWFException )
{
    //
    // create an input stream from it and return it
    //
    char *pBuffer = NULL;
    size_t nBufferBytes = 0;

    nBufferBytes = _pCurrentResourceOutputStream->buffer((void **)&pBuffer);
    DWFBufferInputStream* pResourceInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream((void *)pBuffer, nBufferBytes, true));

    return pResourceInputStream;
}
