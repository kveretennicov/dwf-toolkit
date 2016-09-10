//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/DWFSection.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/DWFSection.h"
#include "dwf/package/Section.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;



#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFXDWFSection::kzName = "descriptor.xml";
//DNT_End

#endif


_DWFTK_API
DWFXDWFSection::DWFXDWFSection( DWFSection* pSection )
throw( DWFException )
             : _pSection( pSection )
             , _oResourceParts()
{
    if (_pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A valid DWFSection must be passed" );
    }

    setName( DWFXDWFSection::kzName );
}

_DWFTK_API
DWFXDWFSection::~DWFXDWFSection()
throw()
{
    DWFXResourcePart::tIterator* piParts = _oResourceParts.iterator();
    if (piParts)
    {
        for (; piParts->valid(); piParts->next())
        {
            DWFXResourcePart* pPart = piParts->get();
            if (pPart->owner() == this)
            {
                DWFCORE_FREE_OBJECT( pPart );
            }
            else
            {
                pPart->unobserve( *this );
            }
        }
        DWFCORE_FREE_OBJECT( piParts );
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
void
DWFXDWFSection::setPath( const DWFString& zPath )
throw()
{
    OPCPart::setPath( zPath );

    //
    // update the descriptor's href
    //
    DWFResourceContainer::ResourceKVIterator* piResources = _pSection->getResourcesByRole();
    for ( ; piResources && piResources->valid(); piResources->next() )
    {
        DWFResource* pResource = piResources->value();
        if (pResource->role() == DWFXML::kzRole_Descriptor)
        {
            pResource->setRequestedName( /*NOXLATE*/L"descriptor.xml" );
            DWFString zTargetHRef( zPath );
            zTargetHRef.append( /*NOXLATE*/L"/" );
            zTargetHRef.append( pResource->requestedName() );
            pResource->setTargetHRef( zTargetHRef );
            break;
        }
    }
    DWFCORE_FREE_OBJECT( piResources );

}

_DWFTK_API
void
DWFXDWFSection::addResourcePart( DWFXResourcePart* pResourcePart,
                                 bool bOwn )
throw()
{
    if (pResourcePart)
    {
        _oResourceParts.push_back( pResourcePart );

        if (bOwn)
        {
            pResourcePart->own( *this );
        }
        else
        {
            pResourcePart->observe( *this );
        }

        addRelationship( pResourcePart, DWFXXML::kzRelationship_RequiredResource );

        const wchar_t* const zRelationship = DWFXXML::GetRelationship( pResourcePart->resource()->role() );
        if (zRelationship != NULL)
        {
            addRelationship( pResourcePart, zRelationship );
        }
    }
}

_DWFTK_API
bool
DWFXDWFSection::removeResourcePart( DWFXResourcePart* pResourcePart )
throw()
{
    if (_oResourceParts.erase( pResourcePart ))
    {
        //
        // delete the relationships to this part
        //
        deleteRelationshipsByTarget( pResourcePart );

        if (pResourcePart->owner() == this)
        {
            pResourcePart->disown( *this, true );
        }
        else
        {
            pResourcePart->unobserve( *this );
        }

        return true;
    }
    else
    {
        return false;
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXDWFSection::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    if (_pSection == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The section pointer cannot be null in the DWFPage." );
    }

    //
    //  Serialize the section descriptor
    //
    _pSection->serializeXML( rSerializer, DWFPackageWriter::eDescriptor );
}

#endif

_DWFTK_API
void
DWFXDWFSection::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    size_t n = 0;
    for (; n<_oResourceParts.size(); n++)
    {
        DWFOwnable* pOwnable = (DWFOwnable*)(_oResourceParts[n]);
        if (pOwnable == (&rOwnable))
        {
            _oResourceParts.eraseAt( n );
            return;
        }
    }
}

_DWFTK_API
void
DWFXDWFSection::_updatePageRels()
throw()
{
    //
    //  <TODO>  
    //  We need to run through all resources to see if there are DWFRelationships
    //  between resources that need to be tracked in the rels files in DWFX.
    //
}







