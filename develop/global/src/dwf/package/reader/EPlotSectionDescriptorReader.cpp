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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/EPlotSectionDescriptorReader.cpp#1 $
//



#include "dwf/package/Constants.h"
#include "dwf/package/reader/EPlotSectionDescriptorReader.h"
#include "dwf/package/SectionContentResource.h"
using namespace DWFToolkit;



_DWFTK_API
DWFEPlotSectionDescriptorReader::DWFEPlotSectionDescriptorReader( DWFPackageReader* pPackageReader,
                                                                  unsigned int      nProviderFlags )
throw()
                               : DWFSectionDescriptorReader( pPackageReader )
                               , _nProviderFlags( nProviderFlags )
                               , _nCurrentCollectionProvider( 0 )
                               , _pCurrentResource( NULL )
                               , _pCoordinateSystem( NULL )
                               , _pFilter( NULL )
{
    ;
}

_DWFTK_API
DWFEPlotSectionDescriptorReader::~DWFEPlotSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFEPlotSectionDescriptorReader* 
DWFEPlotSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFEPlotSectionDescriptorReader::setFilter( DWFEPlotSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
const char*
DWFEPlotSectionDescriptorReader::provideName( const char* zName )
throw()
{
    return zName;
}

_DWFTK_API
const char*
DWFEPlotSectionDescriptorReader::provideObjectID( const char* zObjectID )
throw()
{
    return zObjectID;
}

_DWFTK_API
double
DWFEPlotSectionDescriptorReader::provideVersion( double nVersion )
throw()
{
    return nVersion;
}

_DWFTK_API
double
DWFEPlotSectionDescriptorReader::providePlotOrder( double nPlotOrder )
throw()
{
    return nPlotOrder;
}

_DWFTK_API
unsigned int
DWFEPlotSectionDescriptorReader::provideColor( unsigned int nColorARGB )
throw()
{
    return nColorARGB;
}

_DWFTK_API
DWFPaper*
DWFEPlotSectionDescriptorReader::providePaper( DWFPaper* pPaper )
throw()
{
    return pPaper;
}

_DWFTK_API
DWFProperty*
DWFEPlotSectionDescriptorReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

_DWFTK_API
DWFResource*
DWFEPlotSectionDescriptorReader::provideResource( DWFResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFEPlotSectionDescriptorReader::provideFontResource( DWFFontResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFEPlotSectionDescriptorReader::provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFEPlotSectionDescriptorReader::provideImageResource( DWFImageResource* pResource )
throw()
{
    return pResource;
}

_DWFTK_API
DWFContentPresentationResource*
DWFEPlotSectionDescriptorReader::provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    return pResource;
}

void
DWFEPlotSectionDescriptorReader::_provideAttributes( const char** ppAttributeList )
throw()
{
        //
        // evaluate attribute list only if asked
        //
    if (_nProviderFlags & eProvideAttributes)
    {
        size_t iAttrib = 0;
        unsigned char nFound = eProvideNone;
        const char* pAttrib = NULL;

        for(; ppAttributeList[iAttrib]; iAttrib += 2)
        {
                //
                // skip over any acceptable prefixes in the element name
                //
            if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][4];
            } 
            else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][8];
            } 
            else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
            {
                pAttrib = &ppAttributeList[iAttrib][6];
            }
            else
            {
                pAttrib = &ppAttributeList[iAttrib][0];
            }

                //
                // provide the version
                //
            if ((_nProviderFlags & eProvideVersion) &&
                !(nFound & eProvideVersion)         &&
                 (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Version) == 0))
            {
                nFound |= eProvideVersion;
                
                _provideVersion( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
            }
                //
                // provide the name
                //
            else if ((_nProviderFlags & eProvideName)   &&
                     !(nFound & eProvideName)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
            {
                nFound |= eProvideName;

                _provideName( ppAttributeList[iAttrib+1] );
            }
                //
                // provide the plot order
                //
            else if ((_nProviderFlags & eProvidePlotOrder)   &&
                     !(nFound & eProvidePlotOrder)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_PlotOrder) == 0))
            {
                nFound |= eProvidePlotOrder;

                _providePlotOrder( DWFString::StringToDouble(ppAttributeList[iAttrib+1]) );
            }
                //
                // provide the color
                //
            else if ((_nProviderFlags & eProvideColor)   &&
                     !(nFound & eProvideColor)           &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Color) == 0))
            {
                nFound |= eProvideColor;

                char* pColor = (char*)ppAttributeList[iAttrib+1];
                char* pSavePtr;
                char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pColor, /*NOXLATE*/" ", &pSavePtr );
                unsigned int nColorARGB = ((unsigned char)::atoi(pToken) << 16);

                pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                nColorARGB |= ((unsigned char)::atoi(pToken) << 8);
                
                pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
                nColorARGB |= (unsigned char)::atoi(pToken);

                _provideColor( nColorARGB );
            }
                //
                // provide the object id
                //
            else if ((_nProviderFlags & eProvideObjectID)   &&
                     !(nFound & eProvideName)               &&
                      (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
            {
                nFound |= eProvideObjectID;

                _provideObjectID( ppAttributeList[iAttrib+1] );
            }
        }
    }
}

_DWFTK_API
void
DWFEPlotSectionDescriptorReader::notifyStartElement( const char*   zName,
                                                     const char**  ppAttributeList )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyStartElement( zName, ppAttributeList );

        //
        // skip over any known prefixes in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, zName, 8) == 0)
    {
        zName+=8;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, zName, 6) == 0)
    {
        zName+=6;
    }

        //
        //
        //
    switch (_nElementDepth)
    {
            //
            // should be ePlot:Page, if not, disable all further processing...
            //
        case 0:
        {
            if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Page) != 0)
            {
                _nProviderFlags = eProvideNone;
            }

            break;
        }
            //
            // *:Properties, *:Paper, *:Resources
            //
        case 1:
        {
            if ((_nProviderFlags & eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Properties) == 0))
            {
                _nCurrentCollectionProvider = eProvideProperties;
            }
            else if ((_nProviderFlags & eProvideResources) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resources) == 0))
            {
                _nCurrentCollectionProvider = eProvideResources;
            }
            else if ((_nProviderFlags & eProvidePaper) &&
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Paper) == 0))
            {
                //
                // create and provide new paper object
                //
                _providePaper( _pElementBuilder->buildPaper(ppAttributeList) );

                _nCurrentCollectionProvider = eProvideNone;
            }
            else
            {
                _nCurrentCollectionProvider = eProvideNone;
            }

            break;
        }
            //
            // *:Property, *:Resource
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideProperties) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // create and provide new property object
                //
                _provideProperty( _pElementBuilder->buildProperty(ppAttributeList) );
            }
            else if (_nCurrentCollectionProvider == eProvideResources)
            {
                if (((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) ||
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentResource) == 0)) &&
                    (_nProviderFlags & eProvideUntypedResource))
                {
                    //
                    // create new resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildResource( ppAttributeList, _pPackageReader );

                    //
                    //  Since 7.3:
                    //  As of 7.3 we are adding the capability to modify the section content resource when it is
                    //  read in and then republish it. To allow this plain resources that were published with the
                    //  role kzRole_ContentDefinition must be reinstantiated as a section content resource.
                    //
                    if (_pCurrentResource->role() == DWFXML::kzRole_ContentDefinition)
                    {
                        //
                        //  Create a section content resource object
                        //
                        DWFCORE_FREE_OBJECT( _pCurrentResource );
                        _pCurrentResource = _pElementBuilder->buildSectionContentResource( ppAttributeList, _pPackageReader );
                    }  
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_FontResource) == 0) &&
                         (_nProviderFlags & eProvideFontResource))
                {
                    //
                    // create new font resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildFontResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GraphicResource) == 0) &&
                         (_nProviderFlags & eProvideGraphicResource))
                {
                    //
                    // create new graphic resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildGraphicResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ImageResource) == 0) &&
                         (_nProviderFlags & eProvideImageResource))
                {
                    //
                    // create new image resource object
                    //
                    _pCurrentResource = _pElementBuilder->buildImageResource( ppAttributeList, _pPackageReader );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentationResource) == 0) &&
                         (_nProviderFlags & eProvideContentPresentationResource))
                {
                    //
                    // create new resource object for content metadata
                    //
                    _pCurrentResource = _pElementBuilder->buildContentPresentationResource( ppAttributeList, _pPackageReader );
                }
            }
            break;
        }
            //
            // *:Properties, *:CoordinateSystems
            //
        case 3:
        {
            break;
        }
            //
            // *:Property, *:CoordinateSystem
            //
        case 4:
        {
            if (_pCurrentResource &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // add new property object to the resource
                //
                _pCurrentResource->addProperty( _pElementBuilder->buildProperty(ppAttributeList), true );
            }            
            else if (_pCurrentResource &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_CoordinateSystem) == 0))
            {
                DWFGraphicResource* pGraphicResource = dynamic_cast<DWFGraphicResource*>(_pCurrentResource);
                if (pGraphicResource)
                {
                    //
                    // add new coordinate system object to the resource
                    //
                    _pCoordinateSystem = _pElementBuilder->buildCoordinateSystem( ppAttributeList );
                    pGraphicResource->addCoordinateSystem( _pCoordinateSystem );
                }
            }
            else if (_pCurrentResource &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Relationship) == 0))
            {
                //
                // add new relationship object to the resource
                //
                _pCurrentResource->addRelationship( _pElementBuilder->buildRelationship( ppAttributeList, _pPackageReader ) );
            }
            break;
        }
            //
            // *:Properties
            //
        case 5:
        {
            break;
        }
            //
            // *:Property
            //
        case 6:
        {
            if (_pCoordinateSystem &&
                (_nCurrentCollectionProvider == eProvideResources) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0))
            {
                //
                // add new property object to the resource
                //
                _pCoordinateSystem->addProperty( _pElementBuilder->buildProperty(ppAttributeList), true );
            }
            break;            
        }
        default:
        {
            ;
        }
    };

    _nElementDepth++;
}

_DWFTK_API
void
DWFEPlotSectionDescriptorReader::notifyEndElement( const char* zName )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndElement( zName );


        //
        // skip over any known prefixes in the element name
        //
    if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, zName, 4) == 0)
    {
        zName+=4;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, zName, 8) == 0)
    {
        zName+=8;
    } 
    else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, zName, 6) == 0)
    {
        zName+=6;
    }

        //
        // decrement the depth first
        // this lets us match up the switch statements for start and end
        //
    switch (--_nElementDepth)
    {
            //
            // *:Properties, *:Paper, *:Resources
            //
        case 1:
        {
            _nCurrentCollectionProvider = eProvideNone;
            break;
        }
            //
            // *:Property, *:Resource
            //
        case 2:
        {
            if ((_nCurrentCollectionProvider == eProvideResources) && _pCurrentResource)
            {
                if (((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Resource) == 0) ||
                     (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentResource) == 0)) &&
                    (_nProviderFlags & eProvideUntypedResource))
                {
                    //
                    // provide new resource object
                    //
                    _provideResource( _pCurrentResource );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_FontResource) == 0) &&
                         (_nProviderFlags & eProvideFontResource))
                {
                    //
                    // provide new font resource object
                    //
                    _provideFontResource( dynamic_cast<DWFFontResource*>(_pCurrentResource) );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_GraphicResource) == 0) &&
                         (_nProviderFlags & eProvideGraphicResource))
                {
                    //
                    // provide new graphic resource object
                    //
                    _provideGraphicResource( dynamic_cast<DWFGraphicResource*>(_pCurrentResource) );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ImageResource) == 0) &&
                         (_nProviderFlags & eProvideImageResource))
                {
                    //
                    // provide new image resource object
                    //
                    _provideImageResource( dynamic_cast<DWFImageResource*>(_pCurrentResource) );
                }
                else if ((DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_ContentPresentationResource) == 0) &&
                         (_nProviderFlags & eProvideContentPresentationResource))
                {
                    //
                    // provide new content resource object
                    //
                    _provideContentPresentationResource( dynamic_cast<DWFContentPresentationResource*>(_pCurrentResource) );
                }

                _pCurrentResource = NULL;
            }
            break;
        }
        default:
        {
            ;
        }
    };
}

_DWFTK_API
void
DWFEPlotSectionDescriptorReader::notifyStartNamespace( const char* zPrefix,
                                                       const char* zURI )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyStartNamespace( zPrefix, zURI );
}

_DWFTK_API
void
DWFEPlotSectionDescriptorReader::notifyEndNamespace( const char* zPrefix )
throw()
{
    //
    // invoke base reader
    //
    DWFSectionDescriptorReader::notifyEndNamespace( zPrefix );
}


void
DWFEPlotSectionDescriptorReader::_provideName( const char* zName )
throw()
{
    provideName( _pFilter ? _pFilter->provideName(zName) : zName );
}

void
DWFEPlotSectionDescriptorReader::_provideObjectID( const char* zObjectID )
throw()
{
    provideObjectID( _pFilter ? _pFilter->provideObjectID(zObjectID) : zObjectID );
}

void
DWFEPlotSectionDescriptorReader::_provideVersion( double nVersion )
throw()
{
    provideVersion( _pFilter ? _pFilter->provideVersion(nVersion) : nVersion );
}

void
DWFEPlotSectionDescriptorReader::_providePlotOrder( double nPlotOrder )
throw()
{
    providePlotOrder( _pFilter ? _pFilter->providePlotOrder(nPlotOrder) : nPlotOrder );
}

void
DWFEPlotSectionDescriptorReader::_provideColor( unsigned int nColorARGB )
throw()
{
    provideColor( _pFilter ? _pFilter->provideColor(nColorARGB) : nColorARGB );
}

void
DWFEPlotSectionDescriptorReader::_providePaper( DWFPaper* pPaper )
throw()
{
    providePaper( _pFilter ? _pFilter->providePaper(pPaper) : pPaper );
}

void
DWFEPlotSectionDescriptorReader::_provideProperty( DWFProperty* pProperty )
throw()
{
    provideProperty( _pFilter ? _pFilter->provideProperty(pProperty) : pProperty );
}

void
DWFEPlotSectionDescriptorReader::_provideResource( DWFResource* pResource )
throw()
{
    provideResource( _pFilter ? _pFilter->provideResource(pResource) : pResource );
}

void
DWFEPlotSectionDescriptorReader::_provideFontResource( DWFFontResource* pResource )
throw()
{
    provideFontResource( _pFilter ? _pFilter->provideFontResource(pResource) : pResource );
}

void
DWFEPlotSectionDescriptorReader::_provideGraphicResource( DWFGraphicResource* pResource )
throw()
{
    provideGraphicResource( _pFilter ? _pFilter->provideGraphicResource(pResource) : pResource );
}

void
DWFEPlotSectionDescriptorReader::_provideImageResource( DWFImageResource* pResource )
throw()
{
    provideImageResource( _pFilter ? _pFilter->provideImageResource(pResource) : pResource );
}

void
DWFEPlotSectionDescriptorReader::_provideContentPresentationResource( DWFContentPresentationResource* pResource )
throw()
{
    if (_pFilter)
    {
        pResource = _pFilter->provideContentPresentationResource( pResource );
    }
    provideContentPresentationResource( pResource );
}
