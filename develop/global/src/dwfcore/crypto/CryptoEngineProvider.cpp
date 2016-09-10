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


#include "dwfcore/crypto/CryptoEngineProvider.h"
using namespace DWFCore;

DWFThreadMutex DWFCryptoEngineProvider::_oEngineLock;

_DWFCORE_API
DWFCryptoEngineProvider::DWFCryptoEngineProvider()
throw()
                       : _oFactories()
{
    //
    //  Initialized the STATIC engine lock
    //
    DWFCryptoEngineProvider::_oEngineLock.init();
}

_DWFCORE_API
DWFCryptoEngineProvider::~DWFCryptoEngineProvider()
throw()
{
    _tFactoryList::Iterator* piFactory = _oFactories.iterator();
    if (piFactory)
    {
        for (; piFactory->valid(); piFactory->next())
        {
            DWFCryptoEngineFactory* pFactory = piFactory->value();
            DWFCORE_FREE_OBJECT( pFactory );
        }

        DWFCORE_FREE_OBJECT( piFactory )
    }

    //
    //  Destroy the STATIC engine lock
    //
    DWFCryptoEngineProvider::_oEngineLock.destroy();
}

_DWFCORE_API
DWFCryptoEngineProvider&
DWFCryptoEngineProvider::_Instance()
throw( DWFException )
{
    static DWFCryptoEngineProvider oEngineProvider;
    return oEngineProvider;
}

_DWFCORE_API
void
DWFCryptoEngineProvider::RegisterEngine( DWFCryptoEngine::teType eEngineType,
                                         DWFCryptoEngineFactory* pFactory )
throw( DWFException )
{
    bool bRegistered = false;
    DWFCryptoEngineProvider& rInstance = _Instance();

    if (pFactory == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A factory must be provided. The factory pointer was NULL." );
    }

    try
    {
        DWFCryptoEngineProvider::_oEngineLock.lock();
        {
            //
            //  Insert the factory for the engine. If an engine with the same id is already registered, 
            //  this will not insert the new engine.
            //
            bRegistered = rInstance._oFactories.insert( int(eEngineType), pFactory, false );
        }
        DWFCryptoEngineProvider::_oEngineLock.unlock();
    }
    catch (...)
    {
        DWFCryptoEngineProvider::_oEngineLock.unlock();
        throw;
    }

    if (!bRegistered)
    {
        _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"An engine factory was already registered to the same identifier." );
    }
}

_DWFCORE_API
DWFCryptoEngine*
DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::teType eEngineType )
throw( DWFException )
{
    DWFCryptoEngineFactory** ppFactory = NULL;
    DWFCryptoEngineProvider& rInstance = _Instance();

    try
    {
        DWFCryptoEngineProvider::_oEngineLock.lock();
        {
            ppFactory = rInstance._oFactories.find( int(eEngineType) );
        }
        DWFCryptoEngineProvider::_oEngineLock.unlock();
    }
    catch (...)
    {
        DWFCryptoEngineProvider::_oEngineLock.unlock();
        throw;
    }

    return (ppFactory ? (*ppFactory)->build(eEngineType) : NULL );
}

#endif

