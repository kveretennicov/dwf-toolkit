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


#include "dwfcore/FileInputStream.h"
using namespace DWFCore;


_DWFCORE_API
DWFFileInputStream::DWFFileInputStream()
throw()
        : _bOwner( false )
        , _nAvailableBytes( 0 )
        , _pFileDescriptor( NULL )
{
    ;
}

_DWFCORE_API
DWFFileInputStream::~DWFFileInputStream()
throw()
{
    if (_bOwner && (_pFileDescriptor != NULL))
    {
        DWFCORE_FREE_OBJECT( _pFileDescriptor );
    }
}

_DWFCORE_API
void
DWFFileInputStream::attach( DWFFileDescriptor*  pFileDescriptor,
                            bool                bOwnDescriptor  )
throw( DWFException )
{
    if (pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file provided" );
    }

    _nAvailableBytes = pFileDescriptor->size();
    _pFileDescriptor = pFileDescriptor;

    _bOwner = bOwnDescriptor;
}

_DWFCORE_API
void
DWFFileInputStream::detach()
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    if (_bOwner)
    {
        DWFCORE_FREE_OBJECT( _pFileDescriptor );
    }

    _pFileDescriptor = NULL;
    _nAvailableBytes = 0;
    _bOwner = false;
}

_DWFCORE_API
size_t
DWFFileInputStream::available() const
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    return _nAvailableBytes;
}

_DWFCORE_API
size_t
DWFFileInputStream::read( void*     pBuffer,
                          size_t    nBytesToRead )
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }


    size_t nBytesRead = _pFileDescriptor->read( pBuffer, nBytesToRead );
    _nAvailableBytes -= nBytesRead;

    return nBytesRead;
}

_DWFCORE_API
off_t
DWFFileInputStream::seek( int   eOrigin,
                          off_t nOffset )
throw( DWFException )
{
    if (_pFileDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No file currently attached" );
    }

    off_t nPrevOffset = _pFileDescriptor->seek( eOrigin, nOffset );

    if (eOrigin == SEEK_SET)
    {
        _nAvailableBytes = _pFileDescriptor->size() - nOffset;
    }
    else if (eOrigin == SEEK_CUR)
    {
        _nAvailableBytes -= nOffset;
    }
    else if (nOffset < 0)
    {
        _nAvailableBytes += nOffset;
    }

    return nPrevOffset;
}

