//
//  Copyright (c) 1996-2005 by Autodesk, Inc.
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

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/package/SignatureSection.h"
using namespace DWFToolkit;



_DWFTK_API
DWFSignatureSection::DWFSignatureSection( const DWFString&  zName,
                                const DWFString&  zTitle,
                                DWFPackageReader* pPackageReader )
throw()
              : DWFSection( _DWF_FORMAT_SIGNATURE_SECTION_TYPE_STRING,
                             zName,
                             zTitle,
                             pPackageReader )
              , DWFSignatureSectionDescriptorReader( pPackageReader )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;

    _nVersion = _DWF_FORMAT_SIGNATURE_SECTION_VERSION_CURRENT_FLOAT;

    DWFSection::tBehavior tSectionBehavior = behavior();
    tSectionBehavior.bRenameOnPublish = false;
    tSectionBehavior.bPublishDescriptor = true;
    tSectionBehavior.bPublishResourcesToManifest = true; // Note that SignatureResource will not report itself to the manifest.
    applyBehavior(tSectionBehavior);

}

_DWFTK_API
DWFSignatureSection::DWFSignatureSection( const DWFString& zTitle,
                                const DWFString& zObjectID,
                                double           nPlotOrder,
                                const DWFSource& rSource )
throw()
              : DWFSection( _DWF_FORMAT_SIGNATURE_SECTION_TYPE_STRING,
                             zTitle,
                             zObjectID,
                            _DWF_FORMAT_SIGNATURE_SECTION_VERSION_CURRENT_FLOAT,
                             nPlotOrder,
                             rSource )
               , DWFSignatureSectionDescriptorReader()
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;

    DWFSection::tBehavior tSectionBehavior = behavior();
    tSectionBehavior.bRenameOnPublish = false;
    tSectionBehavior.bPublishDescriptor = true;
    tSectionBehavior.bPublishResourcesToManifest = true; // Note that SignatureResource will not report itself to the manifest.
    applyBehavior(tSectionBehavior);
}

_DWFTK_API
DWFSignatureSection::~DWFSignatureSection()
throw()
{
}

_DWFTK_API
DWFInterface*
DWFSignatureSection::buildInterface()
throw( DWFException )
{
    DWFInterface* pInterface =
                DWFCORE_ALLOC_OBJECT( DWFInterface(DWFInterface::kzSignatures_Name,
                                                   DWFInterface::kzSignatures_HRef,
                                                   DWFInterface::kzSignatures_ID) );
    if (pInterface == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate interface" );
    }

    return pInterface;
}

_DWFTK_API
const char*
DWFSignatureSection::provideName( const char* zName )
throw()
{
    //
    // the name attribute in the descriptor actually corresponds with
    // the title attribute in the manifest, there must be consistency
    // since the name is used as a map key in the DWFManifest object
    //
    _zTitle.assign( zName );
    
    return zName;
}

_DWFTK_API
const char*
DWFSignatureSection::provideObjectID( const char* zObjectID )
throw()
{
    _zObjectID.assign( zObjectID );
    
    return zObjectID;
}

_DWFTK_API
double
DWFSignatureSection::provideVersion( double nVersion )
throw()
{
    _nVersion = nVersion;
    
    return nVersion;
}

_DWFTK_API
DWFProperty*
DWFSignatureSection::provideProperty( DWFProperty* pProperty )
throw()
{
    addProperty( pProperty, true );
    
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFSignatureSection::provideResource( DWFResource* pResource )
throw()
{
    addResource( pResource, true );
    
    return pResource;
}

_DWFTK_API
DWFSignatureResource*
DWFSignatureSection::provideSignatureResource( DWFSignatureResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}


_DWFTK_API
const DWFResource&
DWFSignatureSection::readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader )
const
throw( DWFException )
{
    return DWFSection::readDescriptor( (pSectionDescriptorReader 
                                       ? pSectionDescriptorReader 
                                       : (DWFSignatureSectionDescriptorReader*)this) );
}

_DWFTK_API
DWFSignature*
DWFSignatureSection::getSignature( DWFResource* pResource, 
                                   DWFSignatureReader* pReaderFilter )
    throw( DWFException )
{
    // TODO: How to avoid re-load each time called?

    if (pResource && (pResource->role()==DWFXML::kzRole_Signature))
    {
        DWFSignature* pSignature = DWFCORE_ALLOC_OBJECT( DWFSignature() );

        if(pSignature != NULL)
        {
            DWFSignatureReader* pReader = dynamic_cast<DWFSignatureReader*>(pSignature);

            pReader->setResourceObjectID( pResource->objectID() );

            if (pReaderFilter)
            {
                pReader->setFilter( pReaderFilter );
            }

            //
            // acquire the data feed from the resource directly and don't cache
            // since hopefully the document is read only once and any useful data
            // is collected by the implementation of the reader
            //
            DWFPointer<DWFInputStream> apObjectStream( pResource->getInputStream(), false );

            _parseDocument( *(DWFInputStream*)apObjectStream, *pReader );

            pReader->setResourceObjectID( L"" );
        }

        return pSignature;
    }

    return NULL;
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFSignatureSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        //
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
#define _DWFTK_DATA_TEMP_BUFFER_BYTES   38

        wchar_t     zTempBuffer[_DWFTK_DATA_TEMP_BUFFER_BYTES] = {0};
        DWFString   zTempString;

        //
        // make sure subsequent writes are in the correct context for a Signatures section
        //
        nFlags &= ~(DWFPackageWriter::eEModel | DWFPackageWriter::eEPlot | DWFPackageWriter::eData);
        nFlags |= DWFPackageWriter::eSignatures;

        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Section, namespaceXML(nFlags) );
        {
                //
                // attributes
                //
            {
                _DWFCORE_SWPRINTF( zTempBuffer, _DWFTK_DATA_TEMP_BUFFER_BYTES, /*NOXLATE*/L"%#0.2g", _nVersion );

                zTempString.assign( /*NOXLATE*/L"DWF-Signatures:" );
                zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

                rSerializer.addAttribute( /*NOXLATE*/L"Signatures", zTempString, /*NOXLATE*/L"xmlns:" );
                rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );
                rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );

                //
                // The <Section> element's 'name' is the "pretty name" aka title.
                //
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zTitle );

                // We don't need plotOrder, do we?
                //_DWFCORE_SWPRINTF( zTempBuffer, _DWFTK_DATA_TEMP_BUFFER_BYTES, /*NOXLATE*/L"%.17g", _nPlotOrder );
                //rSerializer.addAttribute( DWFXML::kzAttribute_PlotOrder, zTempBuffer );

            }

            //
            // base implementation
            //
            DWFSection::serializeXML( rSerializer, nFlags );
        }
        rSerializer.endElement();
    }
        //
        // default implementation
        //
    else
    {
        DWFSection::serializeXML( rSerializer, nFlags );
    }
}

#endif

///
///
///

_DWFTK_API
DWFSignatureSection::Factory::Factory()
throw()
    : DWFSection::Factory( _DWF_FORMAT_SIGNATURE_SECTION_TYPE_STRING )
{
    ;
}

_DWFTK_API
DWFSignatureSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFSignatureSection::Factory::build( const DWFString&     zName,
                                     const DWFString&     zTitle,
                                     DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFSignatureSection* pSection = DWFCORE_ALLOC_OBJECT( DWFSignatureSection(zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}



