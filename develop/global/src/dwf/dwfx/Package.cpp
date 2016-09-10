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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/Package.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

//DNT_Start

#include <float.h>

#include "dwf/dwfx/Package.h"
#include "dwf/dwfx/DWFDocumentSequence.h"
#include "dwf/dwfx/DWFDocument.h"
#include "dwf/dwfx/DWFSection.h"
#include "dwf/dwfx/DWFProperties.h"
#include "dwf/dwfx/CustomProperties.h"
#include "dwf/dwfx/FixedDocumentSequence.h"
#include "dwf/dwfx/FixedDocument.h"
#include "dwf/dwfx/FixedPage.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/opc/ContentTypes.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Section.h"
#include "dwf/package/Resource.h"
using namespace DWFToolkit;

#include "dwfcore/MIME.h"
using namespace DWFCore;

_DWFTK_API
DWFXPackage::DWFXPackage()
throw( DWFException )
           : _pDWFDocumentSequence( NULL )
           , _nResourceInternalID( 0 )
{
    ;
}

_DWFTK_API
DWFXPackage::~DWFXPackage()
throw()
{
    if (_pDWFDocumentSequence != NULL)
    {
        if (_pDWFDocumentSequence->owner() == this)
        {
            notifyOwnableDeletion( *_pDWFDocumentSequence );
            DWFCORE_FREE_OBJECT( _pDWFDocumentSequence );
        }
        else
        {
            _pDWFDocumentSequence->unobserve( *this );
        }
    }
}

_DWFTK_API
DWFXDWFDocumentSequence* 
DWFXPackage::setDWFDocumentSequence( DWFXDWFDocumentSequence* pDWFDocumentSequence, 
                                     bool bOwn )
throw()
{
    DWFXDWFDocumentSequence* pOldDDS = _pDWFDocumentSequence;

    if (pOldDDS != NULL)
    {
        //
        // delete the relationship to this part
        //
        deleteRelationshipsByTarget( pOldDDS );

        if (pOldDDS->owner() == this)
        {
            pOldDDS->disown( *this, true );
        }
        else
        {
            pOldDDS->unobserve( *this );
        }
    }

    _pDWFDocumentSequence = pDWFDocumentSequence;

    if (_pDWFDocumentSequence != NULL)
    {
        if (bOwn)
        {
            _pDWFDocumentSequence->own( *this );
        }
        else
        {
            _pDWFDocumentSequence->observe( *this );
        }

        addRelationship( _pDWFDocumentSequence, DWFXXML::kzRelationship_DocumentSequence );
    }

    return pOldDDS;
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void 
DWFXPackage::serialize( OPCPhysicalLayerWriter& rWriter )
throw( DWFException )
{
    //
    //  Update the URIs of the different parts based on the container
    //  heirarchy. This will set the names of parts that don't have it set.
    //
    //  Upto this point all parts should have been added to their containers,
    //  but not to the package part container. This will go through the heirarchy
    //  and add the parts to the "part container" for the OPC based serialization.
    //
    _updateURIsAndOPCContainment();

    XPSPackage::serialize( rWriter );
}

void
DWFXPackage::_updateURIsAndOPCContainment()
throw( DWFException )
{
    DWFUUID uuidGenerator;

    //
    //  This updates the URIs first on the XPS side of the DWFX package,
    //  and then on the DWFX side.
    //

    //
    //  Update the URIs on the XPS side of things
    //
    DWFString zPkgRoot( L"/" );
    
    if (xpsFixedDocumentSequence() != NULL)
    {
        //
        // The name on the document sequences are already set. We need to set only the path.
        //
        xpsFixedDocumentSequence()->setPath( zPkgRoot );
        addPart( xpsFixedDocumentSequence() );
        contentTypes()->addContentType( DWFMIME::kzExtension_FDSEQ, DWFMIME::kzMIMEType_FDSEQ ); 

        //
        //  Update the fixed documents
        //
        XPSFixedDocument::tConstIterator* piDocs = xpsFixedDocumentSequence()->fixedDocuments();
        if (piDocs)
        {
            for (; piDocs->valid(); piDocs->next())
            {
                //
                //  We know that the XPSFixedDocuments are in fact DWFXFixedDocuments
                //
                DWFXFixedDocument* pDoc = (DWFXFixedDocument*)(piDocs->get());
                if (pDoc)
                {
                    DWFString zDocPath( zPkgRoot );

                    DWFManifest* pManifest = pDoc->manifest();
                    if (pManifest)
                    {
                        zDocPath.append( L"dwf/documents/" );
                        zDocPath.append( pManifest->objectID() );
                    }
                    else
                    {
                        _DWFCORE_THROW( DWFNullPointerException, L"The DWFXFixedDocument returned a null pointer for the manifest." );
                    }

                    pDoc->setPath( zDocPath );
                    addPart( pDoc );
                    contentTypes()->addContentType( DWFMIME::kzExtension_FDOC, DWFMIME::kzMIMEType_FDOC ); 

                    //
                    //  Update the fixed pages
                    //
                    DWFPointer<XPSFixedPage::tConstIterator> piPages( pDoc->fixedPages(), false );
                    if (!piPages.isNull())
                    {
                        for (; piPages->valid(); piPages->next())
                        {
                            DWFXFixedPage* pPage = (DWFXFixedPage*)(piPages->get());
                            if (pPage)
                            {
                                DWFString zPagePath( zDocPath );

                                DWFSection* pSection = pPage->section();
                                if (pSection)
                                {
                                    zPagePath.append( L"/sections/" );
                                    zPagePath.append( pSection->name() );
                                }

                                pPage->setPath( zPagePath );
                                addPart( pPage );
                                contentTypes()->addContentType( DWFMIME::kzExtension_FPAGE, DWFMIME::kzMIMEType_FPAGE ); 

                                //
                                //  Get the URI once since it is calculated at every call.
                                //
                                DWFString zPageURI( pPage->uri() );

                                //
                                //  Update internalIDs of the resources used by the fixed page
                                //
                                DWFPointer<DWFGraphicResource::tIterator> piResource( pPage->graphicResources(), false );
                                if (!piResource.isNull())
                                {
                                    for (; piResource->valid(); piResource->next())
                                    {
                                        DWFGraphicResource* pResource = piResource->get();
                                        _updateInternalID( pResource );

                                        //
                                        //  Set the resource HREF so that from the manifest or section descriptor,
                                        //  we can locate the chunk of XML in the fixed page that corresponds to
                                        //  the resource: HREF = FixedPageURI?ResourceInternalID.
                                        //
                                        DWFString zTargetHREF( zPageURI );
                                        zTargetHREF.append( /*NOXLATE*/L"?" );
                                        zTargetHREF.append( pResource->internalID() );
                                        pResource->setRequestedName( zTargetHREF );
                                        pResource->setTargetHRef( zTargetHREF );
                                    }
                                }
                            }
                        }
                    }
                }
            }
            DWFCORE_FREE_OBJECT( piDocs );
        }
    }

    //
    //  Update the URIs on the DWF side of things
    //
    if (dwfDocumentSequence() != NULL)
    {
        //
        // The name on the document sequences are already set. We need to set only the path.
        //
        dwfDocumentSequence()->setPath( zPkgRoot );
        addPart( dwfDocumentSequence() );
        contentTypes()->addContentType( DWFMIME::kzExtension_DWFSEQ, DWFMIME::kzMIMEType_DWFSEQ );

        //
        //  Update the DWF documents
        //
        DWFXDWFDocument::tConstIterator* piDocs = dwfDocumentSequence()->dwfDocuments();
        if (piDocs)
        {
            for (; piDocs->valid(); piDocs->next())
            {
                DWFXDWFDocument* pDoc = piDocs->get();
                if (pDoc)
                {
                    DWFString zDocPath( zPkgRoot );

                    DWFManifest* pManifest = pDoc->manifest();
                    if (pManifest)
                    {
                        zDocPath.append( L"dwf/documents/" );
                        zDocPath.append( pManifest->objectID() );
                    }
                    else
                    {
                        _DWFCORE_THROW( DWFNullPointerException, L"The DWFXDWFdocument returned a null pointer for the manifest." );
                    }

                    //
                    //  Add the document itself to the part container
                    //
                    pDoc->setPath( zDocPath );
                    addPart( pDoc );
                    contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );

                    //
                    //  Add the document level DWF properties
                    //
                    DWFXDWFProperties* pDWFProperties = pDoc->dwfProperties();
                    if (pDWFProperties)
                    {
                        DWFString zPropPartName = uuidGenerator.next(false);
                        zPropPartName.append( L"." );
                        zPropPartName.append( DWFXDWFProperties::kzName );
                        
                        pDWFProperties->setName( zPropPartName );
                        pDWFProperties->setPath( zDocPath );
                        addPart( pDWFProperties );
                        contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );
                    }

                    //
                    //  Add the document level custom published properties
                    //
                    DWFXCustomProperties* pCustomProperties = pDoc->customProperties();
                    if (pCustomProperties)
                    {
                        DWFString zPropPartName = uuidGenerator.next(false);
                        zPropPartName.append( L"." );
                        zPropPartName.append( DWFXCustomProperties::kzName );
                        
                        pCustomProperties->setName( zPropPartName );
                        pCustomProperties->setPath( zDocPath );
                        addPart( pCustomProperties );
                        contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );
                    }

                    //
                    //  Update the DWFX ContentParts
                    //
                    DWFXContentPart::tConstIterator* piContentParts = pDoc->contentParts();
                    if (piContentParts)
                    {
                        for (; piContentParts->valid(); piContentParts->next())
                        {
                            DWFXContentPart* pPart = piContentParts->get();
                            if (pPart)
                            {
                                pPart->setPath( zDocPath );
                                addPart( pPart );
                                contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );

                                DWFContent* pContent = pPart->content();
                                if (pContent == NULL)
                                {
                                    _DWFCORE_THROW( DWFNullPointerException, L"A content part was created with no reference to any content." );
                                }
                                //
                                //  The href is it's ID and suffix, and is used for the URI.
                                //
                                pPart->setName( pContent->href() );

                                //
                                // update the href with the new path
                                //
                                pContent->setHRef( pPart->uri() );
                            }
                        }
                        DWFCORE_FREE_OBJECT( piContentParts );
                    }

                    //
                    //  Update the DWFX DocumentPresentationPart
                    //
                    DWFXDocumentPresentationsPart* pDocPresentationsPart = pDoc->documentPresentationsPart();
                    if (pDocPresentationsPart)
                    {
                        pDocPresentationsPart->setPath( zDocPath );
                        addPart( pDocPresentationsPart );
                        contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );

                        DWFPackageContentPresentations* pPresentations = pDocPresentationsPart->presentations();
                        if (pPresentations == NULL)
                        {
                            _DWFCORE_THROW( DWFNullPointerException, L"A document presentations part was created with no reference to any presentations." );
                        }
                        //
                        //  The href is it's ID and suffix, and is used for the URI.
                        //
                        pDocPresentationsPart->setName( pPresentations->href() );
                        //
                        // update the href with the new path
                        //
                        pPresentations->setHRef( pDocPresentationsPart->uri() );
                    }

                    //
                    // keep track of the earliest section (by order) containing thumbnail
                    //
                    double dOrder = DBL_MAX;
                    DWFSection* pEarliestSectionContainingThumbnail = NULL;
                    OPCPart* pPackageThumbnailPart = NULL;

                    //
                    //  Update the DWFX DWFSections
                    //
                    DWFXDWFSection::tConstIterator* piSections = pDoc->dwfSections();
                    if (piSections)
                    {
                        for (; piSections->valid(); piSections->next())
                        {
                            DWFXDWFSection* pDWFSectionPart = piSections->get();
                            if (pDWFSectionPart)
                            {
                                DWFString zSectionPath( zDocPath );

                                DWFSection* pSection = pDWFSectionPart->section();
                                if (pSection)
                                {
                                    zSectionPath.append( L"/sections/" );
                                    zSectionPath.append( pSection->name() );
                                }
                                else
                                {
                                    _DWFCORE_THROW( DWFNullPointerException, L"The DWFXDWFSection returned a null pointer for the section." );
                                }

                                pDWFSectionPart->setPath( zSectionPath );
                                addPart( pDWFSectionPart );
                                contentTypes()->addContentType( DWFMIME::kzExtension_XML, DWFMIME::kzMIMEType_XML );

                                //
                                //  Update the DWFXResourceParts owned by the section
                                //
                                DWFXResourcePart::tConstIterator* piResourceParts = pDWFSectionPart->resourceParts();
                                if (piResourceParts)
                                {
                                    for (; piResourceParts->valid(); piResourceParts->next())
                                    {
                                        DWFXResourcePart* pPart = piResourceParts->get();
                                        if (pPart)
                                        {
                                            _updateResourcePartInfo( pPart, zSectionPath, uuidGenerator );
                                        }

                                        //
                                        // if we have a thumbnail or preview resource
                                        //
                                        DWFResource* pResource = pPart->resource();
                                        if ((pResource->role() == DWFXML::kzRole_Preview ||
                                            pResource->role() == DWFXML::kzRole_Thumbnail) &&
											(pResource->mime() == DWFMIME::kzMIMEType_JPG ||
											pResource->mime() == DWFMIME::kzMIMEType_PNG))
                                        {
                                            //
                                            // if this section's order is less than what we have so far
                                            // then use this resource's part as the package thumbnail part
                                            //
                                            if (pSection->order() < dOrder)
                                            {
                                                pEarliestSectionContainingThumbnail = pSection;
                                                dOrder = pSection->order();
                                                pPackageThumbnailPart = pPart;
                                            }
                                        }
                                    }
                                    DWFCORE_FREE_OBJECT( piResourceParts );
                                }
                            }
                        }
                        DWFCORE_FREE_OBJECT( piSections );
                    }

                    if (pPackageThumbnailPart)
                    {
                        setThumbnail( pPackageThumbnailPart );
                    }
                }
            }
            DWFCORE_FREE_OBJECT( piDocs );
        }
    }

}

void 
DWFXPackage::_updateResourcePartInfo( DWFXResourcePart* pPart,
                                      const DWFString& zPath,
                                      DWFUUID& uuidGenerator )
throw( DWFException )
{
    DWFResource* pResource = pPart->resource();
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, L"The DWFXResourcePart returned a null pointer for the resource." );
    }

    pPart->setPath( zPath );
    addPart( pPart );

    DWFString zReqName( pResource->requestedName() );
    DWFString zExtension;
    
    if (zReqName.chars() == 0)
    {
        zExtension = DWFMIME::GetExtension( pResource->mime() );

        const DWFString& zHRef = pResource->href();
        if (zHRef)
        {
            off_t nOffset = zHRef.find( L'/', 0, true );
            if (nOffset != -1)
            {
                zReqName = zHRef.substring( (size_t)nOffset + 1, (size_t)-1 );
            }
        }

        if (zReqName.chars() == 0)
        {
            zReqName.assign( uuidGenerator.next(false) );

            if (zExtension.bytes())
            {
                zReqName.append( L"." );
                zReqName.append( zExtension );
            }
        }


        //
        //  Setting it here incase it is required elsewhere after this point.
        //
        pResource->setRequestedName( zReqName );
    }
    else
    {
        off_t nOffset = zReqName.find( L'.', 0, true );
        if (nOffset != -1)
        {
            zExtension = zReqName.substring( nOffset+1, (size_t)-1 );
        }

		//
		//to get extension
		//
		else
		{
			zExtension = DWFMIME::GetExtension( pResource->mime() );
		}
    }

    pPart->setName( zReqName );

    //
    //  The object ID is set normally during the the identity setup - hence we
    //  do this check here.
    //  Unfortunately we create the Object ID separately since the object ID 
    //  has always been squashed. This keeps it consistent with the old format.
    //  The resource name (above) cannot be squashed because XPS does not 
    //  differentiate between upper and lower cases.
    //
    DWFString zObjectID( pResource->objectID() );
    if (zObjectID.chars() == 0)
    {
        pResource->setObjectID( uuidGenerator.next(true) );
    }

    //
    //  Set the HREF on the resource since this will be used in the serialization 
    //  of the manifest (DWFXDocument) and the section description.
    //
    DWFString zResourceHREF( pPart->path() );
    zResourceHREF.append( /*NOXLATE*/L"/" );
    zResourceHREF.append( zReqName );

	//
	//or (pResource->publishedIdentity()).chars(); 
	//integrate all character blocks to be one.
	//any function can make _zTargetHRef contains only one or no character block.
	//
	(pResource->publishedIdentity()).bytes();
    pResource->setTargetHRef( zResourceHREF );

    
    //
    //  Set the internal ID on resources. This is used to identify them in fixed pages
    //
    _updateInternalID( pResource );

    if (zExtension.bytes())
    {
        if (!contentTypes()->addContentType( zExtension, pResource->mime() ))
        {
            contentTypes()->addOverride( pPart->uri(), pResource->mime() );
        }
    }
    else
    {
        contentTypes()->addOverride( pPart->uri(), pResource->mime() );
    }
}

void
DWFXPackage::_updateInternalID( DWFResource* pResource )
throw( DWFException )
{
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, L"The resource pointer was NULL." );
    }

    _nResourceInternalID++;

    size_t nBufferSize = 16;
    DWFPointer<wchar_t> zIDValue( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
    int nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zIDValue, nBufferSize, L"%d", _nResourceInternalID );

    DWFString zID( L"dwfresource_" );
    zID.append( zIDValue, nBytes );
    pResource->setInternalID( zID );
}

#endif

//DNT_End
