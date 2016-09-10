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


#include "dwfcore/UnzippingInputStream.h"
using namespace DWFCore;



_DWFCORE_API
DWFUnzippingInputStream::DWFUnzippingInputStream( unzFile pUnzipStream )
throw()
                       : DWFInputStream()
                       , _bFileOpen( false )
                       , _nFileBytesRemaining( 0 )
                       , _pUnzipStream( pUnzipStream )
                       , _pDescriptor( NULL )
                       , _bOwnDescriptor( false )
{
}

_DWFCORE_API
DWFUnzippingInputStream::~DWFUnzippingInputStream()
throw()
{
    if (_bFileOpen)
    {
        unzCloseCurrentFile( _pUnzipStream );
    }

    if (_pDescriptor)
    {
        if (_bOwnDescriptor)
        {
            DWFCORE_FREE_OBJECT( _pDescriptor );
        }
        else
        {
            _pDescriptor->unobserve( *this );
        }
    }
}

_DWFCORE_API
void
DWFUnzippingInputStream::open( const DWFString& zArchivedFile,
                               const DWFString& zPassword )
throw( DWFException )
{
    _bFileOpen = false;
    _nFileBytesRemaining = 0;

    if (_pUnzipStream == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No archive open for unzipping" );
    }

        //
        // locate the file
        //
    if (unzLocateFile(_pUnzipStream, zArchivedFile, 1) == UNZ_OK)
    {
            //
            // open the file
            //
        if (unzOpenCurrentFile(_pUnzipStream, zPassword, zArchivedFile) == UNZ_OK)
        {
            unz_file_info tFileInfo;
            unzGetCurrentFileInfo( _pUnzipStream, &tFileInfo, NULL, 0, NULL, 0, NULL, 0 );

            _nFileBytesRemaining = tFileInfo.uncompressed_size;
            //_bIsEncrypted = tFileInfo.flag & 0x01;
        }
        else
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open archived file" );
        }
    }
    else
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"File not found in archive" );
    }

    _bFileOpen = true;
}


_DWFCORE_API
size_t
DWFUnzippingInputStream::available() const
throw( DWFException )
{
    if (_bFileOpen == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file open for unzipping" );
    }

    return _nFileBytesRemaining;
}

_DWFCORE_API
size_t
DWFUnzippingInputStream::read( void* pBuffer,
                               size_t nBytesToRead )
throw( DWFException )
{
    if (_bFileOpen == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No file open for unzipping" );
    }

    //
    // stream from the archive directly to the caller
    //
    int nBytesRead = unzReadCurrentFile( _pUnzipStream, pBuffer, static_cast<unsigned int>(nBytesToRead) );

        //
        // an error occurred
        //
    if (nBytesRead < 0)
    {
        if (nBytesRead == UNZ_BADPASSWORD)
        {
            _DWFCORE_THROW( DWFInvalidPasswordException, /*NOXLATE*/L"Bad password provided" );
        }
        else
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Internal zip library error occured" );
        }
    }

    _nFileBytesRemaining -= (size_t)nBytesRead;

    // TODO this can be enabled when we have time to beat on this
    //  -Sriram 6/18/04

    /*
    //
    // if we are encrypted and if the bytes we have uncompressed
    // doesnt match the uncompressed size then we probably have
    // a bad password. we didnt get UNZ_BADPASSWORD because the stream
    // must have appeared to be a valid compressed stream
    //
    if (_bIsEncrypted && _nFileBytesRemaining != 0 && (int)nBytesToRead != nBytesRead)
    {
        _DWFCORE_THROW( DWFInvalidPasswordException, L"Bad password provided" );
    }
    */

    return (size_t)nBytesRead;
}

_DWFCORE_API
off_t
DWFUnzippingInputStream::seek( int  /*eOrigin*/,
                               off_t /*nOffset*/ )
throw( DWFException )
{
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Seeking not permitted on this stream" );
}

_DWFCORE_API
void
DWFUnzippingInputStream::attach( DWFZipFileDescriptor*  pFileDescriptor,
                                 bool                   bOwnDescriptor  )
throw()
{
    _pDescriptor = pFileDescriptor;
    _bOwnDescriptor = bOwnDescriptor;
    if (_bOwnDescriptor)
    {
        pFileDescriptor->own( *this );
    }
    else
    {
        pFileDescriptor->observe( *this );
    }
}

_DWFCORE_API
void DWFUnzippingInputStream::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
    throw( DWFException )
{
    //
    // we no longer own this descriptor
    //
    _bOwnDescriptor = false;
}

_DWFCORE_API
void DWFUnzippingInputStream::notifyOwnableDeletion( DWFOwnable& /*rOwnable*/ )
    throw( DWFException )
{
    //
    // this descriptor is going away
    //
    _pDescriptor = NULL;
}
