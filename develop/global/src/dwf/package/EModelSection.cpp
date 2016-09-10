//
//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
#include "dwf/package/EModelSection.h"
using namespace DWFToolkit;


_DWFTK_API
DWFEModelSection::DWFEModelSection( const DWFString&  zName,
                                    const DWFString&  zTitle,
                                    DWFPackageReader* pPackageReader )
throw()
               : DWFSection( _DWF_FORMAT_EMODEL_TYPE_STRING,
                              zName,
                              zTitle,
                              pPackageReader )
               , DWFEModelSectionDescriptorReader( pPackageReader )
               , _pUnits( NULL )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;
}

_DWFTK_API
DWFEModelSection::DWFEModelSection( const DWFString& zTitle,
                                    const DWFString& zObjectID,
                                    double           nPlotOrder,
                                    const DWFSource& rSource,
                                    const DWFUnits*  pUnits )
throw()
               : DWFSection( _DWF_FORMAT_EMODEL_TYPE_STRING,
                              zTitle,
                              zObjectID,
                             _DWF_FORMAT_EMODEL_VERSION_CURRENT_FLOAT,
                              nPlotOrder,
                              rSource )
               , DWFEModelSectionDescriptorReader()
               , _pUnits( NULL )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;

        //
        // copy units
        //
    if (pUnits)
    {
        _pUnits = DWFCORE_ALLOC_OBJECT( DWFUnits(*pUnits) );
    }
}

_DWFTK_API
DWFEModelSection::~DWFEModelSection()
throw()
{
    if (_pUnits)
    {
        DWFCORE_FREE_OBJECT( _pUnits );
    }
}

_DWFTK_API
DWFInterface*
DWFEModelSection::buildInterface()
throw( DWFException )
{
    DWFInterface* pInterface =
                DWFCORE_ALLOC_OBJECT( DWFInterface(DWFInterface::kzEModel_Name,
                                                   DWFInterface::kzEModel_HRef,
                                                   DWFInterface::kzEModel_ID) );
    if (pInterface == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate interface" );
    }

    return pInterface;
}

_DWFTK_API
const char*
DWFEModelSection::provideName( const char* zName )
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
double
DWFEModelSection::provideVersion( double nVersion )
throw()
{
    _nVersion = nVersion;

    return nVersion;
}

_DWFTK_API
double
DWFEModelSection::providePlotOrder( double nPlotOrder )
throw()
{
    _nPlotOrder = nPlotOrder;

    return nPlotOrder;
}

_DWFTK_API
DWFUnits*
DWFEModelSection::provideUnits( DWFUnits* pUnits )
throw()
{
        //
        // copy units
        //
    if (pUnits)
    {
        _pUnits = DWFCORE_ALLOC_OBJECT( DWFUnits(*pUnits) );

        DWFCORE_FREE_OBJECT( pUnits );
    }

    return NULL;
}

_DWFTK_API
DWFProperty*
DWFEModelSection::provideProperty( DWFProperty* pProperty )
throw()
{
    addProperty( pProperty, true );

        //
        // set the section label
        //
    if (pProperty->name() == DWFSection::kzProperty_Label)
    {
        setLabel( pProperty->value() );
    }
        //
        // set the section label icon resource id
        //
    else if (pProperty->name() == DWFSection::kzProperty_LabelIconResourceID)
    {
        setLabelIconResourceID( pProperty->value() );
    }
        //
        // set the initial URI
        //
    else if (pProperty->name() == DWFSection::kzProperty_InitialURI)
    {
        setInitialURI( pProperty->value() );
    }

    return pProperty;
}

_DWFTK_API
DWFResource*
DWFEModelSection::provideResource( DWFResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFEModelSection::provideFontResource( DWFFontResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFEModelSection::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFEModelSection::provideImageResource( DWFImageResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFContentPresentationResource*
DWFEModelSection::provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
const DWFResource&
DWFEModelSection::readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader )
const
throw( DWFException )
{
    return DWFSection::readDescriptor( (pSectionDescriptorReader 
                                       ? pSectionDescriptorReader 
                                       : (DWFEModelSectionDescriptorReader*)this) );
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFEModelSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // 
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
#define _DWFTK_EMODEL_TEMP_BUFFER_BYTES   32

        wchar_t     zTempBuffer[_DWFTK_EMODEL_TEMP_BUFFER_BYTES] = {0};
        DWFString   zTempString;

        //
        // make sure subsequent writes are in the correct context for a space
        //
        nFlags &= ~(DWFPackageWriter::eEPlot | DWFPackageWriter::eData);
        nFlags |= DWFPackageWriter::eEModel;

        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Space, namespaceXML(nFlags) );
        {
                //
                // attributes
                //
            {
                _DWFCORE_SWPRINTF( zTempBuffer, _DWFTK_EMODEL_TEMP_BUFFER_BYTES, /*NOXLATE*/L"%#0.2g", _nVersion );

                zTempString.assign( /*NOXLATE*/L"DWF-eModel:" );
                zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

                rSerializer.addAttribute( /*NOXLATE*/L"eModel", zTempString, /*NOXLATE*/L"xmlns:" );
                rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );

                //
                // The <Space> element's 'name' is the "pretty name" aka title.
                //
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zTitle );
                rSerializer.addAttribute( DWFXML::kzAttribute_PlotOrder, _nPlotOrder );
            }

                //
                // units element
                //
            if (_pUnits)
            {
                _pUnits->serializeXML( rSerializer, nFlags );
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
DWFEModelSection::Factory::Factory()
throw()
                         : DWFSection::Factory( _DWF_FORMAT_EMODEL_TYPE_STRING )
{
    ;
}

_DWFTK_API
DWFEModelSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFEModelSection::Factory::build( const DWFString&     zName,
                                  const DWFString&     zTitle,
                                  DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFEModelSection* pSection = DWFCORE_ALLOC_OBJECT( DWFEModelSection(zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}





