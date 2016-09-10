//
//  Copyright (c) 2004-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7/develop/global/src/dwf/package/CustomSection.cpp 1     1/25/05 5:30p Evansg $
//


#include "dwf/package/CustomSection.h"
using namespace DWFToolkit;



_DWFTK_API
DWFCustomSection::DWFCustomSection( DWFPackageReader* pPackageReader )
throw()
                : DWFSection( pPackageReader )
{
    _tBehavior.bRenameOnPublish = true;
    _tBehavior.bPublishDescriptor = false;
    _tBehavior.bPublishResourcesToManifest = false;
}

_DWFTK_API
DWFCustomSection::DWFCustomSection( const DWFString&  zType,
                                    const DWFString&  zName,
                                    const DWFString&  zTitle,
                                    DWFPackageReader* pPackageReader )
throw()
                : DWFSection( zType, zName, zTitle, pPackageReader )
{
    _tBehavior.bRenameOnPublish = true;
    _tBehavior.bPublishDescriptor = false;
    _tBehavior.bPublishResourcesToManifest = false;
}

_DWFTK_API
DWFCustomSection::DWFCustomSection( const DWFString& zType,
                                    const DWFString& zTitle,
                                    const DWFString& zObjectID,
                                    double           nVersion,
                                    double           nPlotOrder,
                                    const DWFSource& rSource )
throw()
                : DWFSection( zType, 
                              zTitle, 
                              zObjectID, 
                              nVersion, 
                              nPlotOrder, 
                              rSource )
{
    _tBehavior.bRenameOnPublish = true;
    _tBehavior.bPublishDescriptor = false;
    _tBehavior.bPublishResourcesToManifest = false;
}

_DWFTK_API
DWFCustomSection::DWFCustomSection( const DWFCustomSection& rSection )
throw()
                : DWFSection( rSection )
{
}

_DWFTK_API
DWFCustomSection&
DWFCustomSection::operator=( const DWFCustomSection& rSection )
throw()
{
    DWFSection::operator=( rSection );

    return *this;
}

_DWFTK_API
DWFCustomSection::~DWFCustomSection()
throw()
{
    ;
}


///
///
///

_DWFTK_API
DWFCustomSection::Factory::Factory( const DWFString& zType )
throw()
                   : _zType( zType )
{
    ;
}

_DWFTK_API
DWFCustomSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
const DWFString&
DWFCustomSection::Factory::type()
const
throw()
{
    return _zType;
}

_DWFTK_API
DWFCustomSection*
DWFCustomSection::Factory::build( const DWFString&  zName,
                                  const DWFString&  zTitle,
                                  DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFCustomSection* pSection = DWFCORE_ALLOC_OBJECT( DWFCustomSection(_zType, zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}

_DWFTK_API
DWFCustomSection*
DWFCustomSection::Factory::build( const DWFString&     zType,
                            const DWFString&     zName,
                            const DWFString&     zTitle,
                            DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFCustomSection* pSection = DWFCORE_ALLOC_OBJECT( DWFCustomSection(zType, zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}

