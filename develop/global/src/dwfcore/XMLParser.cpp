//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

#ifdef  HAVE_CONFIG_H
#include "dwfcore/config.h"
#endif

#include "dwfcore/XMLParser.h"


namespace DWFCore
{

#ifndef DWFCORE_XML_PARSER_BUFFER_BYTES
#define DWFCORE_XML_PARSER_BUFFER_BYTES 16384
#endif

_DWFCORE_API
DWFXMLParser::DWFXMLParser(DWFXMLCallback* pCallBack) throw(DWFException)
: _apParseBuffer(NULL,true)
, _pXMLParser(NULL) 
, _pCallBack(pCallBack)
, _bIsParsingFinished( false )
{	
   initializeParser();
}

_DWFCORE_API
void DWFXMLParser::initializeParser() 
{
    //Callback should have been set through constructor
    if(_pCallBack == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Callback Pointer is Null " );
    }

    _apParseBuffer = DWFCORE_ALLOC_MEMORY(char, (size_t)DWFCORE_XML_PARSER_BUFFER_BYTES);
    if (_apParseBuffer.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate work buffer" );
    }

    //Free the parser if not Null
    releaseParser();

    //Reset the flag to false.
    _bIsParsingFinished = false;

    //
    // Parser initialization
    //
    _pXMLParser = XML_ParserCreate( NULL );

    //
    // setup the callback
    //
    XML_SetUserData( _pXMLParser, (void*)_pCallBack );

    //
    // use the static handler hooks from this class
    //
    XML_SetElementHandler( _pXMLParser, StartElementHandler, EndElementHandler );

	//
    // use the static handler hooks from the DWFPackageReader
    // this is unused (to date) by the standard DWF sections,
    // however, it is provided nonetheless for extensibility
    //
    XML_SetCharacterDataHandler( _pXMLParser, CharacterDataHandler );

}

_DWFCORE_API
void DWFXMLParser::releaseParser()
{
    if(_pXMLParser != NULL)
    {
		XML_ParserFree( _pXMLParser );
        _pXMLParser = NULL;
    }

}

_DWFCORE_API
void DWFXMLParser::stopParser() throw(DWFException)
{
   XML_Status retCode = XML_StopParser(_pXMLParser,XML_TRUE);

   if(retCode == XML_STATUS_ERROR)
   {
		//
		// capture the error information
		//
        XML_Error xmlerror = XML_GetErrorCode(_pXMLParser);
		DWFString zParseError( XML_ErrorString( xmlerror ) );

		wchar_t zBuffer[32] = {0};
		_DWFCORE_SWPRINTF( zBuffer, 32, /*NOXLATE*/L" [line: %d]", XML_GetCurrentLineNumber(_pXMLParser) );
		zParseError.append( zBuffer );
		//
		// release the parser
		//
		releaseParser();
        if (xmlerror == XML_ERROR_DUPLICATE_ATTRIBUTE)
        {
		    _DWFCORE_THROW( DWFXMLDuplicateAttributeException, zParseError );
        }
        else
        {
		    _DWFCORE_THROW( DWFIOException, zParseError );
        }
   }
}

_DWFCORE_API
DWFXMLParser::~DWFXMLParser()
{
	releaseParser();
}

_DWFCORE_API
void
DWFXMLParser::parseDocument( DWFInputStream& rDocumentStream)
throw( DWFException )
{
	bool bAdvance = false;
    XML_Status retCode = XML_STATUS_OK;
    XML_ParsingStatus status; 
    XML_GetParsingStatus(_pXMLParser, &status);

    if(status.parsing == XML_SUSPENDED)
    {
        retCode = XML_ResumeParser(_pXMLParser);			
        switch(retCode) 
        {
        case XML_STATUS_OK:
            break;
        case XML_STATUS_SUSPENDED:
            bAdvance = true; 
            break;
        default:
            break;
        }
    }
    else if(status.parsing == XML_FINISHED)
    {
        _bIsParsingFinished = true;
        bAdvance=true;
    }
   

 	//
    // The reader may have a filter associated with it, if it does,
    // we need to attach the document stream so the filter can process
    // the initial bytes before Expat gets to look at them.
    // Use this mechanism to handle malformed XML from bad publishers...
    //
	DWFBufferInputStream* pFilterStream = _pCallBack->getStreamFilter();
	if (pFilterStream)
	{
		pFilterStream->chainInputStream( &rDocumentStream, false );
	}

	//
	// with which stream are we finally going to feed Expat?
	//
	DWFInputStream* pReadStream = (pFilterStream ? pFilterStream : &rDocumentStream);
	while(!bAdvance)
	{
		size_t nBytesRead = 0;
		try
		{	
			nBytesRead = pReadStream->read( _apParseBuffer, (size_t)DWFCORE_XML_PARSER_BUFFER_BYTES );			
		}
        catch(...)
        {
            //
            // release the parser
            //
            releaseParser();

            //
            // Throw the exception to the next handler. A common exception is the DWFInvalidPasswordException
            // exception. We re-throw here so higher level code can deal with it.
            //
            throw;
        }
       
            //
            // there seems to be a problem with the parser's EOS
            // logic if this condition exists. the overall byte
            // count appears to have the requirement that it
            // cover the data itself and must exclude any terminating
            // marker if one exists.
            //
        if ((nBytesRead > 0) && (((char*)_apParseBuffer)[nBytesRead-1] == 0))
        {
            nBytesRead--;
        }

        retCode = XML_Parse(_pXMLParser, _apParseBuffer, (int)nBytesRead, nBytesRead==0);
        switch(retCode) 
        {
        case XML_STATUS_OK:
            if(nBytesRead==0) 
            {
                _bIsParsingFinished = true;
                bAdvance=true;
            }
            break;
        case XML_STATUS_SUSPENDED:
            bAdvance = true; 
            break;
		case XML_STATUS_ERROR:
			{
				//
				// capture the error information
				//
                XML_Error xmlerror = XML_GetErrorCode(_pXMLParser);
				DWFString zParseError( XML_ErrorString( xmlerror ) );

				wchar_t zBuffer[32] = {0};
				_DWFCORE_SWPRINTF( zBuffer, 32, /*NOXLATE*/L" [line: %d]", XML_GetCurrentLineNumber(_pXMLParser) );
				zParseError.append( zBuffer );
				//
				// release the parser
				// Clients intending to call parseDocument() again, need to call Initialize() first.
                //
				releaseParser();
                if (xmlerror == XML_ERROR_DUPLICATE_ATTRIBUTE)
                {
				    _DWFCORE_THROW( DWFXMLDuplicateAttributeException, zParseError );
                }
                else
                {
				    _DWFCORE_THROW( DWFIOException, zParseError );
                }
                
				break;
			}
        default:
            break;
        }
    }
}
//
//
// Expat Handlers
//
//
_DWFCORE_API
void XMLCALL
DWFXMLParser::StartElementHandler( void*           pCallback,
                                       const char*     zName,
                                       const char**    ppAttributeList )
throw()
{
    DWFXMLCallback* pXMLCallback = static_cast<DWFXMLCallback*>(pCallback);
    pXMLCallback->notifyStartElement( zName, ppAttributeList );
}

_DWFCORE_API
void XMLCALL
DWFXMLParser::EndElementHandler( void*             pCallback,
                                     const char*       zName )
throw()
{
    DWFXMLCallback* pXMLCallback = static_cast<DWFXMLCallback*>(pCallback);
    pXMLCallback->notifyEndElement( zName );
}

_DWFCORE_API
void XMLCALL
DWFXMLParser::StartNamespaceDeclHandler( void*       pCallback,
                                             const char* zPrefix,
                                             const char* zURI )
throw()
{
    DWFXMLCallback* pXMLCallback = static_cast<DWFXMLCallback*>(pCallback);
    pXMLCallback->notifyStartNamespace( zPrefix, zURI );
}

_DWFCORE_API
void XMLCALL
DWFXMLParser::EndNamespaceDeclHandler( void*         pCallback,
                                           const char*   zPrefix )
throw()
{
    DWFXMLCallback* pXMLCallback = static_cast<DWFXMLCallback*>(pCallback);
    pXMLCallback->notifyEndNamespace( zPrefix );
}

_DWFCORE_API
void XMLCALL
DWFXMLParser::CharacterDataHandler ( void*         pCallback,
                                         const char*   zCData,
                                         int           nLength )
throw()
{
     DWFXMLCallback* pXMLCallback = static_cast<DWFXMLCallback*>(pCallback);
     pXMLCallback->notifyCharacterData( zCData, nLength );
}

}
