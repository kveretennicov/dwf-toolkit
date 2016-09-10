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

#include "dwfcore/crypto/SymmetricCipher.h"
using namespace DWFCore;



_DWFCORE_API
DWFSymmetricCipher::DWFSymmetricCipher()
throw()
                      : _pIV( NULL )
                      , _pKey( NULL )
{
    ;
}

_DWFCORE_API
DWFSymmetricCipher::~DWFSymmetricCipher()
throw()
{
    ;
}

_DWFCORE_API
void DWFSymmetricCipher::setKey( DWFCryptoKey&          rKey, 
                                 DWFCryptoKey::teType   eKeyType )
throw( DWFException )
{
    switch (eKeyType)
    {
        case DWFCryptoKey::eSymmetricKey:
        {
            _pKey = &rKey;
            break;
        }
        case DWFCryptoKey::eInitializationVector:
        {
            _pIV = &rKey;
            break;
        }
        default:
        {
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The key type specified is invalid for this engine." );
        }
    }
}

_DWFCORE_API
const DWFCryptoKey* const
DWFSymmetricCipher::_key( DWFCryptoKey::teType eKeyType ) const
throw()
{
    DWFCryptoKey* pKey = NULL;

    switch (eKeyType)
    {
        case DWFCryptoKey::eSymmetricKey:
        {
            pKey = _pKey;
            break;
        }
        case DWFCryptoKey::eInitializationVector:
        {
            pKey = _pIV;
            break;
        }
        default:
        {
            pKey = NULL;
        }
    }

    return pKey;
}

#endif
