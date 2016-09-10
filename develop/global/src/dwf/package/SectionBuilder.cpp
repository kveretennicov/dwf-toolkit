//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#include "dwf/Toolkit.h"
#include "dwf/Version.h"
#include "dwf/package/Section.h"
#include "dwf/package/Constants.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/SectionBuilder.h"
using namespace DWFToolkit;


DWFSection::Factory  DWFSectionBuilder::_koDefaultFactory( /*NOXLATE*/L"" );


_DWFTK_API
DWFSectionBuilder::DWFSectionBuilder()
throw()
    : _oFactories()
{
    ;
}

_DWFTK_API
DWFSectionBuilder::~DWFSectionBuilder()
throw()
{
    DWFSection::Factory::tMap::Iterator* piFactory = _oFactories.iterator();
    
    if (piFactory)
    {
        for(; piFactory->valid(); piFactory->next())
        {
            DWFCORE_FREE_OBJECT( piFactory->value() );
        }

        DWFCORE_FREE_OBJECT( piFactory );
    }
}

_DWFTK_API
DWFSection::Factory*
DWFSectionBuilder::addFactory( DWFSection::Factory* pFactory )
throw( DWFException )
{
    if (pFactory == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No factory provided" );
    }

    DWFSection::Factory* pOldFactory = NULL;
    DWFSection::Factory** ppFactory = _oFactories.find( pFactory->type() );

    if (ppFactory)
    {
        pOldFactory = *ppFactory;
    }

    _oFactories.insert( pFactory->type(), pFactory );

    return pOldFactory;
}

_DWFTK_API
DWFSection::Factory*
DWFSectionBuilder::removeFactory( const DWFString& zType )
throw( DWFException )
{
    DWFSection::Factory* pOldFactory = NULL;
    DWFSection::Factory** ppFactory = _oFactories.find( zType );

    if (ppFactory)
    {
        pOldFactory = *ppFactory;
        _oFactories.erase( zType );
    }
    
    return pOldFactory;
}


_DWFTK_API
DWFSection*
DWFSectionBuilder::buildSection( const char**       ppAttributeList,
                                 DWFPackageReader*  pPackageReader )
throw( DWFException )
{
    DWFString zName;
    DWFString zType;
    DWFString zTitle;

    //
    // IMPORTANT: This logic must remain exactly the same as DWFSection ctor
    //

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        //
        // skip over any "dwf:" in the attribute name
        //
        pAttrib = (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0) ?
                  &ppAttributeList[iAttrib][4] :
                  &ppAttributeList[iAttrib][0];

            //
            // set the name
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x01;

            zName.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the type
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x02;

            zType.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the title
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Title) == 0))
        {
            nFound |= 0x04;

            zTitle.assign( ppAttributeList[iAttrib+1] );
        }
    }

    return buildSection( zType, zName, zTitle, pPackageReader );
}

_DWFTK_API
DWFSection*
DWFSectionBuilder::buildSection( const DWFString&  zType,
                                 const DWFString&  zName,
                                 const DWFString&  zTitle,
                                 DWFPackageReader* pPackageReader )
throw( DWFException )
{
    //
    // acquire a specific factory for this section type
    //
    DWFSection::Factory** ppFactory = _oFactories.find( zType );

        //
        // don't have one? no problem, build a "regular" section
        //
    if (ppFactory == NULL)
    {
        return _koDefaultFactory.build( zType, zName, zTitle, pPackageReader );
    }
        //
        // build a typed section
        //
    else
    {
        return (*ppFactory)->build( zName, zTitle, pPackageReader );
    }
}


