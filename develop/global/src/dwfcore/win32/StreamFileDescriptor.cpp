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


#ifdef  DWFCORE_USE_WIN32_FILE

#include "dwfcore/Pointer.h"
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

    const DWFString& rFilename( _oFile.name() );


#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

        //
        // must force into ASCII/MBCS/DBCS on Win9x systems
        //
    if (IsWindows9x())
    {
        //
        // get recommended converted string byte length
        //
        int nMBCSBytes = ::WideCharToMultiByte( CP_ACP, 0, 
                                                rFilename,
                                               (int)rFilename.bytes(),
                                                NULL, 0,
                                                NULL, NULL );

        //
        // This is a generous error pad but something is necessary
        // to account for the random behavior of this function
        // which sometimes returns a byte count that is not big enough
        // to hold the converted string
        //
        nMBCSBytes *= 2;

        //
        // new buffer
        //
        DWFPointer<char> apMBCSFilename( DWFCORE_ALLOC_MEMORY(char, nMBCSBytes), true );
        if (apMBCSFilename.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate temporary string buffer" );
        }

        nMBCSBytes = ::WideCharToMultiByte( CP_ACP, 0, 
                                            rFilename,
                                           (int)rFilename.bytes(),
                                            apMBCSFilename, 
                                            nMBCSBytes,
                                            NULL, NULL );
            //
            // conversion failed
            //
        if (nMBCSBytes == 0)
        {
            _DWFCORE_THROW( DWFStringException, /*NOXLATE*/L"Failed to convert filename using local ANSI code page" );
        }

        //
        // now do the flags string but since this guy will be only a few chars
        // rip thru it by hand
        //
        char zFlags[8];
        size_t i = 0;
        wchar_t* pIn = const_cast<wchar_t*>((const wchar_t*)_zFlags);

        for (; i < _zFlags.chars(); i++)
        {
            if (*pIn == 0)
            {
                zFlags[i] = 0;
                break;
            }

            zFlags[i] = (char)(*pIn++);
        }

        zFlags[i] = NULL;   // Terminate the string 

        _fp = ::fopen( apMBCSFilename, zFlags );

            //
            // check stream
            //
        if (_fp == NULL)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open file" );
        }

        struct _stat tStat;
        if (_stat( apMBCSFilename, &tStat) == -1)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to stat file" );
        }

        _nFilesize = tStat.st_size;
    }
        //
        // this is Win32 specific - therefore, the filename, 
        // if not ASCII, must be assumed to be UCS2 or UTF16 
        // or whatever Windows uses
        //
    else
#endif
    {
        _fp = ::_wfopen( rFilename, _zFlags );

            //
            // check stream
            //
        if (_fp == NULL)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open file" );
        }

        struct _stat tStat;
        if (_wstat(rFilename, &tStat) == -1)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to stat file" );
        }

        _nFilesize = tStat.st_size;
    }
}

_DWFCORE_API
void
DWFStreamFileDescriptor::close()
throw( DWFException )
{
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
DWFStreamFileDescriptor::seek( int      eOrigin,
                               off_t    nOffset )
throw( DWFException )
{
    if (_fp == NULL)
    {
        open();
    }

    off_t nPrevOffset = ::ftell( _fp );

    if (::fseek(_fp, nOffset, eOrigin) == -1)
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
