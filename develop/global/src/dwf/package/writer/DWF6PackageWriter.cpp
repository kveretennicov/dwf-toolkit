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


#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"
#include "dwfcore/Owner.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Constants.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/SignatureSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/utility/PropertyContainer.h"
#include "dwf/package/writer/DWF6PackageWriter.h"
#include "dwf/package/writer/extensions/6.0/PackageVersionExtension.h"
#include "dwf/package/writer/extensions/6.01/PackageVersionExtension.h"
#include "dwf/package/writer/extensions/6.11/PackageVersionExtension.h"
#include "dwf/package/writer/extensions/6.20/PackageVersionExtension.h"
#include "dwf/presentation/PackageContentPresentations.h"

#include "dwf/opc/Constants.h"
#include "dwf/dwfx/Constants.h"

using namespace DWFToolkit;




    //
    // used for temporary buffer to transfer bytes
    // from a resource to the dwf archive
    //
#define _DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES  16384

//DNT_Start

_DWFTK_API
DWF6PackageWriter::DWF6PackageWriter( const DWFFile&               rDWFPackageFile,
                                      const DWFString&             zDWFPackagePassword,
                                      DWF6PackageVersionExtension* pVersionExtension,
                                      bool                         bNoPasswordSalting )
throw()
                : DWFPackageWriter( rDWFPackageFile )
                , _zDWFPassword( zDWFPackagePassword )
                , _pPackageDescriptor( NULL )
                , _pXMLSerializer( NULL )
                , _pVersionExtension( pVersionExtension )
                , _pDWFProperties( NULL )
                , _pCustomProperties( NULL )
                , _oStreamsToDelete()
                , _oSignatureRequests()
                , _oManifestReferences()
                , _oItemSignatureReferenceMap()
{
    _bNoPasswordSalting = bNoPasswordSalting;
}

_DWFTK_API
DWF6PackageWriter::~DWF6PackageWriter()
throw()
{
    if (_pPackageDescriptor)
    {
        DWFCORE_FREE_OBJECT( _pPackageDescriptor );
    }

    if (_pVersionExtension)
    {
        DWFCORE_FREE_OBJECT( _pVersionExtension );
    }

    if (_pXMLSerializer)
    {
        DWFCORE_FREE_OBJECT( _pXMLSerializer );
    }

    _tSignatureRequestVector::iterator piReq = _oSignatureRequests.begin();
    for(; piReq != _oSignatureRequests.end(); piReq++)
    {
        DWFSignatureRequest *pReq = *piReq;
        DWFCORE_FREE_OBJECT( pReq );
    }
}

_DWFTK_API
void DWF6PackageWriter::addSection( DWFSection*    pSection,
                                    DWFInterface*  pInterface )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No section provided" );
    }

        //
        // always route through the version extension if it exists
        // and return immediately if instructed (ext. will return false)
        //
    if (_pVersionExtension && (_pVersionExtension->addSection(pSection, pInterface) == false))
    {
        return;
    }

    DWFPackageWriter::addSection( pSection, pInterface );
}

_DWFTK_API
void DWF6PackageWriter::addGlobalSection( DWFGlobalSection* pSection )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No section provided" );
    }

        //
        // always route through the version extension if it exists
        // and return immediately if instructed (ext. will return false)
        //
    if (_pVersionExtension && (_pVersionExtension->addGlobalSection(pSection) == false))
    {
        return;
    }

    DWFPackageWriter::addGlobalSection( pSection );
}

_DWFTK_API
void
DWF6PackageWriter::addSignatureRequest( DWFSignatureRequest* pSignatureRequest )
    throw( DWFException )
{
    if(pSignatureRequest == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Signature request must be specified" );
    }

    _oSignatureRequests.push_back(pSignatureRequest);
}

_DWFTK_API
void DWF6PackageWriter::setCoreProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    _ensureManifest();

    if (pSet->getSchemaID() != OPCXML::kzSchemaID_CoreProperties)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The PropertySet does not have the CoreProperties SchemaID" );
    }

    //
    //  This just filters the properties to only those that correspond to the OPC Core Properties
    //  specification. The properties get added to the manifest (property container) with category
    //  set to the OPC Core Properties schema ID.
    //
    unsigned int nFound = 0;
    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            DWFString& zName = piProperties->key();
            DWFProperty* pProperty = piProperties->value();

            if (!(nFound & 0x00001) && (zName == OPCXML::kzCoreProperty_Keywords))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0001;
            }

            else if (!(nFound & 0x0002) && (zName == OPCXML::kzCoreProperty_ContentType))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0002;
            }

            else if (!(nFound & 0x0004) && (zName == OPCXML::kzCoreProperty_Category))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0004;
            }

            else if (!(nFound & 0x0008) && (zName == OPCXML::kzCoreProperty_Version))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0008;
            }

            else if (!(nFound & 0x0010) && (zName == OPCXML::kzCoreProperty_Revision))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0010;
            }

            else if (!(nFound & 0x0020) && (zName == OPCXML::kzCoreProperty_LastModifiedBy))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0020;
            }

            else if (!(nFound & 0x0040) && (zName == OPCXML::kzCoreProperty_LastPrinted))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0040;
            }

            else if (!(nFound & 0x0080) && (zName == OPCXML::kzCoreProperty_ContentStatus))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0080;
            }

            else if (!(nFound & 0x0100) && (zName == OPCXML::kzCoreProperty_Creator))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0100;
            }

            else if (!(nFound & 0x0200) && (zName == OPCXML::kzCoreProperty_Identifier))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0200;
            }

            else if (!(nFound & 0x0400) && (zName == OPCXML::kzCoreProperty_Title))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0400;
            }

            else if (!(nFound & 0x0800) && (zName == OPCXML::kzCoreProperty_Subject))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x0800;
            }

            else if (!(nFound & 0x1000) && (zName == OPCXML::kzCoreProperty_Description))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x1000;
            }

            else if (!(nFound & 0x2000) && (zName == OPCXML::kzCoreProperty_Language))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x2000;
            }

            else if (!(nFound & 0x4000) && (zName == OPCXML::kzCoreProperty_Created))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x4000;
            }

            else if (!(nFound & 0x8000) && (zName == OPCXML::kzCoreProperty_Modified))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), OPCXML::kzSchemaID_CoreProperties );
                nFound |= 0x8000;
            }
        }
        DWFCORE_FREE_OBJECT( piProperties );
    }
}

_DWFTK_API
void DWF6PackageWriter::setDWFProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    _ensureManifest();

    if (pSet->getSchemaID() != DWFXXML::kzSchemaID_DWFProperties)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The PropertySet does not have the CoreProperties SchemaID" );
    }

    unsigned int nFound = 0;
    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            DWFString& zName = piProperties->key();
            DWFProperty* pProperty = piProperties->value();

            if (!(nFound & 0x00001) && (zName == DWFXML::kzDWFProperty_SourceProductVendor))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0001;
            }

            else if (!(nFound & 0x0002) && (zName == DWFXML::kzDWFProperty_SourceProductName))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0002;
            }

            else if (!(nFound & 0x0004) && (zName == DWFXML::kzDWFProperty_SourceProductVersion))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0004;
            }

            else if (!(nFound & 0x0008) && (zName == DWFXML::kzDWFProperty_DWFProductVendor))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0008;
            }

            else if (!(nFound & 0x0010) && (zName == DWFXML::kzDWFProperty_DWFProductVersion))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0010;
            }

            else if (!(nFound & 0x0020) && (zName == DWFXML::kzDWFProperty_DWFToolkitVersion))
            {
                _pPackageManifest->addProperty( zName, pProperty->value(), DWFXXML::kzSchemaID_DWFProperties );
                nFound |= 0x0020;
            }
        }
        DWFCORE_FREE_OBJECT( piProperties );
    }
}

_DWFTK_API
void DWF6PackageWriter::setCustomProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    _ensureManifest();

    //
    //  The documentation of this method in PackageWriter mentions that CustomProperties does
    //  not support nested property sets. This is for backwards compatability reasons.
    //
    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            _pPackageManifest->addProperty( piProperties->value(), false );
        }
        DWFCORE_FREE_OBJECT( piProperties );
    }
}

_DWFTK_API
void
DWF6PackageWriter::write( const DWFString&                   zSourceProductVendor,
                          const DWFString&                   zSourceProductName,
                          const DWFString&                   zSourceProductVersion,
                          const DWFString&                   zDWFProductVendor,
                          const DWFString&                   zDWFProductVersion,
                          DWFZipFileDescriptor::teFileMode   eCompressionMode )
throw( DWFException )
{
        //
        // if we are aggregating, do this before using or serializing the content
        // information.
        //
    if (_bAggregating)
    {
        _aggregateContent();
    }

        //
        // Try to determine a typeinfo type.
        //
        // if no ePlot sections have been added write out eModel typeinfo
        // else write out the ePlot typeinfo
        //
    DWFString zTypeInfo;
    if (_nEPlotSectionCount == 0)
    {
            //
            // No ePlot sections
            //
        if (_nEModelSectionCount != 0)
        {
                //
                // No ePlot and some eModel sections - tag as eModel.
                //
            zTypeInfo = DWFInterface::kzEModel_ID;
        }
        else
        {
                //
                // No ePlot and no emodel sections - do not create typeinfo.
                //
            zTypeInfo = /*NOXLATE*/L"";
        }
    }
    else
    {
            //
            // Some ePlot sections
            //
        zTypeInfo = DWFInterface::kzEPlot_ID;
    }

        //
        // if no version extension has been provided, hook in the appropriate
        // one based on the section type counts and by looking for content,
        // content presentations, or Raster Overlays.
        //
    if (_pVersionExtension == NULL)
    {
        _determinePackageVersionExtension( zTypeInfo );
    }

        //
        // always route through the version extension if it exists
        // and return immediately if instructed (ext. will return false)
        //
    if (_pVersionExtension && 
       (_pVersionExtension->write(zSourceProductVendor,
                                  zSourceProductName,
                                  zSourceProductVersion,
                                  zDWFProductVendor,
                                  zDWFProductVersion,
                                  eCompressionMode) == false))
    {
        return;
    }

    if (_pPackageManifest == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Cannot write package - no manifest exists" );
    }

    if (_pXMLSerializer == NULL)
    {
        _pXMLSerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer(_oUUID) );
        if (_pXMLSerializer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate serializer" );
        }
    }

        //
        // open the package
        //
    if (_pPackageDescriptor == NULL)
    {
        if (_pVersionExtension)
        {
            _pPackageDescriptor = DWFCORE_ALLOC_OBJECT( DWFPackageFileDescriptor(_rDWFPackage, eCompressionMode, _pVersionExtension->major(), _pVersionExtension->minor()) );
        }
        else
        {
            _pPackageDescriptor = DWFCORE_ALLOC_OBJECT( DWFPackageFileDescriptor(_rDWFPackage, eCompressionMode) );
        }

        if (_pPackageDescriptor == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file descriptor" );
        }

        _pPackageDescriptor->open();
    }

        //
        // if no version extension has been hooked in by this point then write out a typeinfo file
        // 
    if (_pVersionExtension == NULL)
    {
        if (zTypeInfo.chars())
        {
            zTypeInfo.append( /*NOXLATE*/L".TYPEINFO" );
        
            //
            // create the file
            //
            DWFOutputStream* pFilestream = _pPackageDescriptor->zip( zTypeInfo, _zDWFPassword, _bNoPasswordSalting );

            //
            // this file is empty, so just delete the stream (and close the file)
            //
            DWFCORE_FREE_OBJECT( pFilestream );
        }
    }


    //
    // Pre-process all SignatureRequest's.
    // - Deal with forward references.
    //
    DWFSection* pSignatureSection = NULL;

    if (!_oSignatureRequests.empty())
    {
        //
        //  Get the signature section for later use
        //
        pSignatureSection = _ensureSignatureSection();

        //
        //  Preprocess the requests to track the item-reference mappings.
        //
        _partitionSignatureReferences();

        //
        // Add a resource to the signatures section for each signature request.
        //
        // This is done so that the resources of the new signatures will be
        // listed in the section's descriptor.
        //
        _tSignatureRequestVector::iterator iRequest = _oSignatureRequests.begin();
        for( ; iRequest!=_oSignatureRequests.end(); iRequest++)
        {
            DWFSignatureRequest* pRequest = *iRequest;

            // Grab the "signature" resource from the request.
            DWFResource *pResource = pRequest->resource();
            if(pResource != NULL)
            {
                pSignatureSection->addResource(pResource, false); // Note: The request retains ownership of the resource.
            }
        }

    }

        //
        // give extension chance to write a file before the manifest
        //
    if (_pVersionExtension)
    {
       _pVersionExtension->prewriteManifest( *this, *_pPackageDescriptor, *_pXMLSerializer, _zDWFPassword );
    }
    
    //
    // just before writing out the manifest,
    // add any document-wide properties to it
    //
    _addPropertiesToManifest( zSourceProductVendor, zSourceProductName, zSourceProductVersion, 
                              zDWFProductVendor, zDWFProductVersion );

    //
    // create the MANIFEST stream
    //
    DWFOutputStream* pFilestream = _pPackageDescriptor->zip( /*NOXLATE*/L"manifest.xml", _zDWFPassword, _bNoPasswordSalting );

    //
    // For Digital Signatures. If there were any ManifestReferences given as part
    // of any SignatureRequests, then we're going to need to hook the Manifest's
    // outputStream to digest its bytes.
    //
    DWFOutputStream* pManifestSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, (void*)_pPackageManifest );

    //
    // bind the stream to the writer, this will directly
    // populate the archive as the manifest serializes itself
    //
    _pXMLSerializer->attach( (pManifestSignatureStreamChain) ? *pManifestSignatureStreamChain : *pFilestream );

    //
    // pass on the package content presentation to the manifest
    //
    _pPackageManifest->provideContentPresentations( _pPackageContentPresentations );
    //
    // tell the manifest to serialize himself as an XML document
    //
    _pPackageManifest->serializeXML( *_pXMLSerializer, eManifest );

    //
    // unbind the stream and finalize the file in the archive
    //
    _pXMLSerializer->detach();

    //
    // release the file stream
    //
    DWFCORE_FREE_OBJECT( pFilestream );

        //
        // give extension chance to write a file after the manifest
        //
    if (_pVersionExtension)
    {
       _pVersionExtension->postwriteManifest( *this, *_pPackageDescriptor, *_pXMLSerializer, _zDWFPassword );
    }

    //
    // Serialize all the contents in the content manager to the package.
    //
    DWFContentManager* pContentManager = _pPackageManifest->getContentManager();
    if (pContentManager)
    {
        DWFContent::tMap::Iterator* piContents = pContentManager->getContents();

        if (piContents)
        {
            DWFContent* pContent = NULL;
            for (; piContents->valid(); piContents->next())
            {
                pContent = piContents->value();

                if (pContent->isLoaded())
                {
                    //
                    //  Start the content file.
                    //
                    pFilestream = _pPackageDescriptor->zip( pContent->href(), _zDWFPassword, _bNoPasswordSalting );

                    //
                    // For Digital Signatures. If there were any ContentReferences referring to this content
                    // given as part of any SignatureRequests, then we're going to need to hook the content's
                    // outputStream to digest its bytes.
                    //
                    DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, pContent, pContent->href() );

                    //
                    // bind the stream to the writer, this will directly
                    // populate the archive as the page descriptor serializes itself
                    //
                    _pXMLSerializer->attach( pSignatureStreamChain ? *pSignatureStreamChain : *pFilestream );
                    
                    //
                    //  Tell the content to serialize itself.
                    //
                    pContent->serializeXML( *_pXMLSerializer, eGlobalContent );

                    //
                    //  Unbind the stream. This will also finalize the file in the archive.
                    //
                    _pXMLSerializer->detach();

                    //
                    //  Then release unbound stream.
                    //
                    DWFCORE_FREE_OBJECT( pFilestream );
                }
                else
                {
                    //
                    //  Since the content isn't loaded, instead of loading it, lets try to get the input stream
                    //  directly write out the bytes to new package
                    //
                    _serializeContent( pContent );
                }
            }

            DWFCORE_FREE_OBJECT( piContents );
        }
    }

    //
    // serialize package level presentations
    //
    if (_pPackageContentPresentations->presentationCount() > 0)
    {
        //
        //  Start the content file.
        //
        pFilestream = _pPackageDescriptor->zip( _pPackageContentPresentations->href(), _zDWFPassword, _bNoPasswordSalting );

        //
        // For Digital Signatures. If there were any ContentReferences referring to this content
        // given as part of any SignatureRequests, then we're going to need to hook the content's
        // outputStream to digest its bytes.
        //
        DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, _pPackageContentPresentations, _pPackageContentPresentations->href() );

        //
        // bind the stream to the writer, this will directly
        // populate the archive as the page descriptor serializes itself
        //
        _pXMLSerializer->attach( pSignatureStreamChain ? *pSignatureStreamChain : *pFilestream );
        
        //
        //  Tell the content to serialize itself.
        //
        _pPackageContentPresentations->serializeXML( *_pXMLSerializer, eContentPresentation );

        //
        //  Unbind the stream. This will also finalize the file in the archive.
        //
        _pXMLSerializer->detach();

        //
        //  Then release unbound stream.
        //
        DWFCORE_FREE_OBJECT( pFilestream );
    }

    DWFString zSection;
    DWFSection* pSection = NULL;


    DWFSection::tBehavior tSectionBehavior;

    //
    // Add any global sections first
    //
    DWFGlobalSection::tList::const_iterator iGlobalSection;

    for (iGlobalSection = _oGlobalSections.begin();
         iGlobalSection!= _oGlobalSections.end();
         iGlobalSection++)
    {
        pSection = *iGlobalSection;

        //
        // Grab the behavior from the section. In 7.1, this was on CustomSection.
        // In 7.2, we've promoted it to Section in general.
        //
        tSectionBehavior = pSection->behavior();

            //
            // manage section descriptor
            //
        if (tSectionBehavior.bPublishDescriptor)
        {
            //
            // name the new descriptor
            //
            zSection.assign( pSection->name() );
            zSection.append( /*NOXLATE*/L"\\descriptor.xml" );

            //
            // start with the descriptor
            //
            pFilestream = _pPackageDescriptor->zip( zSection, _zDWFPassword, _bNoPasswordSalting );

            //
            // For Digital Signatures. If there were any SectionDescriptorReferences to this section given as part
            // of any SignatureRequests, then we're going to need to hook the section descriptor's
            // outputStream to digest its bytes.
            //
            DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, pSection, zSection );

            //
            // bind the stream to the writer, this will directly
            // populate the archive as the page descriptor serializes itself
            //
            _pXMLSerializer->attach( pSignatureStreamChain ? *pSignatureStreamChain : *pFilestream );

            //
            // tell the page to serialize itself into the descriptor XML document
            //
            pSection->serializeXML( *_pXMLSerializer, eDescriptor );

            //
            // unbind the stream and finalize the file in the archive
            //
            _pXMLSerializer->detach();

            //
            // release the file stream
            //
            DWFCORE_FREE_OBJECT( pFilestream );
        }

        //
        // resources
        //
        DWFResource* pResource = NULL;
        DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

        if (piAllResources)
        {
            for (; piAllResources->valid(); piAllResources->next())
            {
                pResource = piAllResources->value();

                if (pResource->role() != DWFXML::kzRole_Descriptor)
                {
                    _serializeResource( pSection->name(), *pResource );
                }
            }

            DWFCORE_FREE_OBJECT( piAllResources );
        }
    }

        //
        // Add the non-global sections
        //
    DWFSection::tList::const_iterator iSection;
    for (iSection = _oSections.begin();
         iSection!= _oSections.end();
         iSection++)
    {
        pSection = *iSection;

        //
        // Grab the behavior from the section. In 7.1, this was on CustomSection.
        // In 7.2, we've promoted it to Section in general.
        //
        tSectionBehavior = pSection->behavior();

            //
            // manage section descriptor
            //
        if (tSectionBehavior.bPublishDescriptor)
        {
            //
            // name the new descriptor
            //
            zSection.assign( pSection->name() );
            zSection.append( /*NOXLATE*/L"\\descriptor.xml" );

            //
            // start with the descriptor
            //
            pFilestream = _pPackageDescriptor->zip( zSection, _zDWFPassword, _bNoPasswordSalting );

            //
            // For Digital Signatures. If there were any SectionDescriptorReferences to this section given as part
            // of any SignatureRequests, then we're going to need to hook the section descriptor's
            // outputStream to digest its bytes.
            //
            DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, pSection, zSection );

            //
            // bind the stream to the writer, this will directly
            // populate the archive as the page descriptor serializes itself
            //
            _pXMLSerializer->attach( pSignatureStreamChain ? *pSignatureStreamChain : *pFilestream );

            //
            // tell the page to serialize itself into the descriptor XML document
            //
            pSection->serializeXML( *_pXMLSerializer, eDescriptor );

            //
            // unbind the stream and finalize the file in the archive
            //
            _pXMLSerializer->detach();

            //
            // release the file stream
            //
            DWFCORE_FREE_OBJECT( pFilestream );
        }


        //
        // Remove any resources that were added, above, for signature
        // requests. It's too early to serialize these yet. They will
        // be handled seperately, below.
        //
        if(pSection == pSignatureSection)
        {
            _tSignatureRequestVector::iterator iRequest = _oSignatureRequests.begin();
            for( ; iRequest!=_oSignatureRequests.end(); iRequest++)
            {
                DWFSignatureRequest* pRequest = *iRequest;
                DWFResource *pResource = pRequest->resource();
                if(pResource != NULL)
                {
                    pSignatureSection->removeResource(*pResource, false);
                }
            }
        }

        //
        // iterator for finders
        //
        DWFIterator<DWFResource*>* piResources = NULL;

            //
            // thumbnails
            //

        piResources = pSection->findResourcesByRole( DWFXML::kzRole_Thumbnail );
        if (piResources)
        {
            for (; piResources->valid(); piResources->next())
            {
                _serializeResource( pSection->name(), *(piResources->get()) );
            }

            //
            // clean up
            //
            DWFCORE_FREE_OBJECT( piResources );
            piResources = NULL;
        }

            //
            // fonts
            //
        piResources = pSection->findResourcesByRole( DWFXML::kzRole_Font );
        if (piResources)
        {
            for (; piResources->valid(); piResources->next())
            {
                _serializeResource( pSection->name(), *(piResources->get()) );
            }

            //
            // clean up
            //
            DWFCORE_FREE_OBJECT( piResources );
            piResources = NULL;
        }

            //
            // w2d
            //
        piResources = pSection->findResourcesByRole( DWFXML::kzRole_Graphics2d );
        if (piResources)
        {
            for (; piResources->valid(); piResources->next())
            {
                _serializeResource( pSection->name(), *(piResources->get()) );
            }

            //
            // clean up
            //
            DWFCORE_FREE_OBJECT( piResources );
            piResources = NULL;
        }

            //
            // w3d
            //
        piResources = pSection->findResourcesByRole( DWFXML::kzRole_Graphics3d );
        if (piResources)
        {
            for (; piResources->valid(); piResources->next())
            {
                _serializeResource( pSection->name(), *(piResources->get()) );
            }

            //
            // clean up
            //
            DWFCORE_FREE_OBJECT( piResources );
            piResources = NULL;
        }

            //
            // textures
            //
        piResources = pSection->findResourcesByRole( DWFXML::kzRole_Texture );
        if (piResources)
        {
            for (; piResources->valid(); piResources->next())
            {
                _serializeResource( pSection->name(), *(piResources->get()) );
            }

            //
            // clean up
            //
            DWFCORE_FREE_OBJECT( piResources );
            piResources = NULL;
        }

        //
        // remaining resources
        //
        DWFResource* pResource = NULL;
        DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

        if (piAllResources)
        {
            for (; piAllResources->valid(); piAllResources->next())
            {
                pResource = piAllResources->value();

                    //
                    // if someone can think of a faster, cleaner way of doing this, please fix it
                    //
                if ((pResource->role() != DWFXML::kzRole_Descriptor)    &&
                    (pResource->role() != DWFXML::kzRole_Thumbnail)     &&
                    (pResource->role() != DWFXML::kzRole_Font)          &&
                    (pResource->role() != DWFXML::kzRole_Graphics2d)    &&
                    (pResource->role() != DWFXML::kzRole_Graphics3d)    &&
                    (pResource->role() != DWFXML::kzRole_Texture))
                {
                    _serializeResource( pSection->name(), *pResource );
                }
            }

            DWFCORE_FREE_OBJECT( piAllResources );
        }
    }

        //
        // give extension chance to write a file after the sections
        //
    if (_pVersionExtension)
    {
       _pVersionExtension->postwriteSections( *this, *_pPackageDescriptor, *_pXMLSerializer, _zDWFPassword );
    }

    _tSignatureRequestVector::iterator iRequest = _oSignatureRequests.begin();
    //
    // Generate Signatures
    //
    for (; iRequest!=_oSignatureRequests.end(); iRequest++)
    {
        DWFSignatureRequest* pRequest = *iRequest;

        //
        // Extract name from the resource that we used.
        //
        DWFResource *pResource = pRequest->resource();
        DWFString zSigName = pResource->publishedIdentity();

        //
        // Open the zip entry.
        //
        pFilestream = _pPackageDescriptor->zip( zSigName, _zDWFPassword, _bNoPasswordSalting );

        DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, pRequest, zSigName );

        //
        // bind the stream to the writer, this will directly
        // populate the archive as the page descriptor serializes itself
        //
        _pXMLSerializer->attach( pSignatureStreamChain ? *pSignatureStreamChain : *pFilestream );
       
        //
        // tell the SignatureRequest to serialize itself into the XML document
        //
        pRequest->serializeXML( *_pXMLSerializer, eSignatureRequest );

        //
        // unbind the stream and finalize the file in the archive
        //
        _pXMLSerializer->detach();

        //
        // release the file stream
        //
        DWFCORE_FREE_OBJECT( pFilestream );
    }

    //
    //  Add undeclared resources (those resources added to the package that
    //  are not declared in the manifest / descriptor hierarchy. )
    //
    if ( _pUndeclaredResourceProvider != NULL )
    {
        DWF6UndeclaredResourceReceiver oResourceReceiver( _pPackageDescriptor, _zDWFPassword, _bNoPasswordSalting );
        _pUndeclaredResourceProvider->provideResources( &oResourceReceiver );
    }

    //
    // Free all the digest streams that we created.
    //
    std::vector<DWFOutputStream*>::iterator iStream = _oStreamsToDelete.begin();
    for( ; iStream!=_oStreamsToDelete.end(); iStream++)
    {
        DWFOutputStream* pStream = (*iStream);
        DWFCORE_FREE_OBJECT(pStream);
    }
    _oStreamsToDelete.clear();

    //
    //  Free the mapped reference vectors
    //
    _tItemReferenceMap::iterator iItem = _oItemSignatureReferenceMap.begin();
    for (; iItem != _oItemSignatureReferenceMap.end(); ++iItem)
    {
        DWFCORE_FREE_OBJECT( iItem->second );
    }

    //
    // all done
    //
    _pPackageDescriptor->close();

        //
        // if we were aggregating, cleanup now
        //
    if (_bAggregating)
    {
        _postAggregationCleanup();
    }
    
}

void
DWF6PackageWriter::_serializeContent( DWFContent* pContent )
throw( DWFException )
{
    if (_pPackageDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No package descriptor exists" );
    }

    //
    //  Get the input stream from the content
    //
    DWFPointer<DWFInputStream>  apInputStream( pContent->getInputStream(), false );
    if (apInputStream.isNull())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Error acquiring streams" );
    }

    DWFOutputStream* pFilestream = NULL;
    try
    {
        //
        // Determine the HREF to use
        //
        DWFString zHREF = pContent->href();

        //
        //  Create the output stream
        //
        pFilestream = _pPackageDescriptor->zip( zHREF, _zDWFPassword, _bNoPasswordSalting );

        //
        // For Digital Signatures. If there were any ContentReferences referring to this content
        // given as part of any SignatureRequests, then we're going to need to hook the content's
        // outputStream to digest its bytes.
        //
        DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, pContent, zHREF );

        DWFOutputStream* pWriteToStream = (pSignatureStreamChain ? pSignatureStreamChain : pFilestream);

        //
        // stream the bytes from the resource into the archive
        //
        size_t  nBytesRead = 0;
        char    aBuffer[_DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES];
        while (apInputStream->available() > 0)
        {
            nBytesRead = apInputStream->read( aBuffer, _DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES );
            pWriteToStream->write( aBuffer, nBytesRead );
        }

        //
        // release the file stream
        //
        pWriteToStream->flush();

        DWFCORE_FREE_OBJECT( pFilestream );
    }
    catch(...)
    {
        if (pFilestream)
        {
            DWFCORE_FREE_OBJECT( pFilestream );
        }
        throw;
    }
}

void
DWF6PackageWriter::_serializeResource( const DWFString&  zSection,
                                      DWFResource&      rResource )
throw( DWFException )
{
    if (_pPackageDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No package descriptor exists" );
    }

    //
    // open the stream to acquire the resource bytes
    //
    DWFPointer<DWFInputStream>  apResourceStream( rResource.getInputStream(), false );
    if (apResourceStream.isNull())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Error acquiring streams" );
    }

    DWFOutputStream* pFilestream = NULL;
    try
    {
        //
        // open the resource file in the archive
        //
        DWFString zHREF = rResource.publishedIdentity();
        if (zHREF.bytes() == 0)
        {
            zHREF.assign( zSection );
            zHREF.append( /*NOXLATE*/L"\\" );
            zHREF.append( rResource.href() );
        }

        if (rResource.useCustomZipMode())
        {
            pFilestream = _pPackageDescriptor->zip( zHREF, rResource.customZipMode(), _zDWFPassword, _bNoPasswordSalting );
        }
        else if (DWFMIME::CompressByDefault( rResource.mime() ))
        {
            pFilestream = _pPackageDescriptor->zip( zHREF, _zDWFPassword, _bNoPasswordSalting );
        }
        else
        {
            pFilestream = _pPackageDescriptor->zip( zHREF, DWFZipFileDescriptor::eZipNone, _zDWFPassword, _bNoPasswordSalting );
        }

        DWFOutputStream* pSignatureStreamChain = _wrapOutputStreamForReferences( pFilestream, (void*)(&rResource) );

        DWFOutputStream* pWriteToStream = (pSignatureStreamChain ? pSignatureStreamChain : pFilestream);

            //
            // stream the bytes from the resource into the archive
            //
        size_t  nBytesRead = 0;
        char    aBuffer[_DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES];
        while (apResourceStream->available() > 0)
        {
            nBytesRead = apResourceStream->read( aBuffer, _DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES );
            pWriteToStream->write( aBuffer, nBytesRead );
        }

        //
        // release the file stream
        //
        pWriteToStream->flush();

        DWFCORE_FREE_OBJECT( pFilestream );
    }
    catch(...)
    {
        if (pFilestream)
        {
            DWFCORE_FREE_OBJECT( pFilestream );
        }
        throw;
    }
}

void
DWF6PackageWriter::_addPropertiesToManifest( const DWFString& zSourceProductVendor,
                                             const DWFString& zSourceProductName,
                                             const DWFString& zSourceProductVersion,
                                             const DWFString& zDWFProductVendor,
                                             const DWFString& zDWFProductVersion )
throw( DWFException )
{
    DWFProperty* pStandardProperty = NULL;

    if (zSourceProductVendor.bytes() > 0)
    {
        pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_SourceProductVendor, zSourceProductVendor) );
        if (pStandardProperty == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
        }

        _pPackageManifest->addProperty( pStandardProperty, true );
    }

    if (zSourceProductName.bytes() > 0)
    {
        pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_SourceProductName, zSourceProductName) );
        if (pStandardProperty == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
        }

        _pPackageManifest->addProperty( pStandardProperty, true );
    }

    if (zSourceProductVersion.bytes() > 0)
    {
        pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_SourceProductVersion, zSourceProductVersion) );
        if (pStandardProperty == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
        }

        _pPackageManifest->addProperty( pStandardProperty, true );
    }

    if (zDWFProductVendor.bytes() > 0)
    {
        pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_DWFProductVendor, zDWFProductVendor) );
        if (pStandardProperty == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
        }

        _pPackageManifest->addProperty( pStandardProperty, true );
    }

    if (zDWFProductVersion.bytes() > 0)
    {
        pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_DWFProductVersion, zDWFProductVersion) );
        if (pStandardProperty == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
        }
        _pPackageManifest->addProperty( pStandardProperty, true );
    }

    pStandardProperty = DWFCORE_ALLOC_OBJECT( DWFProperty(DWFXML::kzDWFProperty_DWFToolkitVersion, _DWFTK_VERSION_STRING) );
    if (pStandardProperty == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate property" );
    }
    _pPackageManifest->addProperty( pStandardProperty, true );
}

void
DWF6PackageWriter::_determinePackageVersionExtension( const DWFString& zTypeInfo )
throw( DWFException )
{
    if (_pPackageManifest == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Cannot write package - no manifest exists" );
    }

    //
    // Check for 3D markup of EModel.
    // These came onboard in 6.21.
    //
    // Check for Content Presentation, or Raster Overlay.
    // These came onboard in 6.20.
    //
    // This check is more complex, as we have to go through all the
    // sections, looking for resources of type kzRole_ContentPresentation,
    // kzRole_MarkupContentPresentation, kzRole_RasterOverlay, ...
    //
    // TODO: When we have global presentation data, we'll need to
    // detect it here as well.
    //
    bool bHavePresentation = false;
    bool bHaveRasterOverlay = false;
    bool bHaveEModelMarkup = false;
    DWFSection *pSection;
    DWFSection::tList::const_iterator iSection;
    for (iSection = _oSections.begin();
        iSection!= _oSections.end();
        iSection++)
    {
        pSection = *iSection;

        const DWFString &zSectionType = pSection->type();

        //
        // iterator for finders
        //
        DWFResourceContainer::ResourceKVIterator* piResourcesByRole = pSection->getResourcesByRole();
        if (piResourcesByRole && piResourcesByRole->valid())
        {
            for(; piResourcesByRole->valid(); piResourcesByRole->next())
            {
                const DWFString zRole = piResourcesByRole->key();

                // Note: Tests that affect the most current format must be first - because of the break.

                    //
                    // Check for emodel section with 3d markup resource.
                    //
                if(zSectionType == _DWF_FORMAT_EMODEL_TYPE_STRING)
                {
                    if (zRole == DWFXML::kzRole_3dMarkup)
                    {
                        bHaveEModelMarkup = true;
                        break;
                    }
                }

                if (zRole == DWFXML::kzRole_ContentPresentation ||
                    zRole == DWFXML::kzRole_MarkupContentPresentation)
                {
                    bHavePresentation = true;
                    break;
                }

                if(zRole == DWFXML::kzRole_RasterOverlay)
                {
                    bHaveRasterOverlay = true;
                    break;
                }

            }

        }

        DWFCORE_FREE_OBJECT( piResourcesByRole );

            //
            // If we've found one, that's all we need to know.
            //
        if( bHaveEModelMarkup )
        {
            break;
        }
    }

 
        //
        // If we've detected EModel Markup, then we're going to go with the present
        // "default" behavior. Else, we will try to write out an earlier version.
        //
    if(bHaveEModelMarkup == false)
    {
            //
            // Check for Content. This came onboard in 6.20.
            //
            // If the ContentManager has anything in it, that means we have content.
            //
        bool bHaveContent = false;
        DWFContentManager* pContentManager = _pPackageManifest->getContentManager();
        if (pContentManager)
        {
            bHaveContent = (pContentManager->getContentCount() > 0);
        }

            //
            // If we've detected content, presentation, or raster overlay, then we're
            // going to go with 6.20. Else, we will try for an earlier version.
            //
        if( bHaveContent || bHavePresentation || bHaveRasterOverlay )
        {
            _pVersionExtension = DWFCORE_ALLOC_OBJECT( DWFPackageVersion620Extension(zTypeInfo) );
        }
        else
        {
                //
                // if no eModel sections have been added, use the 6.0 extension
                //
            if (_nEModelSectionCount == 0)
            {
                _pVersionExtension = DWFCORE_ALLOC_OBJECT( DWFPackageVersion60Extension() );
            }

                //
                // if only one eModel section has been added, use the TypeInfo extension
                //
            else if (_nEPlotSectionCount == 0 && _nEModelSectionCount == 1)
            {    
                _pVersionExtension = DWFCORE_ALLOC_OBJECT( DWFPackageVersionTypeInfoExtension(DWFPackageVersionTypeInfoExtension::kzDocumentType_EModel) );
            }

                //
                // else, we should output 6.11 - which was the first true "eComposite" version,
                // and may or may not have typeinfo (based on the argument to the constructor).
                //
            else
            {    
                _pVersionExtension = DWFCORE_ALLOC_OBJECT( DWFPackageVersion611Extension(zTypeInfo) );
            }
        }
    }
}

DWFSection* 
DWF6PackageWriter::_ensureSignatureSection()
throw( DWFException )
{
    //
    // look for the Signatures section, and create it if it does not exist.
    //
    DWFSection* pSignatureSection = NULL;

    if(!_oSignatureRequests.empty())
    {
        DWFSection* pSection = NULL;
        DWFSection::tList::const_iterator iSection;
        for (iSection  = _oSections.begin();
             iSection != _oSections.end();
             iSection++)
        {
            pSection = *iSection;
            const DWFString& type = pSection->type();
            if(DWFCORE_COMPARE_WIDE_STRINGS(type, DWFInterface::kzSignatures_Name) == 0)
            {
                pSignatureSection = pSection;
                break;
            }
        }

            //
            // Not found, create one.
            //
        if (pSignatureSection == NULL)
        {
            DWFSignatureSection::Factory oFactory;

            pSignatureSection = oFactory.build(
                                    DWFInterface::kzSignatures_Name,    // Name
                                    /*NOXLATE*/ L"Signatures",          // Title
                                    NULL                                // Package reader
                                );

            addSection(pSignatureSection);
        }
    }

    return pSignatureSection;
}


////
////

void 
DWF6PackageWriter::_partitionSignatureReferences()
throw( DWFException )
{
    //
    //  Nothing to do if there are no signature requests
    //
    if (_oSignatureRequests.empty())
    {
        return;
    }

    //
    //  Iterate over the references across all requests. Create mappings 
    //  (or vectors if mappings aren't possible) from referenced items, e.g.
    //  resources, descriptors, content, etc. to the references that point to
    //  them.
    //
    _tSignatureRequestVector::iterator iRequest = _oSignatureRequests.begin();
    for (; iRequest != _oSignatureRequests.end(); ++iRequest)
    {
        DWFSignatureRequest* pRequest = *iRequest;

        DWFSignature::tReferenceVector::Iterator* piRef = pRequest->references();
        for ( ; piRef->valid(); piRef->next())
        {
            //
            // Is it a DWFSignatureRequest::Reference?
            //
            DWFSignatureRequest::Reference* pRef = dynamic_cast<DWFSignatureRequest::Reference*>(piRef->get());
            if (pRef == NULL)
            {
                continue;
            }

            //
            //  Is it a concrete reference
            //
            DWFSignatureRequest::ConcreteReference* pConcreteRef = dynamic_cast<DWFSignatureRequest::ConcreteReference*>(pRef);
            if (pConcreteRef != NULL)
            {
                const DWFResource* pResource = pConcreteRef->resource();
                _addToItemReferenceMap( (void*)pResource, pConcreteRef );
                continue;
            }

            //
            // Is it a Forward Reference? These come in many sub-flavors: Manifest, SignatureRequest, ...
            //
            DWFSignatureRequest::ForwardReference* pForwardRef = dynamic_cast<DWFSignatureRequest::ForwardReference*>(pRef);
            if(pForwardRef != NULL)
            {
                    //
                    // Is it a Manifest Reference?
                    //
                DWFSignatureRequest::ManifestReference* pManifestRef = dynamic_cast<DWFSignatureRequest::ManifestReference*>(pForwardRef);
                if(pManifestRef != NULL)
                {
                    _addToItemReferenceMap( (void*)_pPackageManifest, pManifestRef );
                    continue;
                }
               
                    //
                    // Is it a Descriptor Reference?
                    //
                DWFSignatureRequest::SectionDescriptorReference* pSectionDescRef = dynamic_cast<DWFSignatureRequest::SectionDescriptorReference*>(pForwardRef);
                if(pSectionDescRef != NULL)
                {
                    const DWFSection* pSection = pSectionDescRef->section();
                    _addToItemReferenceMap( (void*)pSection, pSectionDescRef);
                    continue;
                }

                    //
                    // Is it a SignatureRequest Reference?
                    //
                DWFSignatureRequest::SignatureRequestReference* pSigReqRef = dynamic_cast<DWFSignatureRequest::SignatureRequestReference*>(pForwardRef);
                if(pSigReqRef != NULL)
                {
                    const DWFSignatureRequest* pRequest = pSigReqRef->signatureRequest();
                    _addToItemReferenceMap( (void*)pRequest, pSigReqRef);
                    continue;
                }

                    //
                    // Is it a Content Reference?
                    //
                DWFSignatureRequest::ContentReference* pContentRef = dynamic_cast<DWFSignatureRequest::ContentReference*>(pForwardRef);
                if(pContentRef != NULL)
                {
                    const DWFContent* pContent = pContentRef->content();
                    _addToItemReferenceMap( (void*) pContent, pContentRef);
                    continue;
                }
            }
        }
        DWFCORE_FREE_OBJECT( piRef );
    }
}

void
DWF6PackageWriter::_addToItemReferenceMap( void* pItem, DWFSignatureRequest::Reference* pReference )
throw()
{
    _tItemReferenceMap::iterator iItem = _oItemSignatureReferenceMap.find( pItem );

    if (iItem != _oItemSignatureReferenceMap.end())
    {
        iItem->second->push_back( pReference );
    }
    else
    {
        DWFSignatureRequest::tReferenceVector* pRefVector = DWFCORE_ALLOC_OBJECT( DWFSignatureRequest::tReferenceVector(1, pReference) );
        _oItemSignatureReferenceMap.insert( std::make_pair(pItem, pRefVector) );
    }
}

DWFSignatureRequest::tReferenceVector*
DWF6PackageWriter::_signatureReferences( void* pItem, const DWFString& zURI )
throw()
{
    DWFSignatureRequest::tReferenceVector* pRefVector = NULL;

    _tItemReferenceMap::iterator iItem = _oItemSignatureReferenceMap.find( pItem );
    if (iItem != _oItemSignatureReferenceMap.end())
    {
        pRefVector = iItem->second;
    }

    if (pRefVector && zURI.chars()>0)
    {
        DWFSignatureRequest::tReferenceVector::Iterator* piRef = pRefVector->iterator();
        if (piRef)
        {
            for (; piRef->valid(); piRef->next())
            {
                DWFSignatureRequest::Reference* pRef = (DWFSignatureRequest::Reference*)(piRef->get());
                pRef->setURI( zURI );
            }
            DWFCORE_FREE_OBJECT( piRef );
        }
    }

    return pRefVector;
}

DWFOutputStream*
DWF6PackageWriter::_wrapOutputStreamForReferences( DWFOutputStream* pInitialOutputStream, 
                                                  void* pItem, 
                                                  const DWFString& zURI )
throw( DWFException )
{
    if (pInitialOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No initial output stream provided" );
    }

    //
    //  Get the signature references pointing to the provided item.
    //
    DWFSignatureRequest::tReferenceVector* pRefs = _signatureReferences( pItem, zURI );

    //
    //  Don't wrap if there aren't any references.
    //
    if (pRefs==NULL || pRefs->empty())
    {
        return NULL;
    }

    //
    // First partition the references by digest algorithm
    //
    std::vector<const DWFSignature::DigestMethod*> digestsSeen;
    std::multimap<const DWFSignature::DigestMethod*, DWFSignatureRequest::Reference*> refsByDigest;
    DWFSignature::tReferenceVector::Iterator* piRef = pRefs->iterator();
    for (; piRef->valid(); piRef->next())
    {
            //
            // Is it a DWFSignatureRequest::Reference?
            //
        DWFSignatureRequest::Reference* pRef = dynamic_cast<DWFSignatureRequest::Reference*>(piRef->get());
        if (pRef == NULL)
        {
            continue;
        }

        DWFSignatureRequest* pSignatureRequest = pRef->signatureRequest();

        const DWFSignatureRequest::DigestMethod* pDigestMethod = pSignatureRequest->digestMethod();

        digestsSeen.push_back(pDigestMethod);
        refsByDigest.insert(std::make_pair(pDigestMethod, pRef));

    }
    DWFCORE_FREE_OBJECT( piRef );

    bool didChaining = false;
    DWFOutputStream* mostRecentStream = pInitialOutputStream;

    //
    // Next, for each digest method that we saw:
    //  - create a digest stream and chain it in
    //  - inform all of the references that use this method about the stream.
    //
    std::vector<const DWFSignatureRequest::DigestMethod*>::iterator iDigest = digestsSeen.begin();
    for( ; iDigest!=digestsSeen.end(); iDigest++)
    {
        const DWFSignatureRequest::DigestMethod* digest = (*iDigest);

        //
        // Create the digest output stream.
        // Chain it.
        // Do not take ownership of the original stream!
        //
        DWFDigestOutputStream* newStream = digest->chainOutputStream(mostRecentStream, (mostRecentStream!=pInitialOutputStream));
        if(newStream != NULL)
        {
            // The new one is now the most recent one
            mostRecentStream = newStream;

            // Inform all of our references of the stream that will digest them.
            std::multimap<const DWFSignatureRequest::DigestMethod*, DWFSignatureRequest::Reference*>::const_iterator iRefs = refsByDigest.find(digest);
            if (iRefs != refsByDigest.end())
            {
                for (; (iRefs != refsByDigest.end()) && (iRefs->first == digest); ++iRefs)
                {
                    DWFSignatureRequest::Reference* pRef = iRefs->second;
                    pRef->setDigestOutputStream(newStream);
                }
            }


            // To let whoever called us know that we've done something.
            didChaining = true;
        }
    }


    if(didChaining)
    {
        // track streams for eventual deletion
        _oStreamsToDelete.push_back( mostRecentStream );

        return mostRecentStream;
    }
    else
    {
        return NULL;
    }
}

void 
DWF6PackageWriter::DWF6UndeclaredResourceReceiver::receiveResource( 
    const DWFCore::DWFString &rStoragePath, 
    DWFInputStream* pInStream, 
    DWFZipFileDescriptor::teFileMode eZipMode ) 
throw()
{
    DWFPointer<DWFOutputStream> spFileStream( _pPackageDescriptor->zip( rStoragePath, eZipMode, _zDWFPassword, _bPKZIPCompliantPassword ), false );

    if ( !spFileStream.isNull() && pInStream != NULL )
    {
        char buf[8192];
        bool done = false;
        while( !done )
        {
            //
            //  Spin through the input stream and write it to the output stream
            //
            size_t bytes = pInStream->read(buf, sizeof(buf));
            if (bytes != 0)
            {
                spFileStream->write( buf, bytes );
            }

            //
            //  Check to see if we're done
            //
            if ( bytes == 0 && pInStream->available() == 0)
            {
	            done = true;
            }
        }

        spFileStream->flush();
    }
}


//DNT_End

#endif

