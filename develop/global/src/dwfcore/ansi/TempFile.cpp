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


#include "dwfcore/TempFile.h"


#ifdef  _DWFCORE_FOR_ANSI_SYSTEM_ONLY

#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/ansi/StandardFileDescriptor.h"
using namespace DWFCore;


DWFTempFile::DWFTempFile( DWFFileOutputStream* pOutputStream,
                          bool                 bDeleteOnDestroy )
throw()
           : _pOutputStream( pOutputStream )
           , _bDeleteOnDestroy( bDeleteOnDestroy )
{
    //
    // get the underlying file for this stream
    // so that, if requested, we can delete the file later
    //
    _oFile = _pOutputStream->descriptor()->file();
}

_DWFCORE_API
DWFTempFile::~DWFTempFile()
throw()
{
    if (_pOutputStream)
    {
        //
        // flush out the stream
        //
        try { _pOutputStream->flush(); } catch (...) {;}

        //
        // delete the stream (this takes the descriptor with it)
        //
        DWFCORE_FREE_OBJECT( _pOutputStream );
    }

        //
        // clean up the file - assuming ASCII UTF-8 always...
        //
    if (_bDeleteOnDestroy)
    {
        try
        {
            char* zUTF8Filename = NULL;
            _oFile.name().getUTF8( &zUTF8Filename );

            if (zUTF8Filename)
            {
                ::remove( zUTF8Filename );

                DWFCORE_FREE_MEMORY( zUTF8Filename );
            }
        }
        catch (DWFException&)
        {
            ;
        }
    }
}

_DWFCORE_API
DWFTempFile*
DWFTempFile::Create( DWFString& zTemplate,
                     bool       bDeleteOnDestroy )
throw( DWFException )
{
    zTemplate.append( /*NOXLATE*/L"XXXXXX" );

    //
    // create a copy of the template locally
    // ::mkstemp() will modify this value
    //
    char* pTemplate = NULL;

    //
    // must have UTF8 filenames
    //
    zTemplate.getUTF8( &pTemplate );

    //
    // create and open (O_EXCL) a temporary file
    //
    int fd = ::mkstemp( pTemplate );

    if (fd < 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to create temporary file" );
    }

    //
    // wrap the file handle
    //
    DWFFile oTempFile( pTemplate );
    DWFStandardFileDescriptor* pFD = DWFCORE_ALLOC_OBJECT( DWFStandardFileDescriptor(oTempFile, fd) );

    if (pFD == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file descriptor" );
    }

    //
    // create new output stream for writing to this file
    //
    DWFFileOutputStream* pStream = DWFCORE_ALLOC_OBJECT( DWFFileOutputStream );

    if (pStream == NULL)
    {
        DWFCORE_FREE_OBJECT( pFD );

        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new stream" );
    }

    //
    // bind the descriptor to the stream
    //
    pStream->attach( pFD, true );

    //
    // Build the new object
    //
    DWFTempFile* pNew = DWFCORE_ALLOC_OBJECT( DWFTempFile(pStream, bDeleteOnDestroy) );
    if (pNew == NULL)
    {
        DWFCORE_FREE_OBJECT( pStream );

        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new temporary file object" );
    }

    return pNew;
}

_DWFCORE_API
DWFFileOutputStream&
DWFTempFile::getOutputStream()
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No valid output stream is available" );
    }

        //
        // store the underlying file
        //
    if (_oFile.name().chars() == 0)
    {
        _oFile = _pOutputStream->descriptor()->file();
    }

    return *_pOutputStream;
}

_DWFCORE_API
DWFFileInputStream*
DWFTempFile::getInputStream()
throw( DWFException )
{
        //
        // if the output stream still exists
        // shut it down and close the file
        //
    if (_pOutputStream)
    {
        //
        // flush the output stream
        //
        _pOutputStream->flush();

        //
        // delete the stream (this takes the original descriptor with it)
        //
        DWFCORE_FREE_OBJECT( _pOutputStream );  _pOutputStream = NULL;
    }

    //
    // allocate new stream
    //
    DWFFileInputStream* pInputStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
    if (pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate input stream" );
    }

    //
    // create a new read-only descriptor
    //
    DWFStreamFileDescriptor* pFD = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(_oFile, "rb") );
    if (pFD == NULL)
    {
        DWFCORE_FREE_OBJECT( pInputStream );
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new stream file descriptor" );
    }

    //
    // open and bind the temp file to the new stream
    //
    pFD->open();
    pInputStream->attach( pFD, true );

    return pInputStream;
}


#endif

