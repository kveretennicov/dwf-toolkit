//
//  Copyright (c) 2006 by Autodesk, Inc.
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


#include "dwfcore/DigestOutputStream.h"
using namespace DWFCore;

_DWFCORE_API
DWFDigestOutputStream::DWFDigestOutputStream()
throw()
                    : _pDigest( NULL )
                    , _pOutputStream( NULL )
                    , _bOwnStream( false )
{
    ;
}

_DWFCORE_API
DWFDigestOutputStream::DWFDigestOutputStream( DWFDigest* pDigest,
                                              DWFOutputStream* pOutputStream,
                                              bool bOwnStream )
throw()
                    : _pDigest( pDigest )
                    , _pOutputStream( pOutputStream )
                    , _bOwnStream( bOwnStream )
{
    ;
}

_DWFCORE_API
DWFDigestOutputStream::~DWFDigestOutputStream()
throw()
{
    if (_pOutputStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pOutputStream );
    }

    if (_pDigest)
    {
        DWFCORE_FREE_OBJECT( _pDigest );
    }
}

_DWFCORE_API
void
DWFDigestOutputStream::setDigest( DWFDigest* pDigest )
throw()
{
    if (_pDigest)
    {
        DWFCORE_FREE_OBJECT( _pDigest );
    }
    _pDigest = pDigest;

    _pDigest->reset();
}

_DWFCORE_API
void
DWFDigestOutputStream::chainOutputStream( DWFOutputStream* pOutputStream,
                                          bool             bOwnStream )
throw()
{
    if (_pOutputStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pOutputStream );
    }

    _pOutputStream = pOutputStream;
    _bOwnStream = bOwnStream;

    _pDigest->reset();
}

_DWFCORE_API
bool
DWFDigestOutputStream::isDigestProgressive()
throw( DWFException )
{
    if (_pDigest)
    {
        return _pDigest->isProgressive();
    }
    else
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"A digest engine has not been attached to the digest stream." );
    }
}

_DWFCORE_API
DWFString
DWFDigestOutputStream::digest()
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An output stream has not been linked to the digest stream." );
    }
    if (_pDigest == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"A digest engine has not been attached to the digest stream." );
    }

    return _pDigest->digest();
}

_DWFCORE_API
size_t
DWFDigestOutputStream::digestRawBytes( unsigned char*& rpBytes )
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An output stream has not been linked to the digest stream." );
    }
    if (_pDigest == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"A digest engine has not been attached to the digest stream." );
    }

    return _pDigest->digestRawBytes( rpBytes );
}

_DWFCORE_API
DWFString
DWFDigestOutputStream::digestBase64()
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An output stream has not been linked to the digest stream." );
    }
    if (_pDigest == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"A digest engine has not been attached to the digest stream." );
    }

    return _pDigest->digestBase64();
}

_DWFCORE_API 
void
DWFDigestOutputStream::flush()
throw( DWFException )
{
    if (_pOutputStream != NULL) 
    {
        _pOutputStream->flush();
    }
}

_DWFCORE_API
size_t
DWFDigestOutputStream::write( const void*   pBuffer,
                              size_t        nBytesToWrite )
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An output stream has not been linked to the digest stream." );
    }
    if (_pDigest == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"A digest engine has not been attached to the digest stream." );
    }

    size_t nBytesWritten = _pOutputStream->write( pBuffer, nBytesToWrite );

    _pDigest->update( (const char*)pBuffer, nBytesWritten );

    return nBytesWritten;
}

