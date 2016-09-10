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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/Package.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwfcore/MIME.h"
#include "dwfcore/XMLParser.h"
using namespace DWFCore;

#include "dwf/opc/Package.h"
#include "dwf/opc/CoreProperties.h"
#include "dwf/opc/ContentTypes.h"
#include "dwf/opc/PhysicalLayerWriter.h"
#include "dwf/opc/PhysicalLayerReader.h"
#include "dwf/opc/RelationshipContainer.h"
#include "dwf/opc/Constants.h"
using namespace DWFToolkit;

//DNT_START

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const wchar_t* const OPCPackage::kzURI_PackageRels  = L"_rels/.rels";
const wchar_t* const OPCPackage::kzURI_PackageDigitalSignature        = L"/package/services/digital-signature/origin.psdsor";
const wchar_t* const OPCPackage::kzURI_PackageDigitalSignatureRels    = L"/package/services/digital-signature/_rels/origin.psdsor.rels";
const wchar_t* const OPCPackage::kzURI_PackageDigitalSignatureExt     = L".psdsxs";
const wchar_t* const OPCPackage::kzURI_PackageDigitalSignatureXPS     = L"http://schemas.microsoft.com/xps";


#endif

_DWFTK_API
OPCPackage::OPCPackage()
throw( DWFException )
          : OPCPartContainer()
          , _pRelsContainer( NULL )
          , _pCoreProperties( NULL )
          , _pContentTypes( NULL )
          , _pThumbnailPart( NULL )
{
    _pRelsContainer = DWFCORE_ALLOC_OBJECT( OPCRelationshipContainer );
    if (_pRelsContainer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for the package relationship container." );
    }

    _pCoreProperties = DWFCORE_ALLOC_OBJECT( OPCCoreProperties );
    if (_pCoreProperties == NULL)
    {
        DWFCORE_FREE_OBJECT( _pRelsContainer );
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for the package core properties." );
    }

    _pContentTypes = DWFCORE_ALLOC_OBJECT( OPCContentTypes );
    if (_pContentTypes == NULL)
    {
        DWFCORE_FREE_OBJECT( _pRelsContainer );
        DWFCORE_FREE_OBJECT( _pCoreProperties );
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for the package content types." );
    }

    addPart( _pContentTypes );
    addPart( _pCoreProperties );

    _pContentTypes->addOverride( _pCoreProperties->uri(), DWFMIME::kzMIMEType_COREPROPERTIES );
    //
    //  The package will have at least one .rels - that of the package
    //
    _pContentTypes->addContentType( DWFMIME::kzExtension_RELS, DWFMIME::kzMIMEType_RELS );

    //
    //  Since we have coreproperties, we need a relationship from the package to the core properties
    //
    addRelationship( _pCoreProperties, OPCXML::kzRelationship_CoreProperties );

    _pContentTypes->own( *this );
    _pCoreProperties->own( *this );
}

_DWFTK_API
OPCPackage::~OPCPackage()
throw()
{
    if (_pContentTypes->owner() == this)
    {
        notifyOwnableDeletion( *_pContentTypes );
        DWFCORE_FREE_OBJECT( _pContentTypes );
    }
    else
    {
        _pContentTypes->unobserve( *this );
    }

    if (_pCoreProperties->owner() == this)
    {
        notifyOwnableDeletion( *_pCoreProperties );
        DWFCORE_FREE_OBJECT( _pCoreProperties );
    }
    else
    {
        _pCoreProperties->unobserve( *this );
    }

    if (_pRelsContainer)
    {
        DWFCORE_FREE_OBJECT( _pRelsContainer );
    }
}

_DWFTK_API
void OPCPackage::setCoreProperties( OPCCoreProperties* pCorePropertiesPart, 
                                    bool bOwn )
throw( DWFException )
{
    if (pCorePropertiesPart == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, L"A valid core properties part must be provided" );
    }

    if (_pCoreProperties)
    {
        deleteRelationshipsByTarget( _pCoreProperties );

        //
        //  Transfer over any relationships
        //
        DWFPointer<OPCRelationship::tIterator> piRels( _pCoreProperties->relationships(), false );
        if (!piRels.isNull())
        {
            for (; piRels->valid(); piRels->next())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel)
                {
                    pCorePropertiesPart->addRelationship( pRel->target(), pRel->relationshipType(),
                                                          pRel->targetMode() );
                }
            }
        }

        if (_pCoreProperties->owner() == this)
        {
            notifyOwnableDeletion( *_pCoreProperties );
            DWFCORE_FREE_OBJECT( _pCoreProperties );
        }
        else
        {
            _pCoreProperties->unobserve( *this );
        }
    }

    _pCoreProperties = pCorePropertiesPart;
    addRelationship( _pCoreProperties, OPCXML::kzRelationship_CoreProperties );
    addPart( _pCoreProperties );

    if (bOwn)
    {
        _pCoreProperties->own( *this );
    }
    else
    {
        _pCoreProperties->observe( *this );
    }
}

_DWFTK_API
void OPCPackage::setThumbnail (OPCPart* pThumbnailPart)
    throw()
{
    if (_pThumbnailPart)
    {
        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( _pThumbnailPart );
    }

    _pThumbnailPart = pThumbnailPart;
    addRelationship( _pThumbnailPart, OPCXML::kzRelationship_Thumbnail );
}

_DWFTK_API
void OPCPackage::readRelationships (OPCPhysicalLayerReader* pReader)
throw( DWFException)
{
    if (pReader == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A valid physical layer reader must be provided" );
    }

    DWFPointer<DWFInputStream> apPackageRelsStream( pReader->read( kzURI_PackageRels ), false );
    DWFXMLParser packageRelsParser( _pRelsContainer );
    packageRelsParser.parseDocument( apPackageRelsStream );

}

_DWFTK_API
OPCRelationship*
OPCPackage::addRelationship( OPCPart* pTargetPart, 
                             const DWFString& zRelationshipType, 
                             OPCRelationship::teTargetMode eTargetMode )
throw( DWFException )
{
    return  _pRelsContainer->addRelationship( pTargetPart, zRelationshipType, eTargetMode );
}


_DWFTK_API
bool
OPCPackage::deleteRelationship( OPCRelationship* pRelationship )
throw()
{
    return _pRelsContainer->deleteRelationship( pRelationship );
}

_DWFTK_API
size_t
OPCPackage::relationshipCount() const
throw()
{
    return _pRelsContainer->relationshipCount();
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPackage::relationships()
throw()
{
    return  _pRelsContainer->relationships();
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPackage::relationshipsByTarget( const DWFString& zTargetURI ) const
throw()
{
    return  _pRelsContainer->relationshipsByTarget( zTargetURI );
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPackage::relationshipsByTarget( OPCPart* pPart ) const
throw()
{
    return  _pRelsContainer->relationshipsByTarget( pPart );
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPackage::relationshipsByType( const DWFString& zType ) const
throw()
{
    return  _pRelsContainer->relationshipsByType( zType );
}

_DWFTK_API
DWFInputStream* 
OPCPackage::getRelationshipsInputStream()
throw( DWFException )
{
    return _pRelsContainer->getInputStream();
}

_DWFTK_API
void
OPCPackage::deleteRelationshipsByTarget( OPCPart* pPart )
throw( DWFException )
{
    _pRelsContainer->deleteRelationshipsByTarget( pPart );
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
OPCPackage::serialize( OPCPhysicalLayerWriter& rWriter )
throw( DWFException )
{
    //
    //  Serialize the package rels first.
    //
    if (_pRelsContainer->relationshipCount() > 0)
    {
        DWFString zRelURI( kzURI_PackageRels );
        DWFPointer<DWFInputStream> apRelInpStream( getRelationshipsInputStream(), false );
        if (apRelInpStream.isNull())
        {
            _DWFCORE_THROW( DWFUnexpectedException, L"Error acquiring streams" );
        }
        rWriter.write( zRelURI, apRelInpStream );
    }

    OPCPart::tIterator* piParts = parts();
    if (piParts)
    {
        for (; piParts->valid(); piParts->next())
        {
            OPCPart* pPart = piParts->get();
            if (pPart)
            {
                DWFPointer<DWFInputStream> apPartStream( pPart->getInputStream(), false );
                if (apPartStream.isNull())
                {
                    _DWFCORE_THROW( DWFUnexpectedException, L"Error acquiring streams" );
                }

                rWriter.write( pPart->uri(), apPartStream );

                if (pPart->relationshipCount() > 0)
                {
                    DWFString zRelURI( pPart->relationshipUri() );
                    DWFPointer<DWFInputStream> apRelsInpStream( pPart->getRelationshipsInputStream(), false );
                    if (apRelsInpStream.isNull())
                    {
                        _DWFCORE_THROW( DWFUnexpectedException, L"Error acquiring streams" );
                    }
                    rWriter.write( zRelURI, apRelsInpStream );
                }
            }
        }
        DWFCORE_FREE_OBJECT( piParts );
    }
}

#endif


//DNT_END
