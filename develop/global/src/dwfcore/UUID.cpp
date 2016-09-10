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

#ifdef  _DWFCORE_WIN32_SYSTEM
#include "dwfcore/win32/UUID.h"
#endif

#include "dwfcore/ansi/UUID.h"
using namespace DWFCore;


_DWFCORE_API
DWFUUID::DWFUUID()
throw()
       : _pImpl( NULL )
{
    ;
}

_DWFCORE_API
DWFUUID::DWFUUID( const DWFUUID& zuuid )
throw()
       : _pImpl( NULL )
{
    _pImpl = zuuid._pImpl->clone();
}

_DWFCORE_API
DWFUUID::~DWFUUID()
throw()
{
    DWFCORE_FREE_OBJECT(_pImpl);    
}


_DWFCORE_API
DWFUUID&
DWFUUID::operator=( const DWFUUID& zuuid )
throw()
{
    _pImpl = zuuid._pImpl->clone();
    return *this;
}

_DWFCORE_API
const DWFString&
DWFUUID::uuid( bool bSquash )
throw( DWFException )
{
    if (_pImpl == NULL)
    {

#ifdef  _DWFCORE_WIN32_SYSTEM
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_Win32 );
#else
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_ANSI );
#endif
        if (_pImpl == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to initialize object" );
        }
    }

#ifdef  _DWFCORE_WIN32_SYSTEM

    try
    {
        return _pImpl->uuid( bSquash );
    }
        //
        // Win32 API can fail, if it does rebuild with ANSI impl
        //
    catch (DWFNotImplementedException&)
    {
        //
        // delete old impl
        //
        DWFCORE_FREE_OBJECT( _pImpl );

        //
        // create ANSI impl
        //
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_ANSI );
        if (_pImpl == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to initialize object" );
        }
    }
#endif

    return _pImpl->uuid( bSquash );
}

_DWFCORE_API
const DWFString&
DWFUUID::next( bool bSquash )
throw( DWFException )
{
    if (_pImpl == NULL)
    {

#ifdef  _DWFCORE_WIN32_SYSTEM
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_Win32 );
#else
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_ANSI );
#endif
        if (_pImpl == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to initialize object" );
        }
    }

#ifdef  _DWFCORE_WIN32_SYSTEM

    try
    {
        return _pImpl->next( bSquash );
    }
        //
        // Win32 API can fail, if it does rebuild with ANSI impl
        //
    catch (DWFNotImplementedException&)
    {
        //
        // delete old impl
        //
        DWFCORE_FREE_OBJECT( _pImpl );

        //
        // create ANSI impl
        //
        _pImpl = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_ANSI );
        if (_pImpl == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to initialize object" );
        }
    }
#endif

    return _pImpl->next( bSquash );
}

_DWFCORE_API
DWFUUID
DWFUUID::next()
throw( DWFException )
{
    //
    // copy myself
    //
    DWFUUID oNext( *this );

    //
    // advance and return the copy
    //
    oNext.next( false );
    return oNext;
}

