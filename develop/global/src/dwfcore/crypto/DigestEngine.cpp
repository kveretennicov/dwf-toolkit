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

#ifndef DWFCORE_DISABLE_CRYPTO

#include "dwfcore/crypto/DigestEngine.h"
using namespace DWFCore;

_DWFCORE_API
DWFDigestEngine::DWFDigestEngine()
    throw()
    : DWFCryptoEngine()
    , _zDigestBytes( NULL )
    , _nBytes( 0 )
{
    ;
}

_DWFCORE_API
DWFDigestEngine::~DWFDigestEngine()
throw()
{
    DWFDigestEngine::reset();
}

_DWFCORE_API
void
DWFDigestEngine::_storeBytes( unsigned char* zBytes, size_t nBytes )
throw( DWFException )
{
    DWFDigestEngine::reset();

    if (zBytes != NULL && 
        nBytes > 0)
    {
        _zDigestBytes = DWFCORE_ALLOC_MEMORY( unsigned char, nBytes );
        DWFCORE_ZERO_MEMORY( _zDigestBytes, nBytes );
        if (_zDigestBytes == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"No memory allocated for digest bytes" );
        }
        DWFCORE_COPY_MEMORY( _zDigestBytes, zBytes, nBytes );
        _nBytes = nBytes;
    }
}

#endif



