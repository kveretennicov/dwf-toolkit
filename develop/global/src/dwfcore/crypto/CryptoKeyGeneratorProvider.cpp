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


#include "dwfcore/crypto/CryptoKeyGeneratorProvider.h"
using namespace DWFCore;

DWFThreadMutex DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock;

_DWFCORE_API
DWFCryptoKeyGeneratorProvider::DWFCryptoKeyGeneratorProvider()
throw()
                       : _oFactories()
{
    //
    //  Initialized the STATIC KeyGenerator lock
    //
    DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.init();
}

_DWFCORE_API
DWFCryptoKeyGeneratorProvider::~DWFCryptoKeyGeneratorProvider()
throw()
{
    _tFactoryList::Iterator* piFactory = _oFactories.iterator();
    if (piFactory)
    {
        for (; piFactory->valid(); piFactory->next())
        {
            DWFCryptoKeyGeneratorFactory* pFactory = piFactory->value();
            DWFCORE_FREE_OBJECT( pFactory );
        }

        DWFCORE_FREE_OBJECT( piFactory )
    }

    //
    //  Destroy the STATIC KeyGenerator lock
    //
    DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.destroy();
}

_DWFCORE_API
DWFCryptoKeyGeneratorProvider&
DWFCryptoKeyGeneratorProvider::_Instance()
throw( DWFException )
{
    static DWFCryptoKeyGeneratorProvider oKeyGeneratorProvider;
    return oKeyGeneratorProvider;
}

_DWFCORE_API
void
DWFCryptoKeyGeneratorProvider::RegisterKeyGenerator( DWFCryptoKeyGenerator::teType eKeyGeneratorType,
                                                     DWFCryptoKeyGeneratorFactory* pFactory )
throw( DWFException )
{
    bool bRegistered = false;
    DWFCryptoKeyGeneratorProvider& rInstance = _Instance();

    if (pFactory == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A factory must be provided. The factory pointer was NULL." );
    }

    try
    {
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.lock();
        {
            //
            //  Insert the factory for the key generator. If generator factory with the same id 
            //  is already registered, this will not insert the new factory.
            //
            bRegistered = rInstance._oFactories.insert( int(eKeyGeneratorType), pFactory, false );
        }
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.unlock();
    }
    catch (...)
    {
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.unlock();
        throw;
    }

    if (!bRegistered)
    {
        _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"An key generator factory was already registered to the same identifier." );
    }
}

_DWFCORE_API
DWFCryptoKeyGenerator*
DWFCryptoKeyGeneratorProvider::ProvideKeyGenerator( DWFCryptoKeyGenerator::teType eKeyGeneratorType )
throw( DWFException )
{
    DWFCryptoKeyGeneratorFactory** ppFactory = NULL;
    DWFCryptoKeyGeneratorProvider& rInstance = _Instance();

    try
    {
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.lock();
        {
            ppFactory = rInstance._oFactories.find( int(eKeyGeneratorType) );
        }
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.unlock();
    }
    catch (...)
    {
        DWFCryptoKeyGeneratorProvider::_oKeyGeneratorLock.unlock();
        throw;
    }

    return (ppFactory ? (*ppFactory)->build(eKeyGeneratorType) : NULL );
}

#endif

