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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/DWFDocument.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/DWFDocument.h"
#include "dwf/package/writer/PackageWriter.h"
#include "dwf/package/Manifest.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/dwfx/DWFProperties.h"
#include "dwf/dwfx/CustomProperties.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFXDWFDocument::kzName = "manifest.xml";
//DNT_End

#endif


_DWFTK_API
DWFXDWFDocument::DWFXDWFDocument( DWFManifest* pManifest )
throw( DWFException )
               : _pManifest( pManifest )
               , _pDWFProperties( NULL )
               , _pCustomProperties( NULL )
               , _oContentParts()
               , _pPresentationsPart( NULL )
               , _oDWFSections()
{
    setName( DWFXDWFDocument::kzName );
}

_DWFTK_API
DWFXDWFDocument::~DWFXDWFDocument()
throw()
{
    DWFXContentPart::tIterator* piParts = _oContentParts.iterator();
    if (piParts)
    {
        for (; piParts->valid(); piParts->next())
        {
            DWFXContentPart* pPart = piParts->get();
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

    if (_pPresentationsPart)
    {
        if (_pPresentationsPart->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pPresentationsPart );
        }
        else
        {
            _pPresentationsPart->unobserve( *this );
        }
    }

    DWFXDWFSection::tIterator* piSections = _oDWFSections.iterator();
    if (piSections)
    {
        for (; piSections->valid(); piSections->next())
        {
            DWFXDWFSection* pSection = piSections->get();
            if (pSection->owner() == this)
            {
                DWFCORE_FREE_OBJECT( pSection );
            }
            else
            {
                pSection->unobserve( *this );
            }
        }
        DWFCORE_FREE_OBJECT( piSections );
    }

    if (_pDWFProperties)
    {
        if (_pDWFProperties->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pDWFProperties );
        }
        else
        {
            _pDWFProperties->unobserve( *this );
        }
    }

    if (_pCustomProperties)
    {
        if (_pCustomProperties->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pCustomProperties );
        }
        else
        {
            _pCustomProperties->unobserve( *this );
        }
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
void 
DWFXDWFDocument::addContentPart( DWFXContentPart* pContentPart,
                                 bool bOwn )
throw()
{
    if (pContentPart)
    {
        _oContentParts.push_back( pContentPart );

        if (bOwn)
        {
            pContentPart->own( *this );
        }
        else
        {
            pContentPart->observe( *this );
        }

        addRelationship( pContentPart, DWFXXML::kzRelationship_Content );
    }
}

_DWFTK_API
bool
DWFXDWFDocument::removeContentPart( DWFXContentPart* pContentPart )
throw()
{
    if (_oContentParts.erase( pContentPart ))
    {
        if (pContentPart->owner() == this)
        {
            pContentPart->disown( *this, true );
        }
        else
        {
            pContentPart->unobserve( *this );
        }

        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pContentPart );

        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
void 
DWFXDWFDocument::addDocumentPresentationsPart( DWFXDocumentPresentationsPart* pPresentationsPart,
                                              bool bOwn )
throw()
{
    _pPresentationsPart = pPresentationsPart;

    if (_pPresentationsPart)
    {
        if (bOwn)
        {
            _pPresentationsPart->own( *this );
        }
        else
        {
            _pPresentationsPart->observe( *this );
        }

        addRelationship( pPresentationsPart, DWFXXML::kzRelationship_DocumentPresentations );
    }
}

_DWFTK_API
bool
DWFXDWFDocument::removeDocumentPresentationsPart()
throw()
{
    if (_pPresentationsPart)
    {
        if (_pPresentationsPart->owner() == this)
        {
            _pPresentationsPart->disown( *this, true );
        }
        else
        {
            _pPresentationsPart->unobserve( *this );
        }

        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( _pPresentationsPart );

        _pPresentationsPart = NULL;

        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
void 
DWFXDWFDocument::addDWFSection( DWFXDWFSection* pSection,
                                bool bOwn )
throw()
{
    if (pSection)
    {
        _oDWFSections.push_back( pSection );

        if (bOwn)
        {
            pSection->own( *this );
        }
        else
        {
            pSection->observe( *this );
        }

        addRelationship( pSection, DWFXXML::kzRelationship_Section );
    }
}

_DWFTK_API
bool
DWFXDWFDocument::removeDWFSection( DWFXDWFSection* pSection )
throw()
{
    if (_oDWFSections.erase( pSection ))
    {
        if (pSection->owner() == this)
        {
            pSection->disown( *this, true );
        }
        else
        {
            pSection->unobserve( *this );
        }

        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pSection );

        return true;
    }
    else
    {
        return false;
    }
}


_DWFTK_API
void
DWFXDWFDocument::setDWFProperties( DWFXDWFProperties* pDWFProperties,
                                   bool bOwn )
throw( DWFException )
{
    if (pDWFProperties == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"A valid DWF properties part must be provided" );
    }

    if (_pDWFProperties)
    {
        //
        // delete the relationship to the existing part
        //
        deleteRelationshipsByTarget( _pDWFProperties );

        //
        //  Copy over any relationships from the old to the new part
        //
        DWFPointer<OPCRelationship::tIterator> piRels( _pDWFProperties->relationships(), false );
        if (!piRels.isNull())
        {
            for (; piRels->valid(); piRels->next())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel)
                {
                    pDWFProperties->addRelationship( pRel->target(), 
                                                     pRel->relationshipType(),
                                                     pRel->targetMode() );
                }
            }
        }

        if (_pDWFProperties->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pDWFProperties );
        }
        else
        {
            _pDWFProperties->unobserve( *this );
        }
    }

    _pDWFProperties = pDWFProperties;

    addRelationship( _pDWFProperties, DWFXXML::kzRelationship_DWFProperties );

    if (bOwn)
    {
        _pDWFProperties->own( *this );
    }
    else
    {
        _pDWFProperties->observe( *this );
    }
}

_DWFTK_API
void
DWFXDWFDocument::setCustomProperties( DWFXCustomProperties* pCustomProperties,
                                      bool bOwn )
throw( DWFException )
{
    if (pCustomProperties == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"A valid custom properties part must be provided" );
    }

    if (_pCustomProperties)
    {
        //
        // delete the relationship to the existing part
        //
        deleteRelationshipsByTarget( _pCustomProperties );

        //
        //  Copy over any relationships from the old to the new part
        //
        DWFPointer<OPCRelationship::tIterator> piRels( _pCustomProperties->relationships(), false );
        if (!piRels.isNull())
        {
            for (; piRels->valid(); piRels->next())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel)
                {
                    pCustomProperties->addRelationship( pRel->target(), 
                                                        pRel->relationshipType(),
                                                        pRel->targetMode() );
                }
            }
        }

        if (_pCustomProperties->owner() == this)
        {
            DWFCORE_FREE_OBJECT( _pCustomProperties );
        }
        else
        {
            _pCustomProperties->unobserve( *this );
        }
    }

    _pCustomProperties = pCustomProperties;

    addRelationship( _pCustomProperties, DWFXXML::kzRelationship_CustomProperties );

    if (bOwn)
    {
        _pCustomProperties->own( *this );
    }
    else
    {
        _pCustomProperties->observe( *this );
    }
}

_DWFTK_API
void
DWFXDWFDocument::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    DWFOwnable* pOwnable = (DWFOwnable*)(_pPresentationsPart);
    if (pOwnable == (&rOwnable))
    {
        _pPresentationsPart = NULL;
        return;
    }

    pOwnable = (DWFOwnable*)(_pDWFProperties);
    if (pOwnable == (&rOwnable))
    {
        _pDWFProperties = NULL;
        return;
    }

    size_t n = 0;
    for (; n<_oContentParts.size(); n++)
    {
        pOwnable = (DWFOwnable*)(_oContentParts[n]);
        if (pOwnable == (&rOwnable))
        {
            _oContentParts.eraseAt( n );
            return;
        }
    }

    n = 0;
    for (; n<_oDWFSections.size(); n++)
    {
        pOwnable = (DWFOwnable*)(_oDWFSections[n]);
        if (pOwnable == (&rOwnable))
        {
            _oDWFSections.eraseAt( n );
            return;
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXDWFDocument::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    if (_pManifest == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The manifest pointer cannot be null in the DWFDocument." );
    }

    _pManifest->serializeXML( rSerializer, DWFPackageWriter::eManifest );
}

#endif

