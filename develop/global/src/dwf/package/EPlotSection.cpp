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
#include "dwf/package/EPlotSection.h"
using namespace DWFToolkit;



_DWFTK_API
DWFEPlotSection::DWFEPlotSection( const DWFString&  zName,
                                  const DWFString&  zTitle,
                                  DWFPackageReader* pPackageReader )
throw()
               : DWFSection( _DWF_FORMAT_EPLOT_TYPE_STRING,
                              zName,
                              zTitle,
                              pPackageReader )
               , DWFEPlotSectionDescriptorReader( pPackageReader )
               , _pPaper( NULL )
               , _nColorARGB( 0 )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;
}

_DWFTK_API
DWFEPlotSection::DWFEPlotSection( const DWFString& zTitle,
                                  const DWFString& zObjectID,
                                  double           nPlotOrder,
                                  const DWFSource& rSource,
                                  unsigned int     nColorARGB,
                                  const DWFPaper*  pPaper )
throw()
               : DWFSection( _DWF_FORMAT_EPLOT_TYPE_STRING,
                              zTitle,
                              zObjectID,
                             _DWF_FORMAT_EPLOT_VERSION_CURRENT_FLOAT,
                              nPlotOrder,
                              rSource )
               , DWFEPlotSectionDescriptorReader()
               , _pPaper( NULL )
               , _nColorARGB( nColorARGB )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;

        //
        // copy paper
        //
    if (pPaper)
    {
        _pPaper = DWFCORE_ALLOC_OBJECT( DWFPaper(*pPaper) );
    }
}

_DWFTK_API
DWFEPlotSection::~DWFEPlotSection()
throw()
{
    if (_pPaper)
    {
        DWFCORE_FREE_OBJECT( _pPaper );
    }
}

_DWFTK_API
DWFInterface*
DWFEPlotSection::buildInterface()
throw( DWFException )
{
    DWFInterface* pInterface =
                DWFCORE_ALLOC_OBJECT( DWFInterface(DWFInterface::kzEPlot_Name,
                                                   DWFInterface::kzEPlot_HRef,
                                                   DWFInterface::kzEPlot_ID) );
    if (pInterface == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate interface" );
    }

    return pInterface;
}

_DWFTK_API
const char*
DWFEPlotSection::provideName( const char* zName )
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
DWFEPlotSection::provideObjectID( const char* zObjectID )
throw()
{
    _zObjectID.assign( zObjectID );
    
    return zObjectID;
}

_DWFTK_API
double
DWFEPlotSection::provideVersion( double nVersion )
throw()
{
    _nVersion = nVersion;
    
    return nVersion;
}

_DWFTK_API
double
DWFEPlotSection::providePlotOrder( double nPlotOrder )
throw()
{
    _nPlotOrder = nPlotOrder;
    
    return nPlotOrder;
}

_DWFTK_API
unsigned int
DWFEPlotSection::provideColor( unsigned int nColorARGB )
throw()
{
    _nColorARGB = nColorARGB;
    
    return nColorARGB;
}

_DWFTK_API
DWFPaper*
DWFEPlotSection::providePaper( DWFPaper* pPaper )
throw()
{
        //
        // copy paper
        //
    if (pPaper)
    {
        _pPaper = DWFCORE_ALLOC_OBJECT( DWFPaper(*pPaper) );

        DWFCORE_FREE_OBJECT( pPaper );
    }
    
    return NULL;
}

_DWFTK_API
DWFProperty*
DWFEPlotSection::provideProperty( DWFProperty* pProperty )
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
DWFEPlotSection::provideResource( DWFResource* pResource )
throw()
{
    addResource( pResource, true );
    
    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFEPlotSection::provideFontResource( DWFFontResource* pResource )
throw()
{
    addResource( pResource, true );
    
    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFEPlotSection::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    addResource( pResource, true );
    
    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFEPlotSection::provideImageResource( DWFImageResource* pResource )
throw()
{
    addResource( pResource, true );
    
    return pResource;
}

_DWFTK_API
DWFContentPresentationResource*
DWFEPlotSection::provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
const DWFResource&
DWFEPlotSection::readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader )
const
throw( DWFException )
{
    return DWFSection::readDescriptor( (pSectionDescriptorReader 
                                       ? pSectionDescriptorReader 
                                       : (DWFEPlotSectionDescriptorReader*)this) );
}

_DWFTK_API
void
DWFEPlotSection::setPaper( const DWFPaper* pPaper)
throw( DWFException )
{
        //
        // Delete if present.
        //
    if (_pPaper)
    {
        DWFCORE_FREE_OBJECT( _pPaper );
    }

        //
        // Copy new paper.
        //
    if (pPaper)
    {
        _pPaper = DWFCORE_ALLOC_OBJECT( DWFPaper(*pPaper) );
    }
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFEPlotSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        //
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
#define _DWFTK_EPLOT_TEMP_BUFFER_BYTES   32

        wchar_t     zTempBuffer[_DWFTK_EPLOT_TEMP_BUFFER_BYTES] = {0};
        DWFString   zTempString;

        //
        // make sure subsequent writes are in the correct context for a page
        //
        nFlags &= ~(DWFPackageWriter::eEModel | DWFPackageWriter::eData);
        nFlags |= DWFPackageWriter::eEPlot;

        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Page, namespaceXML(nFlags) );
        {
                //
                // attributes
                //
            {
                _DWFCORE_SWPRINTF( zTempBuffer, _DWFTK_EPLOT_TEMP_BUFFER_BYTES, /*NOXLATE*/L"%#0.2g", _nVersion );

                zTempString.assign( /*NOXLATE*/L"DWF-ePlot:" );
                zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

                rSerializer.addAttribute( /*NOXLATE*/L"ePlot", zTempString, /*NOXLATE*/L"xmlns:" );
                rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );
                rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );

                //
                // The <Page> element's 'name' is the "pretty name" aka title.
                //
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zTitle );
                rSerializer.addAttribute( DWFXML::kzAttribute_PlotOrder, _nPlotOrder );

                if (_nColorARGB != 0x00ffffff)
                {
                    _DWFCORE_SWPRINTF( zTempBuffer, 16, /*NOXLATE*/L"%d %d %d", (unsigned char)((_nColorARGB & 0x00ff0000)>>16),
                                                                     (unsigned char)((_nColorARGB & 0x0000ff00)>>8),
                                                                     (unsigned char)(_nColorARGB & 0x000000ff) );

                    rSerializer.addAttribute( DWFXML::kzAttribute_Color, zTempBuffer );
                }
            }

                //
                // paper
                //
            if (_pPaper)
            {
                _pPaper->serializeXML( rSerializer, nFlags );
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
DWFEPlotSection::Factory::Factory()
throw()
                        : DWFSection::Factory( _DWF_FORMAT_EPLOT_TYPE_STRING )
{
    ;
}

_DWFTK_API
DWFEPlotSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFEPlotSection::Factory::build( const DWFString&     zName,
                                 const DWFString&     zTitle,
                                 DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFEPlotSection* pSection = DWFCORE_ALLOC_OBJECT( DWFEPlotSection(zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}



