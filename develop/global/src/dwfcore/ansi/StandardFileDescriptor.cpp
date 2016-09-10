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

#include "dwfcore/Pointer.h"
#include "dwfcore/ansi/StandardFileDescriptor.h"
using namespace DWFCore;



_DWFCORE_API
DWFStandardFileDescriptor::DWFStandardFileDescriptor( DWFFile&  rFile,
                                                      int       eIOFlags,
                                                      int       eIOMode )
throw()
                         : DWFFileDescriptor( rFile )
                         , _iHandle( -1 )
                         , _eIOFlags( eIOFlags )
                         , _eIOMode( eIOMode )
{
    ;
}

_DWFCORE_API
DWFStandardFileDescriptor::DWFStandardFileDescriptor( DWFFile&  rFile,
                                                      int       iHandle )
throw()
                         : DWFFileDescriptor( rFile )
                         , _iHandle( iHandle )
                         , _eIOFlags( 0 )
                         , _eIOMode( 0 )
{
    ;
}

_DWFCORE_API
DWFStandardFileDescriptor::~DWFStandardFileDescriptor()
throw()
{
    try { close(); } catch (...) {;}
}

_DWFCORE_API
void
DWFStandardFileDescriptor::open()
throw( DWFException )
{
        //
        // already open
        //
    if (_iHandle > -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File already open" );
    }

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
    // open the file per the current option flags
    //
    _iHandle = ::open( pUTF8Filename, _eIOFlags, _eIOMode );

    //
    // free memory
    //
    DWFCORE_FREE_MEMORY( pUTF8Filename );

        //
        // check stream
        //
    if (_iHandle == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open file" );
    }
}

_DWFCORE_API
void
DWFStandardFileDescriptor::close()
throw( DWFException )
{
        //
        // let's be forgiving and not throw exceptions
        //
    if (_iHandle > -1)
    {
        ::close( _iHandle );

        _iHandle = -1;
    }
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::size()
throw( DWFException )
{
    if (_iHandle == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    struct stat tStat;

    if (::fstat(_iHandle, &tStat) == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to stat file" );
    }

    return tStat.st_size;
}

_DWFCORE_API
off_t
DWFStandardFileDescriptor::seek( int eOrigin,
                                 off_t              nOffset )
throw( DWFException )
{
    if (_iHandle == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    if (::lseek(_iHandle, nOffset, eOrigin) == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to seek" );
    }

    //
    // not meaningful
    //
    return -1;
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::read( void* pBuffer, size_t nBufferBytes )
throw( DWFException )
{
    if (_iHandle == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    size_t nBytesRead = ::read( _iHandle, pBuffer, nBufferBytes );
    if (nBytesRead < 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Read failed" );
    }

    return nBytesRead;
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::write( const void* pBuffer, size_t nBufferBytes )
throw( DWFException )
{
    if (_iHandle == -1)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    size_t nBytesWritten = ::write( _iHandle, pBuffer, nBufferBytes );
    if (nBytesWritten < 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Write failed" );
    }

    return nBytesWritten;
}


#endif
