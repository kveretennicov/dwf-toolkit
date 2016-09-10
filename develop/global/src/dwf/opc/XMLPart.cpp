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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/XMLPart.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwfcore/BufferInputStream.h"
#include "dwfcore/String.h"
using namespace DWFCore;

#include "dwf/opc/XMLPart.h"
using namespace DWFToolkit;

//
// use this as a starting point for the document serialization
//
#define _DWFTK_OPC_XMLPART_INITIAL_BUFFER_BYTES  16384


_DWFTK_API
OPCXMLPart::OPCXMLPart()
throw()
          : _pBuffer( NULL )
		  , _useFileStream(true)
{;}


_DWFTK_API
OPCXMLPart::~OPCXMLPart()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }

	//
	//	Release all temp files.
	//
	std::list<DWFTempFile*>::iterator iTempFiles = _lTempFiles.begin();
	for (; iTempFiles != _lTempFiles.end(); iTempFiles++)
	{
		DWFTempFile* pTempFile = *iTempFiles;
		if (pTempFile)
		{
			DWFCORE_FREE_OBJECT(pTempFile);
		}
	}
}


_DWFTK_API
DWFInputStream*
OPCXMLPart::getInputStream()
throw( DWFException )
{
#ifdef DWFTK_READ_ONLY
    // TODO: Decide the best thing to do here. Should we:
    //          - return NULL?
    //          - throw an exception?
    //          - conditionally remove the function?
    return NULL;
#else

	DWFString   tempfilename(L"TKOPCXMLTempStream"); 
	DWFTempFile* pTempTempFile = NULL;
	DWFPointer<DWFBufferOutputStream> apOutputStream( DWFCORE_ALLOC_OBJECT(DWFBufferOutputStream(_DWFTK_OPC_XMLPART_INITIAL_BUFFER_BYTES)), false );
	
	if(_useFileStream)
	{
		pTempTempFile = DWFCore::DWFTempFile::Create(tempfilename, true );
		_lTempFiles.push_back(pTempTempFile);
	}
	else
	{
		//
		// create a buffered stream for serialization
		//
		if (apOutputStream.isNull())
		{
			_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
		}
	}

    //
    // TODO: Need to implement someway to share a single UUID generator across multiple parts
    //       without passing around arguments through getInputStream.
    //

    //
    // create an XMLSerializer
    //
    DWFUUID oUUID;
    DWFPointer<DWFXMLSerializer> apSerializer( DWFCORE_ALLOC_OBJECT(DWFXMLSerializer(oUUID)), false );

    if (apSerializer.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate serializer" );
    }

    //
    // serialize
    //
	if (_useFileStream)
	{
		apSerializer->attach( pTempTempFile->getOutputStream() );
	} 
	else
	{
		apSerializer->attach( apOutputStream );
	}    

    serializeXML( apSerializer );
    apSerializer->detach();

    //
    // free the buffer if it was previously allocated
    //
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }
	DWFInputStream* pInputStream = NULL;
	if (_useFileStream)
	{
		pInputStream = pTempTempFile->getInputStream();
	} 
	else
	{
		//
		// copy the stream buffer
		//
	    size_t nBufferBytes = apOutputStream->buffer( (void**)&_pBuffer );

		//
		// create a buffered input stream for reading
		//
	    pInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pBuffer, nBufferBytes) );
	}	

    if (pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
    }

    //
    // return the document stream (we delete the buffer in our dtor)
    //
    return pInputStream;
#endif
}

