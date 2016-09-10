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
#include "dwfcore/win32/StandardFileDescriptor.h"
using namespace DWFCore;



_DWFCORE_API
DWFStandardFileDescriptor::DWFStandardFileDescriptor( DWFFile&              rFile,
                                                      DWORD                 dwDesiredAccess,
                                                      DWORD                 dwShareMode,
                                                      LPSECURITY_ATTRIBUTES pSecurityAttributes,
                                                      DWORD                 dwCreationDisposition,
                                                      DWORD                 dwFlagsAndAttributes )
throw()
                         : DWFFileDescriptor( rFile )
                         , _hFile( INVALID_HANDLE_VALUE )
                         , _dwDesiredAccess( dwDesiredAccess )
                         , _dwShareMode( dwShareMode )
                         , _pSecurityAttributes( pSecurityAttributes )
                         , _dwCreationDisposition( dwCreationDisposition )
                         , _dwFlagsAndAttributes( dwFlagsAndAttributes )
{
    ;
}

_DWFCORE_API
DWFStandardFileDescriptor::DWFStandardFileDescriptor( DWFFile&  rFile,
                                                      HANDLE    hFile )
throw()
                         : DWFFileDescriptor( rFile )
                         , _hFile( hFile )
                         , _dwDesiredAccess( 0 )
                         , _dwShareMode( 0 )
                         , _pSecurityAttributes( NULL )
                         , _dwCreationDisposition( 0 )
                         , _dwFlagsAndAttributes( 0 )
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
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File already open" );
    }

    const DWFString& rFilename = _oFile.name();

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
                                               (const wchar_t*)rFilename,
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
                                           (const wchar_t*)rFilename,
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

        DWFCORE_ZERO_MEMORY( apMBCSFilename, nMBCSBytes );

        _hFile = ::CreateFileA(  apMBCSFilename,
                                _dwDesiredAccess,
                                _dwShareMode,
                                _pSecurityAttributes,
                                _dwCreationDisposition,
                                _dwFlagsAndAttributes,
                                 NULL );
    }

#endif
        //
        // this is Win32 specific - therefore, the filename, 
        // if not ASCII, must be assumed to be UCS2 or UTF16 
        // or whatever Windows uses
        //
    else
    {
        _hFile = ::CreateFileW( (const wchar_t*)rFilename,
                                _dwDesiredAccess,
                                _dwShareMode,
                                _pSecurityAttributes,
                                _dwCreationDisposition,
                                _dwFlagsAndAttributes,
                                 NULL );
    }

        //
        // check handle
        //
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open file" );
    }
}
    
_DWFCORE_API
void
DWFStandardFileDescriptor::close()
throw( DWFException )
{
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle( _hFile );

        _hFile = INVALID_HANDLE_VALUE;
    }
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::size()
throw( DWFException )
{
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    return static_cast<size_t>(::GetFileSize(_hFile, NULL));
}

_DWFCORE_API
off_t
DWFStandardFileDescriptor::seek( int    eOrigin, 
                                 off_t  nOffset )
throw( DWFException )
{
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    return static_cast<off_t>(::SetFilePointer(_hFile, nOffset, NULL, eOrigin));
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::read( void*  pBuffer,
                                 size_t nBytesToRead )
throw( DWFException )
{
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    DWORD dwBytesRead = 0;
    if (::ReadFile(_hFile, pBuffer, (LONG)nBytesToRead, &dwBytesRead, NULL) == 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File read error" );
    }

    return static_cast<size_t>(dwBytesRead);
}

_DWFCORE_API
size_t
DWFStandardFileDescriptor::write( const void* pBuffer, size_t nBytesToWrite )
throw( DWFException )
{
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file currently open" );
    }

    DWORD dwBytesWritten = 0;

    if (::WriteFile(_hFile, pBuffer, (DWORD)nBytesToWrite, &dwBytesWritten, NULL) == 0)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File write error" );
    }

    return static_cast<size_t>(dwBytesWritten);
}

#endif
