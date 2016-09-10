//
//  Copyright (c) 2000-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
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



#include "dwf/publisher/EmbeddedFont.h"
using namespace DWFToolkit;


_DWFTK_API
DWFEmbeddedFont::DWFEmbeddedFont( DWFEmbeddedFontImpl* pImpl )
throw()
               : _pImpl( pImpl )
{
    ;
}

_DWFTK_API
DWFEmbeddedFont::~DWFEmbeddedFont()
throw()
{
    if (_pImpl)
    {
        DWFCORE_FREE_OBJECT( _pImpl );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFEmbeddedFont::embed()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    _pImpl->embed();
}

_DWFTK_API
void
DWFEmbeddedFont::addCharacters( const DWFString& rCharacters )
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    _pImpl->addCharacters( rCharacters );
}

_DWFTK_API
DWFInputStream*
DWFEmbeddedFont::getInputStream()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getInputStream();
}

_DWFTK_API
const DWFString&
DWFEmbeddedFont::getFaceName()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getFaceName();
}

_DWFTK_API
const DWFString&
DWFEmbeddedFont::getLogfontName()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getLogfontName();
}


#endif

_DWFTK_API
int
DWFEmbeddedFont::getRequest()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getRequest();
}

_DWFTK_API
DWFFontResource::tePrivilege
DWFEmbeddedFont::getPrivilege()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getPrivilege();
}

_DWFTK_API
DWFFontResource::teCharacterCode
DWFEmbeddedFont::getCharacterCode()
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->getCharacterCode();
}

_DWFTK_API
void
DWFEmbeddedFont::load( bool bForceRename )
throw ( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->load( bForceRename );
}

_DWFTK_API
const DWFString&
DWFEmbeddedFont::loadedFontName() const
throw( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    return _pImpl->loadedFontName();
}

_DWFTK_API
void 
DWFEmbeddedFont::unload()
throw ( DWFException )
{
    if (_pImpl == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No implementation provided" );
    }

    _pImpl->unload();
}
