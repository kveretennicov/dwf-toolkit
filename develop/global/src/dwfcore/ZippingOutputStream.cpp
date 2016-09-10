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

#include "dwfcore/ZippingOutputStream.h"
#include "dwfcore/zip/password.h"
using namespace DWFCore;


_DWFCORE_API
DWFZippingOutputStream::DWFZippingOutputStream( zipFile                             pZipStream,
                                                DWFZipFileDescriptor::teFileMode    eZipMode )
throw()
                       : DWFOutputStream()
                       , _bFileOpen( false )
                       , _pZipStream( pZipStream )
                       , _eZipMode( eZipMode )
{
}

_DWFCORE_API
DWFZippingOutputStream::~DWFZippingOutputStream()
throw()
{
    if (_bFileOpen)
    {
        zipCloseFileInZip( _pZipStream );
    }
}

_DWFCORE_API
void
DWFZippingOutputStream::open( const DWFString& zArchiveFile,
                              const DWFString& zPassword,
                              bool bPKZIPCompliantPassword )
throw( DWFException )
{
    _bFileOpen = false;

    if (_pZipStream == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No archive open for zipping" );
    }

    void * extrafield_local = NULL;
    char extrafield_local_buf[4];
    uInt size_extrafield_local = 0;
    if(bPKZIPCompliantPassword == false)
    {
        extrafield_local_buf[0] = kzDWFLocalFileHeader[0];
        extrafield_local_buf[1] = kzDWFLocalFileHeader[1];
        extrafield_local_buf[2] = kzDWFLocalFileHeader[2];
        extrafield_local_buf[3] = kzDWFLocalFileHeader[3];
        extrafield_local_buf[3] |= kzDWFSaltedPasswordMask;

        extrafield_local = extrafield_local_buf;
        size_extrafield_local = 4;
    }

    if (Z_OK != zipOpenNewFileInZip( _pZipStream,
                                     zArchiveFile,
                                     NULL,
                                     extrafield_local, size_extrafield_local,
                                     NULL, 0,
                                     "",
                                     Z_DEFLATED,
                                     _eZipMode,
                                     zPassword))
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open new file in archive" );
    }

    _bFileOpen = true;
}

_DWFCORE_API
void
DWFZippingOutputStream::flush()
throw( DWFException )
{
    // NOP
}

_DWFCORE_API
size_t
DWFZippingOutputStream::write( const void*   pBuffer,
                               size_t        nBytesToWrite )
throw( DWFException )
{
    if (_bFileOpen == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file open for zipping" );
    }

    if (Z_OK != zipWriteInFileInZip(_pZipStream, pBuffer, (unsigned long)nBytesToWrite))
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to deflate bytes" );
    }

    return nBytesToWrite;
}
