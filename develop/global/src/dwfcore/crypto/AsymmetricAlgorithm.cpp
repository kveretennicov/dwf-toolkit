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

#include "dwfcore/crypto/AsymmetricAlgorithm.h"
using namespace DWFCore;



_DWFCORE_API
DWFAsymmetricAlgorithm::DWFAsymmetricAlgorithm()
throw()
                      : DWFEncryptionEngine()
                      , _pPublicKey( NULL )
                      , _pPrivateKey( NULL )
{
    ;
}

_DWFCORE_API
DWFAsymmetricAlgorithm::~DWFAsymmetricAlgorithm()
throw()
{
    ;
}

_DWFCORE_API
void DWFAsymmetricAlgorithm::setKey( DWFCryptoKey&          rKey,
                                     DWFCryptoKey::teType   eKeyType )
throw( DWFException )
{
    switch (eKeyType)
    {
        case DWFCryptoKey::eAsymmetricPublicKey:
        {
            _pPublicKey = &rKey;
            break;
        }
        case DWFCryptoKey::eAsymmetricPrivateKey:
        {
            _pPrivateKey = &rKey;
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
DWFAsymmetricAlgorithm::_key( DWFCryptoKey::teType eKeyType ) const
throw()
{
    DWFCryptoKey* pKey = NULL;

    switch (eKeyType)
    {
        case DWFCryptoKey::eAsymmetricPublicKey:
        {
            pKey = _pPublicKey;
            break;
        }
        case DWFCryptoKey::eAsymmetricPrivateKey:
        {
            pKey = _pPrivateKey;
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
