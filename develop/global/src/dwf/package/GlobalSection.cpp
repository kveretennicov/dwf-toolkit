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
#include "dwf/package/GlobalSection.h"
using namespace DWFToolkit;



_DWFTK_API
DWFGlobalSection::DWFGlobalSection( const DWFString&  zType,
                                    const DWFString&  zName,
                                    const DWFString&  zTitle,
                                    DWFPackageReader* pPackageReader )
throw()
                : DWFSection( zType,
                              zName,
                              zTitle,
                              pPackageReader )
                , DWFGlobalSectionDescriptorReader( pPackageReader )
                , _pRootBookmark( NULL )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;
}

_DWFTK_API
DWFGlobalSection::DWFGlobalSection( const DWFString& zType,
                                    const DWFString& zName,
                                    const DWFString& zTitle,
                                    double           nVersion,
                                    const DWFSource& rSource )
throw()
                : DWFSection( zType,
                              zTitle,
                              /*NOXLATE*/L"",
                              nVersion,
                             -1,
                              rSource )
                , DWFGlobalSectionDescriptorReader()
                , _pRootBookmark( NULL )
{
    //
    // use the element builder from the DWFSection base
    // that knows about the DWFResourceContainer
    //
    _pElementBuilder = this;

    _zName.assign( zName );
}

_DWFTK_API
DWFGlobalSection::~DWFGlobalSection()
throw()
{
    if (_pRootBookmark)
    {
        DWFCORE_FREE_OBJECT( _pRootBookmark );
    }

    //
    // requirement of the DWFOwnable contract
    //
    DWFOwnable::_notifyDelete();

}

_DWFTK_API
const DWFResource&
DWFGlobalSection::readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader )
const
throw( DWFException )
{
    return DWFSection::readDescriptor( (pSectionDescriptorReader 
                                       ? pSectionDescriptorReader 
                                       : (DWFGlobalSectionDescriptorReader*)this) );
}

_DWFTK_API
const char*
DWFGlobalSection::provideName( const char* zName )
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
DWFGlobalSection::provideVersion( double nVersion )
throw()
{
    _nVersion = nVersion;

    return nVersion;
}

_DWFTK_API
DWFProperty*
DWFGlobalSection::provideProperty( DWFProperty* pProperty )
throw()
{
    addProperty( pProperty, true );

    return pProperty;
}

_DWFTK_API
DWFResource*
DWFGlobalSection::provideResource( DWFResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFGlobalSection::provideFontResource( DWFFontResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFGlobalSection::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFGlobalSection::provideImageResource( DWFImageResource* pResource )
throw()
{
    addResource( pResource, true );

    return pResource;
}

_DWFTK_API
DWFBookmark*
DWFGlobalSection::provideBookmark( DWFBookmark* pBookmark )
throw()
{
    _pRootBookmark = pBookmark;

    return _pRootBookmark;
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFGlobalSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    //
    // write flags
    //
    nFlags |= DWFPackageWriter::eGlobal;

        //
        // 
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        //
        // base implementation
        //
        DWFSection::serializeXML( rSerializer, nFlags );

        //
        // object definition
        //
        //DWFDefinedObjectContainer::getSerializableObjectContainer().serializeXML( rSerializer, nFlags );

            //
            // add any bookmarks
            //
        if (_pRootBookmark)
        {
            _pRootBookmark->serializeXML( rSerializer, nFlags );
        }
    }
        //
        // base class for manifest
        // 
    else
    {
        DWFSection::serializeXML( rSerializer, nFlags );
    }
}

///
///
///

_DWFTK_API
void
DWFEPlotGlobalSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // 
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        wchar_t     zTempBuffer[16] = {0};
        DWFString   zTempString;

        //
        // make sure subsequent writes are in the correct context
        //
        nFlags &= ~(DWFPackageWriter::eEModel | DWFPackageWriter::eData);
        nFlags |= DWFPackageWriter::eEPlot;

        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Global, namespaceXML(nFlags) );
        {
                //
                // attributes
                //
            {
                _DWFCORE_SWPRINTF( zTempBuffer, 16, /*NOXLATE*/L"%#0.2g", _nVersion );

                zTempString.assign( /*NOXLATE*/L"DWF-ePlot:" );
                zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

                rSerializer.addAttribute( /*NOXLATE*/L"ePlot", zTempString, /*NOXLATE*/L"xmlns:" );
                rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );
                rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zName );
            }

            //
            // base global section for the rest
            //
            DWFGlobalSection::serializeXML( rSerializer, nFlags );
        }
        rSerializer.endElement();
    }
        //
        // base class for manifest
        // 
    else
    {
        DWFGlobalSection::serializeXML( rSerializer, nFlags );
    }
}


_DWFTK_API
void
DWFEModelGlobalSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // 
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        wchar_t     zTempBuffer[16] = {0};
        DWFString   zTempString;

        //
        // make sure subsequent writes are in the correct context
        //
        nFlags &= ~(DWFPackageWriter::eEPlot | DWFPackageWriter::eData);
        nFlags |= DWFPackageWriter::eEModel;

        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Global, namespaceXML(nFlags) );
        {
                //
                // attributes
                //
            {
                _DWFCORE_SWPRINTF( zTempBuffer, 16, /*NOXLATE*/L"%#0.2g", _nVersion );

                zTempString.assign( /*NOXLATE*/L"DWF-eModel:" );
                zTempString.append( DWFString::RepairDecimalSeparators(zTempBuffer) );

                rSerializer.addAttribute( /*NOXLATE*/L"eModel", zTempString, /*NOXLATE*/L"xmlns:" );
                rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );
                rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zName );
            }

            //
            // base global section for the rest
            //
            DWFGlobalSection::serializeXML( rSerializer, nFlags );
        }
        rSerializer.endElement();
    }
        //
        // base class for manifest
        // 
    else
    {
        DWFGlobalSection::serializeXML( rSerializer, nFlags );
    }
}

#endif


///
///
///

_DWFTK_API
DWFGlobalSection::Factory::Factory( const DWFString& zType )
throw()
                         : DWFSection::Factory( zType )
{
    ;
}

_DWFTK_API
DWFGlobalSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFGlobalSection::Factory::build( const DWFString&  zType,
                                  const DWFString&  zName,
                                  const DWFString&  zTitle,
                                  DWFPackageReader* pPackageReader )
throw( DWFException )
{
    return DWFSection::Factory::build( zType, zName, zTitle, pPackageReader );
}

_DWFTK_API
DWFSection*
DWFGlobalSection::Factory::build( const DWFString&  zName,
                                  const DWFString&  zTitle,
                                  DWFPackageReader* pPackageReader )
throw( DWFException )
{
    return DWFSection::Factory::build( zName, zTitle, pPackageReader );
}

///
///
///

_DWFTK_API
DWFEPlotGlobalSection::Factory::Factory()
throw()
                              : DWFGlobalSection::Factory( _DWF_FORMAT_EPLOT_GLOBAL_TYPE_STRING )
{
    ;
}

_DWFTK_API
DWFEPlotGlobalSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFEPlotGlobalSection::Factory::build( const DWFString&  zName,
                                       const DWFString&  zTitle,
                                       DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFEPlotGlobalSection* pSection = DWFCORE_ALLOC_OBJECT( DWFEPlotGlobalSection(zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}


///
///
///

_DWFTK_API
DWFEModelGlobalSection::Factory::Factory()
throw()
                               : DWFGlobalSection::Factory( _DWF_FORMAT_EMODEL_GLOBAL_TYPE_STRING )
{
    ;
}

_DWFTK_API
DWFEModelGlobalSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
DWFSection*
DWFEModelGlobalSection::Factory::build( const DWFString&  zName,
                                        const DWFString&  zTitle,
                                        DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFEModelGlobalSection* pSection = DWFCORE_ALLOC_OBJECT( DWFEModelGlobalSection(zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}



