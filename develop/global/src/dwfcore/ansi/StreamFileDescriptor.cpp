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

#include "dwfcore/Core.h"


#ifdef  DWFCORE_USE_ANSI_FILE

#include "dwfcore/StreamFileDescriptor.h"
using namespace DWFCore;



_DWFCORE_API
DWFStreamFileDescriptor::DWFStreamFileDescriptor( const DWFFile&   rFile,
                                                  const DWFString& zStreamIOFlags )
throw()
                       : DWFFileDescriptor( rFile )
                       , _fp( NULL )
                       , _nFilesize( 0 )
                       , _zFlags( zStreamIOFlags )
{
    ;
}

_DWFCORE_API
DWFStreamFileDescriptor::~DWFStreamFileDescriptor()
throw()
{
    try { close(); } catch (...) {;}
}

_DWFCORE_API
void
DWFStreamFileDescriptor::open()
throw( DWFException )
{
        //
        // already open
        //
    if (_fp != NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File already open" );
    }

    //
    // reset
    //
    _nFilesize = 0;

    //
    // always use UTF-8 encoded filenames
    //
    char* pUTF8Filename = NULL;
    const DWFString& rFilename = _oFile.name();

    //
    // get the encoded filename
    //
    rFilename.getUTF8( &pUTF8Filename );

    //
    // get the ascii flags string
    //
    char* pFlags = NULL;
    _zFlags.getUTF8( &pFlags );

    //
    // open the file per the current option flags
    //
    _fp = ::fopen( pUTF8Filename, pFlags );

    //
    // free memory
    //
    DWFCORE_FREE_MEMORY( pFlags );

        //
        // check stream
        //
    if (_fp == NULL)
    {
        DWFCORE_FREE_MEMORY( pUTF8Filename );

        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open file" );
    }

    struct stat tStat;

    if (::stat(pUTF8Filename, &tStat) == -1)
    {
        DWFCORE_FREE_MEMORY( pUTF8Filename );

        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to stat file" );
    }

    //
    // free memory
    //
    DWFCORE_FREE_MEMORY( pUTF8Filename );

    _nFilesize = tStat.st_size;
}

_DWFCORE_API
void
DWFStreamFileDescriptor::close()
throw( DWFException )
{
        //
        // let's be forgiving and not throw exceptions
        //
    if (_fp != NULL)
    {
        ::fclose( _fp );

        _fp = NULL;
        _nFilesize = 0;
    }
}

_DWFCORE_API
size_t
DWFStreamFileDescriptor::read( void* pBuffer,
                               size_t nBytesToRead )
throw( DWFException )
{
    if (_fp == NULL)
    {
        open(); 
    }

    size_t nBytesRead = ::fread( pBuffer, 1, nBytesToRead, _fp );

    if (nBytesRead != nBytesToRead)
    {
        if (ferror(_fp) != 0)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File read error" );
        }
    }

    return nBytesRead;
}

_DWFCORE_API
off_t
DWFStreamFileDescriptor::seek( int   iOrigin,
                               off_t nOffset )
throw( DWFException )
{
    if (_fp == NULL)
    {
        open(); 
    }

    off_t nPrevOffset = ::ftell( _fp );

    if (::fseek(_fp, nOffset, iOrigin) == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to seek" );
    }

    return nPrevOffset;
}

_DWFCORE_API
size_t
DWFStreamFileDescriptor::size()
throw( DWFException )
{
    if (_fp == NULL)
    {
        open(); 
    }

    return _nFilesize;
}

_DWFCORE_API
size_t
DWFStreamFileDescriptor::write( const void* pBuffer, size_t nBytesToWrite )
throw( DWFException )
{
    if (_fp == NULL)
    {
        open(); 
    }

    size_t nBytesWritten = ::fwrite( pBuffer, 1, nBytesToWrite, _fp );

    if (nBytesWritten != nBytesToWrite)
    {
        if (ferror(_fp) != 0)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File write error" );
        }
    }

    return nBytesWritten;
}

_DWFCORE_API
void
DWFStreamFileDescriptor::flush()
throw( DWFException )
{
    if (_fp == NULL)
    {
        open(); 
    }

    if (::fflush( _fp ) != 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File flush error" );
    }
}

#endif
